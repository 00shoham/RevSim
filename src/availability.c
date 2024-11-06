#include "base.h"

/* be sure to allocate monthly summary data before calling this */
int CalculateBaselineWorkingDays( _CONFIG* config )
  {
  /* printf( "CalculateBaselineWorkingDays()\n" ); */
  if( config==NULL )
    {
    Warning( "CalculateBaselineWorkingDays - no config" );
    return -1;
    }

  int nDays = config->simulationDurationDays;
  if( nDays<1 )
    {
    Warning( "CalculateBaselineWorkingDays - no duration" );
    return -2;
    }

  if( config->baselineWorkDays!=NULL )
    {
    Warning( "Pre-existing baseline working days in config" );
    free( config->baselineWorkDays );
    config->baselineWorkDays = NULL;
    }

  config->baselineWorkDays = (_SINGLE_DAY*)SafeCalloc( nDays+1, sizeof(_SINGLE_DAY), "baseline working days" );
  config->nBaselineWorkDays = nDays;

  time_t t = config->simulationStart;
  int dow = DayOfWeek( t );
  _SINGLE_DAY* d = config->baselineWorkDays;

  _MONTHLY_SUMMARY* month = config->monthlySummary;
  /* printf( "CalculateBaselineWorkingDays() - initial month is %p\n", month ); */
  int nWorkDaysPerYear = 0;
  for( int i=0; i<nDays; ++i )
    {
    /* printf( "CalculateBaselineWorkingDays() - day %d\n", i ); */
    if( TimeToMMDD( t, &(d->date) )!=0 )
      Error( "Failed to convert time %ld to MMDD", (int)t );
    if( month!=NULL )
      {
      /* printf( "CalculateBaselineWorkingDays() - non-NULL month\n" ); */
      if( d->date.day==1 )
        {
        /* printf( "CalculateBaselineWorkingDays() - day is 1\n" ); */
        if( d->date.year==month->monthStart.year
            && d->date.month==month->monthStart.month
            && d->date.day==month->monthStart.day )
          {
          d->month = month;
          /* printf( "(A) Linking date %04d-%02d-%02d to month %04d-%02d\n", d->date.year, d->date.month, d->date.day, month->monthStart.year, month->monthStart.month ); */
          }
        else
          {
          ++month;
          /* printf( "CalculateBaselineWorkingDays() - incremented month\n" ); */
          if( month >= config->monthlySummary + config->nMonths )
            Error( "Rolled off the end of the global month-summary array" );

          if( d->date.year==month->monthStart.year
              && d->date.month==month->monthStart.month
              && d->date.day==month->monthStart.day )
            {
            d->month = month;
            /* printf( "(B) Linking date %04d-%02d-%02d to month %04d-%02d\n", d->date.year, d->date.month, d->date.day, month->monthStart.year, month->monthStart.month ); */
            }
          else
            Warning( "Cannot find/assign global month-summary for date %04d-%02d-%02d",
                     d->date.year, d->date.month, d->date.day );
          }
        }
      else
        {
        if( d->date.year==month->monthStart.year && d->date.month==month->monthStart.month )
          d->month = month;
        else
          Warning( "Mis-aligned day/month in global config @ %04d-%02d-%02d",
                   d->date.year, d->date.month, d->date.day );
        }
      }

    d->t = t;

    if( dow==0 || dow==6 )
      d->working = 0; /* weekend */
    else
      d->working = 1; /* provisionally */

    if( d->working )
      if( FallsOnHoliday( config->holidays, &(d->date) )==0 )
        d->working = 0;

    ++dow;
    if( dow==7 )
      dow = 0;

    if( i<365 && d->working )
      ++nWorkDaysPerYear;

    t += DAY_IN_SECONDS;
    ++d;
    }

  /* hack if the simulation is shorter than 1 year - just an approximation */
  if( nDays < 365 )
    {
    double d = nWorkDaysPerYear;
    d *= 365.0;
    d /= (double)nDays;
    nWorkDaysPerYear = (int)(d + 0.5);
    }

  /* we're ignoring leap years, at a minimum */
  Event( "Calculated that there are about %d work days per year (people work that, less vacations)", nWorkDaysPerYear );
  config->nWorkDaysPerYear = nWorkDaysPerYear;

  return 0;
  }

