#include "base.h"

_SALES_REP_CLASS* NewSalesRepClass( char* id, _SALES_REP_CLASS* list )
  {
  if( EMPTY( id ) )
    Error( "NewSalesRepClass(NULL)" );
  _SALES_REP_CLASS* rt = SafeCalloc( 1, sizeof( _SALES_REP_CLASS ), "_SALES_REP_CLASS" );
  if( NOTEMPTY( id ) )
    rt->id = strdup( id );
  rt->next = list;
  return rt;
  }

_SALES_REP_CLASS* FindSalesRepClass( _SALES_REP_CLASS* list, char* id )
  {
  if( EMPTY( id ) )
    return NULL;
  for( _SALES_REP_CLASS* rt = list; rt!=NULL; rt=rt->next )
    if( NOTEMPTY( rt->id ) && strcasecmp( rt->id, id )==0 )
      return rt;
  return NULL;
  }

void FreeSalesRepClass( _SALES_REP_CLASS* rt )
  {
  if( rt==NULL )
    return;
  if( rt->next!=NULL )
    {
    FreeSalesRepClass( rt->next );
    rt->next = NULL;
    }
  FreeIfAllocated( &(rt->id) );
  FreeIfAllocated( &(rt->name) );
  if( rt->products != NULL )
    {
    free( rt->products );
    rt->nProducts = 0;
    }

  free( rt );
  }

int ValidateSingleSalesRepClass( _SALES_REP_CLASS* rt )
  {
  if( rt==NULL || EMPTY( rt->id ) )
    return -100;

  if( EMPTY( rt->name ) )
    {
    Warning( "Sales rep class %s has no name", rt->id );
    return -1;
    }

  for( int i=0; i<MONTHS; ++i )
    {
    if( rt->productivity[i]<0 || rt->productivity[i]>100 )
      {
      Warning( "Productivity in rep class %s at month %d is %.1f (invalid)", rt->id, i, rt->productivity[i] );
      return -2;
      }
    }

  if( rt->salaryOnly )
    {
    if( rt->commission!=0 )
      {
      Warning( "salary only rep classes (%s) should have no commission structure", rt->id );
      return -3;
      }
    if( rt->nProducts!=0 || rt->products!=NULL )
      {
      Warning( "salary only rep classes (%s) should have no products", rt->id );
      return -4;
      }
    }
  else
    {
    if( rt->commission<0 || rt->commission>80 )
      {
      Warning( "Commission in rep class %s is %.1f (invalid)", rt->id, rt->commission );
      return -5;
      }

    if( rt->nProducts<1 || rt->products==NULL )
      {
      Warning( "Rep class %s has no products to sell!", rt->id );
      return -6;
      }
    }

  if( rt-> sdevEmploymentMonths > rt->averageEmploymentMonths )
    {
    Warning( "Rep class %s has too large employment months standard deviation!", rt->id );
    return -7;
    }

  if( rt->allowance==NULL )
    {
    Warning( "Rep %s has no vacation allowance!", rt->id );
    /* return -5; */
    }

  return 0;
  }

void PrintSalesRepClass( FILE* f, _SALES_REP_CLASS* rt )
  {
  if( f==NULL || rt==NULL || EMPTY( rt->id ) )
    return;

  fprintf( f, "REP_CLASS=%s\n", rt->id );
  if( NOTEMPTY( rt->name ) )
    fprintf( f, "REP_CLASS_NAME=%s\n", rt->name );

  fprintf( f, "REP_CLASS_PRODUCTIVITY=" );
  for( int i=0; i<MONTHS; ++i )
    {
    fprintf( f, "%s%.1f", i==0?"":",", rt->productivity[i] );
    if( rt->productivity[i]>99.99 )
      break;
    }
  fprintf( f, "\n" );

  fprintf( f, "REP_CLASS_COMMISSION=%.1f\n", rt->commission );
  if( rt->commissionMonths > 0 )
    fprintf( f, "REP_CLASS_COMMISSION_MONTHS=%d\n", rt->commissionMonths );

  if( rt->averageEmploymentMonths > 0 )
    fprintf( f, "REP_CLASS_AVG_MONTHS_EMPLOYMENT=%.1lf\n", rt->averageEmploymentMonths );
  if( rt->sdevEmploymentMonths > 0 )
    fprintf( f, "REP_CLASS_SDEV_MONTHS_EMPLOYMENT=%.1lf\n", rt->sdevEmploymentMonths );

  for( int i=0; i<rt->nProducts; ++i )
    {
    if( rt->products[i]!=NULL && NOTEMPTY( rt->products[i]->id ) )
      fprintf( f, "REP_CLASS_PRODUCT=%s\n", rt->products[i]->id );
    }

  fprintf( f, "REP_CLASS_VACATION=%s\n", rt->allowance->id );

  if( rt->annualPayIncreasePercent>0 )
    fprintf( f, "REP_CLASS_ANNUAL_INCREASE_PERCENT=%.1lf\n", rt->annualPayIncreasePercent );

  if( rt->salaryOnly )
    fprintf( f, "REP_CLASS_SALARY_ONLY=true\n" );

  if( rt->initiateCalls )
    fprintf( f, "REP_CLASS_INITIATE_CALLS=true\n" );

  if( rt->autoReplace )
    fprintf( f, "REP_CLASS_AUTO_REPLACE=true\n" );

  fprintf( f, "\n" );
  }

