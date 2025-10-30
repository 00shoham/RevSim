#include "base.h"

_SALES_STAGE* NewSalesStage( char* id, _SALES_STAGE* list )
  {
  if( EMPTY( id ) )
    Error( "NewSalesStage(NULL)" );
  _SALES_STAGE* s = (_SALES_STAGE*)SafeCalloc( 1, sizeof( _SALES_STAGE ), "_SALES_STAGE" );
  if( NOTEMPTY( id ) )
    s->id = strdup( id );
  s->next = list;
  return s;
  }

_SALES_STAGE* FindSalesStage( _SALES_STAGE* list, char* id )
  {
  if( EMPTY( id ) )
    return NULL;

  for( _SALES_STAGE* s = list; s!=NULL; s=s->next )
    {
    if( NOTEMPTY( s->id )
        && strcasecmp( s->id, id )==0 )
      return s;
    }

  return NULL;
  }

void FreeSalesStage( _SALES_STAGE* s )
  {
  if( s==NULL )
    return;
  if( s->next!=NULL )
    {
    FreeSalesStage( s->next );
    s->next = NULL;
    }
  FreeIfAllocated( &(s->id) );
  FreeIfAllocated( &(s->name) );

  FreeClassPointer( s->repClasses );

  free( s );
  }

int ValidateSingleStage( _SALES_STAGE* s )
  {
  if( s==NULL )
    return -1;

  if( EMPTY( s->id ) )
    {
    Warning( "Sales stage with no ID" );
    return -2;
    }

  if( EMPTY( s->name ) )
    {
    Warning( "Sales stage %s has no name", s->id );
    return -3;
    }

  if( s->daysDelayAverage<0 || s->daysDelayAverage>365 )
    {
    Warning( "Sales stage %s has no/invalid days delay average", s->id );
    return -5;
    }

  if( s->sdevDaysDelay<0 || s->sdevDaysDelay>100 )
    {
    Warning( "Sales stage %s has no/invalid days delay s.deviation", s->id );
    return -6;
    }

  if( s->percentAttrition<=0 || s->percentAttrition>100 )
    {
    Warning( "Sales stage %s has no/invalid percent attrition", s->id );
    return -7;
    }

  return 0;
  }

void PrintSalesStage( FILE* f, _SALES_STAGE* s )
  {
  if( f==NULL || s==NULL || EMPTY( s->id ) )
    return;

  fprintf( f, "STAGE=%s\n", s->id );
  if( NOTEMPTY( s->name ) )
    fprintf( f, "STAGE_NAME=%s\n", s->name );
  if( s->predecessor!=NULL && NOTEMPTY( s->predecessor->id ) )
    fprintf( f, "STAGE_FOLLOWS=%s\n", s->predecessor->id );
  if( s->daysDelayAverage > 0 )
    fprintf( f, "STAGE_DAYS_AVG=%.1lf\n", s->daysDelayAverage  );
  if( s->sdevDaysDelay > 0 )
    fprintf( f, "STAGE_DAYS_SDEV=%.1lf\n", s->sdevDaysDelay );
  if( s->percentAttrition > 0 )
    fprintf( f, "STAGE_ATTRITION_PERCENT=%.1lf\n", s->percentAttrition );
  if( s->connectAttemptsAverage > 0 )
    fprintf( f, "STAGE_CONNECT_ATTEMPTS_AVG=%.1lf\n", s->connectAttemptsAverage );
  if( s->sdevConnectAttempts > 0 )
    fprintf( f, "STAGE_CONNECT_ATTEMPTS_SDEV=%.1lf\n", s->sdevConnectAttempts );
  if( s->connectRetryDaysAverage > 0 )
    fprintf( f, "STAGE_CONNECT_RETRY_DAYS_AVG=%.1lf\n", s->connectRetryDaysAverage );
  if( s->sdevConnectRetryDays > 0 )
    fprintf( f, "STAGE_CONNECT_RETRY_DAYS_SDEV=%.1lf\n", s->sdevConnectRetryDays );
  fprintf( f, "\n" );
  }

int StageDepends( _SALES_STAGE* sa, _SALES_STAGE* sb )
  {
  if( sa==NULL || sb==NULL )
    Error( "StageDepends( %s, %s )", sa==NULL?"NULL":"ptr", sb==NULL?"NULL":"ptr" );

  for( _SALES_STAGE* deps = sa->predecessor; deps!=NULL; deps=deps->predecessor )
    if( deps==sb )
      return 0;
  return -1;
  }

int SSCompareFunc( const void* a, const void* b )
  {
  _SALES_STAGE** saPtr = (_SALES_STAGE**)a;
  _SALES_STAGE** sbPtr = (_SALES_STAGE**)b;
  _SALES_STAGE* sa = *saPtr;
  _SALES_STAGE* sb = *sbPtr;

  if( StageDepends( sa, sb )==0 )
    return 1;
  else
    return -1;
  }

/* QQQ this disregards 'follows'  - fix! */
int SalesStagesArray( _SALES_STAGE* firstStage, _SALES_STAGE*** arrayPtr )
  {
  int nItems = 0;
  for( _SALES_STAGE* s = firstStage; s!=NULL; s=s->successor )
    ++nItems;

  _SALES_STAGE** array = NULL;
  *arrayPtr = array
    = (_SALES_STAGE**)SafeCalloc( nItems, sizeof(_SALES_STAGE*), "SALES_STAGE array" );

  int i = 0;
  for( _SALES_STAGE* s = firstStage; s!=NULL; s=s->successor )
    {
    array[i] = s;
    ++i;
    }

  return nItems;
  }

int SalesRepInIndicatedClass( _CLASS_POINTER* cp, _SALES_REP* rep )
  {
  if( cp==NULL && rep==NULL ) /* weird NULL case? */
    return 0;

  if( cp==NULL || rep==NULL ) /* one arg is NULL, other isn't */
    return -1;

  if( EMPTY( rep->id ) )
    Error( "Rep with no ID" );

  if( rep->class==NULL ) /* validation error */
    Error( "Rep %s has no class", rep->id );

  for( _CLASS_POINTER* ptr=cp; ptr!=NULL; ptr=ptr->next )
    {
    if( cp->class==rep->class )
      return 0; /* match */
    }

  return -2; /* no match */
  }