void PrintAvailability( FILE* out, _CONFIG* config )
  {
  if( config==NULL )
    Error( "PrintAvailability - no config" );

  if( out==NULL )
    Error( "PrintAvailability - no output stream" );

  int nDays = config->simulationDurationDays;
  if( nDays<1 )
    Error( "PrintAvailability - no duration" );

  if( config->baselineWorkDays == NULL )
    Error( "PrintAvailability - no working days" );

  _SINGLE_DAY* d = config->baselineWorkDays;
  for( int i=0; i<nDays; ++i )
    {
    _MMDD* dPtr = &( d->date );
    fprintf( out, "%04d-%02d-%02d %s\n",
             dPtr->year, dPtr->month, dPtr->day,
             d->working ? "WORKING" : "OFF" );
    ++d;
    }
  }

int CountWorkDaysInMonth( _SALES_REP* s, int Y, int M )
  {
  int nWorkDays = 0;
  _SINGLE_DAY* array = s->workDays;
  int arrayLen = s->nWorkDays;

  int started = 0;
  for( int i=0; i<arrayLen; ++i )
    {
    _SINGLE_DAY* day = array + i;
    if( day->date.month==M
        && day->date.year==Y )
      {
      started = 1;
      if( day->working )
        ++nWorkDays;
      }
    else
      {
      if( started ) /* after the month of interest */
        break;
      }
    }

  return nWorkDays;
  }

void SetWorkDay( _SINGLE_DAY* array, int arrayLen, int Y, int M, int D, int val )
  {
  for( int i=0; i<arrayLen; ++i )
    {
    _SINGLE_DAY* day = array + i;
    if( day->date.day==D
        && day->date.month==M
        && day->date.year==Y )
      {
      day->working = val;
      break;
      }
    if( day->date.month>M && day->date.year>=Y )
      break; /* too late - no match?  perhaps 31st or something */
    }
  }

void SetRandomVacationDay( _SINGLE_DAY* array, int arrayLen, int Y, int M )
  {
  int nTries = 100;
  while( nTries-- )
    {
    int n = lrand48() % 31;
    if( n<arrayLen )
      {
      _SINGLE_DAY* sd = array + n;
      if( sd->date.month==M
          && sd->date.year==Y
          && sd->working==1 )
        {
        sd->working = 0;
        break;
        }
      }
    }
  // Notice( "... SetRandomVacationDay - done in %d tries", 100-nTries );
  }

void AddVacationDaysInMonth( _SALES_REP* s, int Y, int M, int iDaysPerMonth )
  {
  // Notice( "Finding %d vacation days for %s during %04d-%02d",
  //         iDaysPerMonth, s->id, Y, M );

  if( CountWorkDaysInMonth( s, Y, M ) < iDaysPerMonth )
    { /* not enough work days already - just take them all off (inefficiently) */
    // Notice( "... %04d-%02d has lots of stat holidays - filling in", Y, M );
    for( int d=1; d<=31; ++d )
      SetWorkDay( s->workDays, s->nWorkDays, Y, M, d, 0 );
    return;
    }

  _SINGLE_DAY* array = s->workDays;
  int arrayLen = s->nWorkDays;
  while( array->date.year < Y || array->date.month < M )
    {
    ++array;
    --arrayLen;
    if( arrayLen==0 )
      {
      Warning( "Failed to add vacation days to month %04d-%02d for %s", Y, M, s->id );
      return;
      }
    }

  // Notice( "... Found first day in array - %04d-%02d", array->date.year, array->date.month );

  for( int i=0; i<iDaysPerMonth; ++i )
    SetRandomVacationDay( array, arrayLen, Y, M );
  }