void AddProductToRepClass( _SALES_REP_CLASS* rt, _PRODUCT* p )
  {
  if( rt==NULL || p==NULL )
    return;
  if( rt->nProducts==0 || rt->products==NULL )
    {
    rt->nProducts = 1;
    rt->products = (_PRODUCT**)SafeCalloc( 1, sizeof( _PRODUCT* ), "_PRODUCT *" );
    }
  else
    {
    ++ rt->nProducts;
    rt->products = (_PRODUCT**)realloc( rt->products, rt->nProducts * sizeof( _PRODUCT* ) );
    if( rt->products==NULL )
      Error( "Failed to reallocate array to %d products", rt->nProducts );
    }
  rt->products[ rt->nProducts-1 ] = p;
  }

_SALES_REP* NewSalesRep( char* id, _SALES_REP* list )
  {
  /*
  if( EMPTY( id ) )
    Error( "NewSalesRep(NULL)" );
  */
  _SALES_REP* r = SafeCalloc( 1, sizeof( _SALES_REP ), "_SALES_REP" );
  if( NOTEMPTY( id ) )
    r->id = strdup( id );
  r->next = list;
  return r;
  }

_SALES_REP* FindSalesRep( _SALES_REP* list, char* id )
  {
  if( EMPTY( id ) )
    return NULL;
  for( _SALES_REP* r = list; r!=NULL; r=r->next )
    if( NOTEMPTY( r->id ) && strcasecmp( r->id, id )==0 )
      return r;
  return NULL;
  }

void FreeSalesRep( _SALES_REP* r )
  {
  if( r==NULL )
    return;
  if( r->next!=NULL )
    {
    FreeSalesRep( r->next );
    r->next = NULL;
    }
  FreeIfAllocated( &(r->id) );
  FreeIfAllocated( &(r->name) );

  if( r->workDays!=NULL )
    {
    _SINGLE_DAY* day = r->workDays;
    for( int i=0; i<r->nWorkDays; ++i )
      {
      if( day->dailySales!=NULL )
        FreeRevenueEvent( day->dailySales );
      if( day->fees!=NULL )
        FreePayEvent( day->fees );
      ++day;
      }
    FREE( r->workDays );
    }

  if( r->monthlySummary!=NULL )
    FREE( r->monthlySummary );

  free( r );
  }

/* If end date happens before simulation end and if
 * rep class calls for auto replace, then create a new sales
 * rep.  Maybe do that in another function before here,
 * so that validation works for that one too.  Or maybe stick
 * it on the end of the list, so the caller will get to it.
 * Don't forget salary escalation.
 */
