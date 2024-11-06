#include "base.h"

FILE* events = NULL;

int main( int argc, char** argv )
  {
  int err = 0;
  char* confDir = ".";
  char* confFile = "config.ini";
  char* outFile = NULL;
  char* eventsFile = NULL;
  char* counterFile = NULL;
  char* unitsFile = NULL;
  char* cashFile = NULL;
  int confTest = 0;

  RandomSeed();

  for( int i=1; i<argc; ++i )
    {
    if( strcmp( argv[i], "-c" )==0 && i+1<argc )
      confFile = argv[++i];
    else if( strcmp( argv[i], "-d" )==0 && i+1<argc )
      confDir = argv[++i];
    else if( strcmp( argv[i], "-o" )==0 && i+1<argc )
      outFile = argv[++i];
    else if( strcmp( argv[i], "-e" )==0 && i+1<argc )
      eventsFile = argv[++i];
    else if( strcmp( argv[i], "-u" )==0 && i+1<argc )
      unitsFile = argv[++i];
    else if( strcmp( argv[i], "-k" )==0 && i+1<argc )
      counterFile = argv[++i];
    else if( strcmp( argv[i], "-s" )==0 && i+1<argc )
      cashFile = argv[++i];
    else if( strcmp( argv[i], "-conftest" )==0 )
      confTest = 1;
    else if( strcmp( argv[i], "-h" )==0 )
      {
      printf("USAGE: %s [-c configFile] [-d configDir] [-o outFile] [-e eventsFile] [-u unitsFile] [-s cashFile] [-conftest]\n", argv[0] );
      exit(0);
      }
    else
      {
      printf("ERROR: unknown argument [%s]\n", argv[i] );
      exit(1);
      }
    }

  char* confPath = MakeFullPath( confDir, confFile );
  _CONFIG* conf = (_CONFIG*)calloc( 1, sizeof( _CONFIG ) );
  if( conf==NULL ) Error( "Cannot allocate CONFIG object" );

  /* printf( "Reading configuration file.\n" ); */

  SetDefaults( conf );
  ReadConfig( conf, confPath );
  ValidateConfig( conf );

  printf( "conf->monthlySummary = %p\n", conf->monthlySummary );

  free( confPath );
  confPath = NULL;

  if( confTest )
    {
    printf( "Configuration OK.\n" );
    exit(0);
    }

  FILE* out = stdout;
  if( NOTEMPTY( outFile ) && strcmp( outFile, "-" )!=0 )
    {
    out = fopen( outFile, "w" );
    if( out==NULL )
      Error( "Cannot open %s", outFile );
    }

  if( NOTEMPTY( eventsFile ) )
    {
    if( strcmp( eventsFile, "-" )==0 )
      events = stdout;
    else
      events = fopen( eventsFile, "w" );

    if( events==NULL )
      Error( "Cannot open %s", eventsFile );
    }

  printf( "Working out a baseline of work days vs. weekends and stat holidays.\n" );

  /* PrintAvailability( out, conf ); */

  printf( "Assigning vacation days, creating random attrition/replacement and paying salaries.\n" );

  err = EstablishWorkDays( conf->customerCare, conf );
  if( err!=0 )
    Warning( "Failed to establish work days for %s", conf->customerCare->id );

  RecordCashEvents( conf );

  Event( "Simulation starts on %04d-%02d-%02d",
         conf->simulationFirstDay.year,
         conf->simulationFirstDay.month,
         conf->simulationFirstDay.day );
  Event( "Simulation ends before %04d-%02d-%02d (%d days)",
         conf->simulationEndDay.year,
         conf->simulationEndDay.month,
         conf->simulationEndDay.day,
         conf->simulationDurationDays );

  for( _SALES_REP* s = conf->salesReps; s!=NULL; s=s->next )
    {
    err = EstablishWorkDays( s, conf );
    if( err!=0 )
      Warning( "Failed to establish work days for %s", s->id );
    PaySingleRepSalary( conf, s );
    }

  int dayNo = 0;
  _SINGLE_DAY* day = conf->baselineWorkDays;
  _SINGLE_DAY* lastDayWhenWeAddedSummaries = NULL;

  printf( "Stepping through the timeline and simulating calls.\n" );

  double carryForwardCashBalance = 0;

  /* Simulate sales to existing customers identified as initial revenue per product: */
  _SINGLE_DAY* startDay = FindSingleDay( &(conf->simulationFirstDay), conf->customerCare->workDays, conf->customerCare->nWorkDays );
  if( startDay==NULL ) Error( "Failed to find _SINGLE_DAY for start of simulation" );

  time_t lastGoodDayInSimulation = conf->simulationEnd - 24 * 60 * 60;
  _MMDD lastGoodDay = {0};
  if( TimeToMMDD( lastGoodDayInSimulation, &lastGoodDay )!=0 )
    Error( "Failed to convert last good day to MMDD" );
  _SINGLE_DAY* endDay = FindSingleDay( &lastGoodDay, conf->customerCare->workDays, conf->customerCare->nWorkDays );
  if( endDay==NULL ) Error( "Failed to find _SINGLE_DAY for end of simulation" );

  for( _PRODUCT* p = conf->products; p!=NULL; p=p->next )
    {
    if( p->initialMonthlyRevenue<=0 )
      continue;
    if( p->initialMonthlyCustomers<1 )
      continue;
    double avgRevenuePerCustomer = p->initialMonthlyRevenue
                                 / (double)p->initialMonthlyCustomers;
    printf( "Calculating %s revenue from initial customers ($%.1lf/customer, %d customers)\n",
            p->id, avgRevenuePerCustomer, p->initialMonthlyCustomers );
    for( int custNo = 0; custNo < p->initialMonthlyCustomers; ++custNo )
      {
      CloseSingleSale( conf, conf->customerCare, NULL,
                       startDay, endDay,
                       p, avgRevenuePerCustomer );
      }
    }

  for( time_t tSim = conf->simulationStart;
       dayNo < conf->simulationDurationDays
       && tSim <= conf->simulationEnd;
       tSim += DAY_IN_SECONDS )
    {
    day->cashOnHand += carryForwardCashBalance;

    /* for reporting purposes, count how many orgs are left to call: */
    if( dayNo < conf->simulationDurationDays
        && day->date.day==1
        && day->month!=NULL )
      day->month->nAvailableOrgs = CountAvailableOrgs( conf, tSim );

    /* if we just started a month, then add up the summaries
       from all the reps plus customer care for the previous
       month */
    if( dayNo>0 && day->date.day==1 )
      {
      int M = day->date.month - 1;
      int Y = day->date.year;
      if( M==0 )
        {
        M=12;
        --Y;
        }
      AddMonthlySummariesSingleMonth( conf, Y, M );
      lastDayWhenWeAddedSummaries = day;
      }

    if( conf->taxRate>0
        && day->date.month == 1
        && day->date.day == 1 )
      { /* happy new year!  now pay tax for last year. */
      double netIncome = NetIncomeForYear( day->date.year-1, conf->monthlySummary, conf->nMonths );
      netIncome -= GetTaxLossCarryForward( conf, day->date.year-1 );
      if( netIncome>0 )
        {
        Notice( "Net income of %.2lf for year %04d", netIncome, day->date.year-1 );
        _SINGLE_DAY* taxDay = day;

        if( taxDay > conf->baselineWorkDays )
          --taxDay; /* pay taxes at end of last year.. */

        double taxAmount = netIncome * conf->taxRate / 100.0;
        Notice( "Tax payable is %.2lf on %04d-%02d-%02d", taxAmount, taxDay->date.year, taxDay->date.month, taxDay->date.day );
        taxDay->cashEvents = NewCashEvent( et_tax_payment,
                                           taxDay->date,
                                           taxAmount,
                                           taxDay->cashEvents );
        taxDay->cashOnHand -= taxAmount;
        day->cashOnHand -= taxAmount;
        }
      else
        SetTaxLossCarryForward( conf, day->date.year, fabs(netIncome) );
      }

    if( day->working==0 ) /* nobody working this day */
      {
      Event( "Nobody working on %04d-%02d-%02d", day->date.year, day->date.month, day->date.day );
      }
    else
      {
      Event( "Simulate calls for %04d-%02d-%02d", day->date.year, day->date.month, day->date.day );
      SimulateCalls( conf, dayNo, tSim );
      }

    carryForwardCashBalance = day->cashOnHand;
    ++day;
    ++dayNo;
    }

  /* do we have to add up summaries for the last month? */
  if( lastDayWhenWeAddedSummaries != NULL
      && lastDayWhenWeAddedSummaries != day-1 )
    {
    _SINGLE_DAY* d = day - 1;
    if( d > conf->baselineWorkDays )
      AddMonthlySummariesSingleMonth( conf, d->date.year - 1, d->date.month );
    }

  printf( "Simulated %d days\n", dayNo );
  printf( "Total customer wins: %d\n", conf->nCustomerWins );
  double duration = conf->nCustomerMonths;
  duration /= (double)conf->nCustomerWins;
  printf( "Average of %.1lf months before each customer is lost or the simulation ends\n", duration );

  _SALES_REP** reps = SalesRepArray( conf->salesReps );
  for( int i=0; i<conf->nSalesReps; ++i )
    PrintRevenueSummaryForRep( out, reps[i] );
  free( reps );

  PrintRevenueSummaryForRep( out, conf->customerCare );

  PrintRevenueSummary( out, conf->monthlySummary, conf->nMonths, "Total revenue and cost of sales" );

  if( NOTEMPTY( unitsFile ) )
    {
    FILE* f = fopen( unitsFile, "w" );
    if( f!=NULL )
      {
      UnitsReport( f, conf );
      fclose( f );
      }
    else
      Warning( "Failed to open %s for writing", unitsFile );
    }

  if( NOTEMPTY( counterFile ) )
    {
    FILE* f = fopen( counterFile, "w" );
    if( f!=NULL )
      {
      PrintCounters( f, conf );
      fclose( f );
      }
    else
      Warning( "Failed to open %s for writing", counterFile );
    }

  if( NOTEMPTY( cashFile ) )
    {
    FILE* f = fopen( cashFile, "w" );
    if( f!=NULL )
      {
      PrintDailyCash( f, conf );
      fclose( f );
      }
    else
      Warning( "Failed to open %s for writing", cashFile );
    }

  FreeConfig( conf );
  conf = NULL;

  if( out!=stdout && out!=NULL )
    fclose( out );

  if( events!=stdout && events!=NULL )
    fclose( events );

  return 0;
  }