void InitializeMonthlySummaryArray( char* subject,
                                    _MONTHLY_SUMMARY** msPtr,
                                    int* nMonthsPtr,
                                    _SINGLE_DAY* daysArray,
                                    int nDays,
                                    _MMDD* firstDay,
                                    _MMDD* lastDay )
  {
  if( msPtr==NULL || nMonthsPtr==NULL || firstDay==NULL || lastDay==NULL )
    return;

#if 0
  Notice( "InitializeMonthlySummaryArray( %s, <ptr>, <ptr>, %04d-%02d-%02d, %04d-%02d-%02d\n",
          subject,
          firstDay->year, firstDay->month, firstDay->day,
          lastDay->year, lastDay->month, lastDay->day );
#endif

  _MONTHLY_SUMMARY* ms = *msPtr;
  if( ms!=NULL )
    FREE( ms );

  int nMonths = NumberOfMonths( firstDay, lastDay );
  if( nMonths<1 )
    Error( "%s does not have any work months", subject );
  *nMonthsPtr = nMonths;
  printf( "Setting nMonths for %s to %d\n", subject, nMonths );

  ms = (_MONTHLY_SUMMARY*)SafeCalloc( nMonths, sizeof( _MONTHLY_SUMMARY ), "Monthly summary array" );
  *msPtr = ms;

  /* allocate monthly summary array */
  int lastMonth = -1;
  int k = 0;
  time_t tStart = MMDDToTime( firstDay );
  time_t tFinish = MMDDToTime( lastDay );

  int dayNo = 0;
  _SINGLE_DAY* day = daysArray;
  _MONTHLY_SUMMARY* thisMonth = NULL;
  for( time_t t = tStart; t <= tFinish + 3600; t += DAY_IN_SECONDS  )
    {
    int m = MonthFromTime( t );
    int y = YearFromTime( t );
    if( m!=lastMonth )
      {
      thisMonth = ms + k;
      thisMonth->monthStart.year = y;
      thisMonth->monthStart.month = m;
      thisMonth->monthStart.day = 1;
      lastMonth = m;
      ++k;
      }
    if( day!=NULL )
      {
      day->month = thisMonth;
      ++dayNo;
      ++day;
      if( dayNo >= nDays )
        break;
      }
    }
  }