void AppendReplacementSalesRepInList( _CONFIG* config, _SALES_REP* r )
  {
  Event( "Rep %s ends on %04d-%02d-%02d - auto-replacing",
          r->id, r->lastDay.year, r->lastDay.month, r->lastDay.day );

  if( r->class->averageEmploymentMonths<1 )
    Error( "Try to automatically replace rep %s but class REP_CLASS_AVG_MONTHS_EMPLOYMENT<1", r->id );

  /* when does the new guy start? */
  int avgDays = config->daysToAutoReplaceRepAvg;
  int sdevDays = config->daysToAutoReplaceRepSDev;
  if( avgDays<=0 || sdevDays<0 )
    Error( "Trying to replace rep %s at %04d-%02d-%02d but replacement timing variables are wonky",
           r->id, r->lastDay.year, r->lastDay.month, r->lastDay.day );
  int nDays = (int)(RandN2( (double)avgDays, (double)sdevDays ) + 0.5 );

  time_t lastGuyStartTime = MMDDToTime( &(r->firstDay) );
  time_t lastGuyEndTime = MMDDToTime( &(r->lastDay) );
  time_t tStart = lastGuyEndTime + nDays * DAY_IN_SECONDS;

  /* when does the new guy end? */
  int nMonths = (int)RandN2( r->class->averageEmploymentMonths, r->class->sdevEmploymentMonths );
  time_t tEnd = tStart + 30 * nMonths * DAY_IN_SECONDS;
  if( tEnd > config->simulationEnd )
    tEnd = config->simulationEnd;

  if( tEnd <= tStart )
    Event( "Tried to auto-replace rep %s but too close to end of sim.", r->id );
  else
    {
    /* create the new guy */
    _SALES_REP* newRep = NewSalesRep( NULL, NULL );
    memcpy( newRep, r, sizeof( _SALES_REP ) );
    newRep->next = NULL;
    newRep->nMonths = 0;
    newRep->monthlySummary = NULL;

    int oldNum = 0;
    char cleanOldID[BUFLEN/2];
    strncpy( cleanOldID, r->id, sizeof(cleanOldID)-1 );
    char* ptr = strstr( cleanOldID, " (" );
    if( ptr!=NULL )
      {
      (void)sscanf( ptr, " (%d)", &oldNum );
      *ptr = 0;
      }
    int newNum = oldNum + 1;

    char repID[BUFLEN];
    snprintf( repID, sizeof(repID)-2, "%s (%d)", cleanOldID, newNum );
    newRep->id = strdup( repID );

    char cleanOldName[BUFLEN/2];
    strncpy( cleanOldName, r->name, sizeof(cleanOldName)-1 );
    ptr = strstr( cleanOldName, " (" );
    if( ptr!=NULL )
      *ptr = 0;

    char repName[BUFLEN];
    snprintf( repName, sizeof(repName), "%s (%d)", cleanOldName, newNum );
    newRep->name = strdup( repName );

    if( TimeToMMDD( tStart, &(newRep->firstDay) )!=0 )
      Error( "Failed to set start date for %s", newRep->id );
    if( TimeToMMDD( tEnd, &(newRep->lastDay) )!=0 )
      Error( "Failed to set end date for rep %s", r->id );

    /* pay may be a bit higher */
    double annualPay = r->annualPay;
    long secondsElapsed = tStart - lastGuyStartTime;
    long daysElapsed = secondsElapsed / DAY_IN_SECONDS;
    double yearsElapsed = (double)daysElapsed / 365.0; 
    annualPay *= pow( 1.0 + r->class->annualPayIncreasePercent / 100.0, yearsElapsed );
    newRep->annualPay = round( annualPay );
    newRep->monthlyPay = round( annualPay / 12.0 );
    
    /* add the new rep to the config */
    ++ config->nSalesReps;
    _SALES_REP **sPtr;
    for( sPtr = &r->next;
         sPtr!=NULL && *(sPtr)!=NULL;
         sPtr = &( (*sPtr)->next ) )
      ;
    if( sPtr!=NULL && *sPtr==NULL )
      *sPtr = newRep;
    else
      Error( "Tried to append a sales rep but got a NULL ptr-ptr" );
    }
  }

