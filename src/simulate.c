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
    else if( strcmp( argv[i], "-k" )==0 && i+1<argc )
      counterFile = argv[++i];
    else if( strcmp( argv[i], "-h" )==0 )
      {
      printf("USAGE: %s [-c configFile] [-d configDir] [-o outFile] [-e eventsFile]\n", argv[0] );
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

  printf( "Reading configuration file.\n" );

  SetDefaults( conf );
  ReadConfig( conf, confPath );
  ValidateConfig( conf );

  free( confPath );
  confPath = NULL;

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

  CalculateBaselineWorkingDays( conf );
  /* PrintAvailability( out, conf ); */

  printf( "Assigning vacation days, creating random attrition/replacement and paying salaries.\n" );

  err = EstablishWorkDays( conf->customerCare, conf );
  if( err!=0 )
    Warning( "Failed to establish work days for %s", conf->customerCare->id );

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

  printf( "Stepping through the timeline and simulating calls.\n" );

  for( time_t tSim = conf->simulationStart;
       dayNo < conf->simulationDurationDays
       && tSim <= conf->simulationEnd;
       tSim += DAY_IN_SECONDS )
    {
    if( day->working==0 ) /* nobody working this day */
      {
      Event( "Nobody working on %04d-%02d-%02d", day->date.year, day->date.month, day->date.day );
      ++day;
      continue;
      }

    SimulateCalls( conf, dayNo, tSim );

    if( dayNo < conf->simulationDurationDays )
      {
      if( day->date.day==1 )
        {
        if( day->month!=NULL )
          day->month->nAvailableOrgs = CountAvailableOrgs( conf, tSim );
        }
      }
    ++day;

    ++dayNo;
    }

  printf( "Simulated %d days\n", dayNo );
  printf( "Total customer wins: %d\n", conf->nCustomerWins );
  double duration = conf->nCustomerMonths;
  duration /= (double)conf->nCustomerWins;
  printf( "Average of %.1lf months before each customer is lost or the simulation ends\n", duration );

  for( _SALES_REP* s = conf->salesReps; s!=NULL; s=s->next )
    AddMonthlySummaries( conf->monthlySummary, conf->nMonths, s->monthlySummary, s->nMonths );

  AddMonthlySummaries( conf->monthlySummary, conf->nMonths, conf->customerCare->monthlySummary, conf->customerCare->nMonths );

  _SALES_REP** reps = SalesRepArray( conf->salesReps );
  for( int i=0; i<conf->nSalesReps; ++i )
    PrintRevenueSummaryForRep( out, reps[i] );
  free( reps );

  PrintRevenueSummaryForRep( out, conf->customerCare );

  PrintRevenueSummary( out, conf->monthlySummary, conf->nMonths, "Total revenue and cost of sales" );

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

  FreeConfig( conf );

  if( out!=stdout && out!=NULL )
    fclose( out );

  if( events!=stdout && events!=NULL )
    fclose( events );

  return 0;
  }