int EstablishWorkDays( _SALES_REP* s, _CONFIG* config )
  {
  if( s==NULL || EMPTY( s->id ) || EMPTY( s->name ) || config==NULL )
    Error( "NULL inputs to EstablishWorkDays" );

  Event( "Setting up work days for %s - %s", s->id, s->name );

  if( s->workDays!=NULL )
    {
    Warning( "Rep %s already has work days.  Recalculating.", s->id );
    free( s->workDays );
    s->workDays = NULL;
    }

  /*
  Notice( "%s - first=%04d-%02d-%02d last=%04d-%02d-%02d\n", s->id,
          s->firstDay.year, s->firstDay.month, s->firstDay.day,
          s->lastDay.year, s->lastDay.month, s->lastDay.day );
  */

  Event( "%s has %04d-%02d-%02d as first day and %04d-%02d-%02d as last day",
         s->id,
         s->firstDay.year, s->firstDay.month, s->firstDay.day,
         s->lastDay.year, s->lastDay.month, s->lastDay.day );

  s->nWorkDays = NumberOfDays( &(s->firstDay), &(s->lastDay) ) - 1;
  // Notice( "%s has %d work days", s->id, s->nWorkDays );
  if( s->nWorkDays<1 )
    Error( "Rep %s does not have any work days", s->id );
  else
    Event( "Rep %s is scheduled to have %d calendar days where they may work", s->id, s->nWorkDays );

  s->workDays = (_SINGLE_DAY*)SafeCalloc( s->nWorkDays + 10 /* safety */, sizeof( _SINGLE_DAY ), "Workdays array" );
  s->endOfWorkDays = s->workDays + s->nWorkDays;

  char subject[BUFLEN];
  snprintf( subject, sizeof(subject)-1, "Sales rep %s", s->id );
  InitializeMonthlySummaryArray( subject,
                                 &(s->monthlySummary),
                                 &(s->nMonths),
                                 s->workDays,
                                 s->nWorkDays,
                                 &(s->firstDay),
                                 &(s->lastDay) );
  
  int daysDelay = NumberOfDays( &(config->simulationFirstDay),
                                &(s->firstDay) )
                - 1;
  if( daysDelay<0 )
    Error( "Rep %s starts work before the simulation?", s->id );

  Notice( "%s has %d workdays", s->id, s->nWorkDays );
  Notice( "Copying that from baseline + %d days delay", daysDelay );
  memcpy( s->workDays,
          config->baselineWorkDays + daysDelay,
          s->nWorkDays * sizeof( _SINGLE_DAY ) );

  /* ensure that the rep's daily fees don't point to the
     common fees */
  _SINGLE_DAY* day = s->workDays;
  _SINGLE_DAY* lastDay = day + s->nWorkDays;
  for( ; day<lastDay; ++day )
    {
    day->dailySales = NULL;
    day->fees = NULL;
    }

  _SALES_REP_CLASS* st = s->class;
  if( st==NULL )
    {
    if( NOTEMPTY( s->id )
        && strcasecmp( s->id, "customer-care" )==0 )
      {
      /* it's okay for customer care to have no 'class' */
      }
    else
      Error( "Rep %s has no class", s->id );
    }

  double yearlyDays = 0;
  _VACATION* v = st==NULL ? NULL : st->allowance;
  if( v==NULL )
    yearlyDays = 0;
  else
    yearlyDays = v->daysPerYear;
  double monthlyDays = yearlyDays / 12;
  int iDaysPerMonth = (int)ceil( monthlyDays + 0.01 );
  // Notice( "Rep %s gets about %d vacation days per month", s->id, iDaysPerMonth );

  int y1 = s->firstDay.year;
  if( y1==0 ) y1 = YearNow();
  int y2 = s->lastDay.year;
  if( y2==0 ) y2 = YearNow();

  if( y1==y2 )
    {
    // Notice( "Rep %s works during a single year - %d", s->id, s->firstDay.year );
    for( int m=s->firstDay.month; m<=s->lastDay.month; ++m )
      AddVacationDaysInMonth( s, s->firstDay.year, m, iDaysPerMonth );
    }
  else /* validation means that y1<y2 */
    {
    // Notice( "Rep %s works during two or more calendar years - %d - %d", s->id, s->firstDay.year, s->lastDay.year );
    for( int m=s->firstDay.month; m<=12; ++m )
      AddVacationDaysInMonth( s, s->firstDay.year, m, iDaysPerMonth );
    for( int y=s->firstDay.year + 1; y<s->lastDay.year; ++y )
      for( int m=1; m<=12; ++m )
        AddVacationDaysInMonth( s, y, m, iDaysPerMonth );
    for( int m=1; m<=s->lastDay.month; ++m )
      AddVacationDaysInMonth( s, s->lastDay.year, m, iDaysPerMonth );
    }

  int monthNo = 0;
  int yearNo = 0;
  double productivity = 0;
  _SINGLE_DAY* prevDay = NULL;
  _SINGLE_DAY* repDay = s->workDays;
  _SINGLE_DAY* repLastDay = repDay + s->nWorkDays;

  /* calculate productivity (maxCalls) in the days array */
  for( int i=0; (i < s->nWorkDays)
                && (repDay < repLastDay)/*safety*/
                && (repDay->date.year!=0 )/* real safe */; ++i )
    {
    if( prevDay!=NULL && repDay->date.month != prevDay->date.month )
      {
      ++monthNo;
      if( monthNo % 12 == 0 )
        ++yearNo;
      }

    if( s->class!=NULL
        && monthNo < MONTHS )
      productivity = s->class->productivity[monthNo];
    else
      productivity = 100.0;

    repDay->maxCalls = s->dailyCalls * productivity / 100.0;

    prevDay = repDay;
    ++repDay;
    }

  return 0;
  }

void PrintWorkDays( FILE* out, _SALES_REP* s )
  {
  if( s==NULL )
    Error( "PrintWorkDays - no sales rep" );

  if( out==NULL )
    Error( "PrintWorkDays - no output stream" );

  int nDays = s->nWorkDays;
  if( nDays<1 )
    Error( "PrintWorkDays - no work days array defined (a)" );
  if( s->workDays==NULL )
    Error( "PrintWorkDays - no work days array defined (b)" );

  _SINGLE_DAY* d = s->workDays;
  for( int i=0; i<nDays; ++i )
    {
    _MMDD* dPtr = &( d->date );
    fprintf( out, "%04d-%02d-%02d %s\n",
             dPtr->year, dPtr->month, dPtr->day,
             d->working ? "WORKING" : "OFF" );
    ++d;
    }
  }