int ValidateSingleSalesRep( _SALES_REP* r, _CONFIG* config )
  {
  if( r==NULL || EMPTY( r->id ) )
    return -100;

  if( r->class==NULL )
    {
    Warning( "Rep %s has no rep class!", r->id );
    return -5;
    }

  if( EMPTY( r->name ) )
    {
    Warning( "Sales rep class %s has no name", r->id );
    return -1;
    }

  if( EmptyMMDD( &(r->firstDay) )==0 )
    {
    Warning( "Rep %s has no starting date", r->id );
    return -2;
    }

  if( EmptyMMDD( &(r->lastDay) )==0 )
    Error( "Rep %s has no SALES_REP_FINISH date.  You can use 'end-of-sim' or 'random'", r->id );

  if( MMDDToTime( &(r->lastDay ) ) > config->simulationEnd )
    {
    Warning( "Rep %s ends work after the simulation is over!  Resetting.", r->id );
    (void)TimeToMMDD( config->simulationEnd, &(r->lastDay) );
    }

  time_t tStart = MMDDToTime( &(r->firstDay) );
  time_t tFinish = MMDDToTime( &(r->lastDay) );
  if( tStart >= tFinish )
    {
    Warning( "Rep %s has out-of-order start and end dates! start=%04d-%02d-%02d, end=%04d-%02d-%02d",
             r->id,
             r->firstDay.year,
             r->firstDay.month,
             r->firstDay.day,
             r->lastDay.year,
             r->lastDay.month,
             r->lastDay.day
             );
    return -3;
    }

  if( r->class->autoReplace
      && MMDDToTime( &(r->lastDay ) ) <= (config->simulationEnd - DAY_IN_SECONDS) )
    { /* we need a new rep to replace this one - create one and put him at the tail end of the list of reps. */
    AppendReplacementSalesRepInList( config, r );
    }

  if( r->annualPay==0 )
    {
    Warning( "Rep %s has no annual salary!", r->id );
    return -4;
    }

  if( r->monthlyPay==0 )
    r->monthlyPay = r->annualPay / 12;

  if( ! r->salaryOnly )
    {
    if( r->dailyCalls<1 )
      {
      Warning( "Rep %s has no steady state daily call volume!", r->id );
      return -6;
      }
    }

  return 0;
  }

void PrintSalesRep( FILE* f, _SALES_REP* r )
  {
  if( f==NULL || r==NULL || EMPTY( r->id ) )
    return;
  fprintf( f, "SALES_REP=%s\n", r->id );
  if( NOTEMPTY( r->name ) )
    fprintf( f, "SALES_REP_NAME=%s\n", r->name );
  if( r->class!=NULL && NOTEMPTY( r->class->id ) )
    fprintf( f, "SALES_REP_CLASS=%s\n", r->class->id );
  fprintf( f, "SALES_REP_START=%04d-%02d-%02d\n", r->firstDay.year, r->firstDay.month, r->firstDay.day );
  fprintf( f, "SALES_REP_FINISH=%04d-%02d-%02d\n", r->lastDay.year, r->lastDay.month, r->lastDay.day );
  fprintf( f, "SALES_REP_ANNUAL_SALARY=%.1f\n", r->annualPay );
  fprintf( f, "SALES_REP_DAILY_CALLS=%d\n", r->dailyCalls );
  if( r->salaryOnly )
    fprintf( f, "SALES_REP_SALARY_ONLY=true\n" );
  if( r->handoffFee )
    fprintf( f, "SALES_REP_HANDOFF_FEE=%.1lf\n", r->handoffFee );
  fprintf( f, "\n" );
  }

void PrintRevenueSummaryForRep( FILE* out, _SALES_REP* s )
  {
  if( out==NULL || s==NULL || EMPTY( s->id ) || s->monthlySummary==NULL )
    return;

  char title[BUFLEN];
  snprintf( title, sizeof(title)-1, "%s - %s", s->id, NULLPROTECT( s->name ) );

  PrintRevenueSummary( out, s->monthlySummary, s->nMonths, title );
  }

int CountSalesRepsInClass( _CONFIG* conf, _SALES_REP_CLASS* class )
  {
  int n=0;
  if( conf==NULL || class==NULL )
    return -1;

  for( _SALES_REP* rep = conf->salesReps; rep!=NULL; rep=rep->next )
    {
    if( rep->class==class )
      ++n;
    }

  return n;
  }

_SALES_REP* RandomRepInClass( _CONFIG* conf, _SALES_REP_CLASS* class )
  {
  int n = CountSalesRepsInClass( conf, class );
  if( n<=0 )
    return NULL;

  _SALES_REP** reps = (_SALES_REP**)SafeCalloc( n, sizeof(_SALES_REP*), "Sales rep array" );
  int i =  0;
  for( _SALES_REP* rep = conf->salesReps; rep!=NULL; rep=rep->next )
    if( rep->class==class && i<n )
      reps[i++] = rep;
  if( i!=n )
    Error( "Tried to build an array of reps in a class but got %d/%d", i, n );

  int r = lrand48() % n;
  _SALES_REP* retVal = reps[r];
  free( reps );
  return retVal;
  }

int CompareReps( const void* a, const void* b )
  {
  _SALES_REP** repAPtr = (_SALES_REP**)a;
  _SALES_REP** repBPtr = (_SALES_REP**)b;
  _SALES_REP* repA = *repAPtr;
  _SALES_REP* repB = *repBPtr;
  return (repA->seq < repB->seq) ? -1 : 1;
  }

_SALES_REP** SalesRepArray( _SALES_REP* list )
  {
  if( list==NULL )
    return NULL;

  int n = 0;
  for( _SALES_REP* rep = list; rep!=NULL; rep=rep->next )
    ++n;

  if( n<1 )
    return NULL;

  _SALES_REP** array = (_SALES_REP**)SafeCalloc( n, sizeof( _SALES_REP* ), "Rep array" );
  int i = 0;
  for( _SALES_REP* rep = list; rep!=NULL && i<n; rep=rep->next )
    array[i++] = rep;

  qsort( array, n, sizeof( _SALES_REP* ), CompareReps );

  return array;
  }

_CLASS_POINTER* NewClassPointer( _SALES_REP_CLASS* class, _CLASS_POINTER* list )
  {
  if( class==NULL )
    Error( "Cannot create a pointer to a NULL sales rep class" );

  _CLASS_POINTER* cp = (_CLASS_POINTER*)SafeCalloc( 1, sizeof(_CLASS_POINTER), "Sales rep class pointer" );
  cp->class = class;
  cp->next = list;
  return cp;
  }

void FreeClassPointer( _CLASS_POINTER* list )
  {
  if( list==NULL )
    return;
  if( list->next )
    FreeClassPointer( list->next );
  free( list );
  }

int RepClassInList( _SALES_REP_CLASS* class, _CLASS_POINTER* list )
  {
  if( class==NULL && list==NULL )
    return 0;
  if( class==NULL || list==NULL )
    return -1;
  for( _CLASS_POINTER* ptr=list; ptr!=NULL; ptr=ptr->next )
    if( ptr->class==class )
      return 0;
  return -2;
  }

void GenerateClassList( char* listBuf, size_t bufLen, _CLASS_POINTER* list )
  {
  char* ptr = listBuf;
  char* end = listBuf + bufLen - 2;
  for( _CLASS_POINTER* cp=list; cp!=NULL; cp=cp->next )
    {
    if( cp->class!=NULL
        && cp->class->id!=NULL )
      {
      if( ptr>listBuf && ptr<end-1 )
        {
        *(ptr++) = ',';
        *(ptr++) = ' ';
        }

      int l = strlen( cp->class->id );

      if( ptr+l<end )
        {
        strcpy( ptr, cp->class->id );
        ptr += strlen( ptr );
        }

      *ptr = 0;
      }
    }
  }

_SALES_REP* RandomRepFromClassList( _CONFIG* conf, _CLASS_POINTER* repClasses, time_t tWhen )
  {
  if( conf==NULL || repClasses==NULL )
    return NULL;

  int n = 0;
  for( _SALES_REP* rep = conf->salesReps; rep!=NULL; rep=rep->next )
    {
    time_t tStart = MMDDToTime( &(rep->firstDay) );
    time_t tEnd = MMDDToTime( &(rep->lastDay) );
    if( tStart <= tWhen
        && tWhen <= tEnd
        && RepClassInList( rep->class, repClasses )==0 )
      ++n;
    }

  if( n==0 )
    {
    char listBuf[BUFLEN];
    GenerateClassList( listBuf, sizeof(listBuf), repClasses );
    _MMDD niceDate;
    if( TimeToMMDD( tWhen, &niceDate )!=0 )
      Error( "Failed to generate nice date from %ld", (long)tWhen );
    Warning( "Seeking reps from classes [%s] - none found at %04d-%02d-%02d",
             listBuf, niceDate.year, niceDate.month, niceDate.day );
    return NULL;
    }

  int repNum = lrand48() % n;

  int i = 0;
  for( _SALES_REP* rep = conf->salesReps; rep!=NULL; rep=rep->next )
    {
    time_t tStart = MMDDToTime( &(rep->firstDay) );
    time_t tEnd = MMDDToTime( &(rep->lastDay) );
    if( tStart <= tWhen
        && tWhen <= tEnd
        && RepClassInList( rep->class, repClasses )==0 )
      {
      if( i==repNum )
        return rep;
      ++i;
      }
    }

  Error( "RandomRepFromClassList -> none found" );
  return NULL; /* just to keep the compiler happy */
  }
