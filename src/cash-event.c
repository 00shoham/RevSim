#include "base.h"

_CASH_EVENT* NewCashEvent( enum eventType type,
                           _MMDD when,
                           double value,
                           struct _cashEvent* list )
  {
  if( type==et_invalid )
    {
    Warning( "Cannot allocate invalid cash event" );
    return list;
    }

  _CASH_EVENT* ce = (_CASH_EVENT*)SafeCalloc( 1, sizeof( _CASH_EVENT ), "CASH_EVENT" );
  ce->next = list;
  ce->type = type;
  ce->value = value;
  ce->when = when;

  return ce;
  }

void FreeCashEventList( _CASH_EVENT* list )
  {
  if( list==NULL )
    return;

  if( list->next!=NULL )
    FreeCashEventList( list->next );

  free( list );
  }

int CountCashEvents( _CASH_EVENT* list )
  {
  int n = 0;
  while( list!=NULL )
    {
    ++n;
    list = list->next;
    }
  return n;
  }

int CompareCashEvents( const void* a, const void* b )
  {
  _CASH_EVENT* eventA = (_CASH_EVENT*)a;
  _CASH_EVENT* eventB = (_CASH_EVENT*)b;

  return -1 * NumberOfDays( &(eventA->when), &(eventB->when) );
  }

char* CashEventTypeName( enum eventType et )
  {
  switch( et )
    {
    case et_invalid:          return "INVALID_CASH_EVENT_TYPE";
    case et_investment:       return "INVESTMENT";
    case et_grant:            return "GRANT";
    case et_tax_refund:       return "TAX_REFUND";
    case et_one_time_income:  return "ONE_TIME_INCOME";
    case et_one_time_expense: return "ONE_TIME_EXPENSE";
    default:                  return "INVALID_CE_TYPE";
    }
  }

void PrintCashEvent( _CASH_EVENT* ce, FILE* f )
  {
  if( ce==NULL || f==NULL )
    return;

  fprintf( f, "%s=%.1lf @ %04d-%02d-%02d\n",
           CashEventTypeName( ce->type ),
           ce->value,
           ce->when.year,
           ce->when.month,
           ce->when.day );
  }

void PrintAllCashEvents( _CONFIG* conf, FILE* f )
  {
  if( conf==NULL || f==NULL || conf->cashEventArray==NULL )
    return;

  _CASH_EVENT* ptr = conf->cashEventArray;

  ptr = conf->cashEventArray;
  for( int i=0; i<conf->nCashEvents; ++i )
    {
    PrintCashEvent( ptr, f );
    ++ptr;
    }
  }

void BuildCashEventArray( _CONFIG* conf )
  { 
  if( conf==NULL )
    return;

  if( conf->cashEventArray!=NULL )
    FREE( conf->cashEventArray );

  int nCashEvents = CountCashEvents( conf->cashEvents );
  if( nCashEvents<=0 )
    return;
  conf->nCashEvents = nCashEvents;
  conf->cashEventArray = (_CASH_EVENT*)SafeCalloc( nCashEvents, sizeof( _CASH_EVENT ), "CASH_EVENT array" );
  _CASH_EVENT* ptr = conf->cashEvents;
  for( int i=0; ptr!=NULL && i<nCashEvents; ++i )
    {
    memcpy( conf->cashEventArray+i, ptr, sizeof( _CASH_EVENT ) );
    (conf->cashEventArray + i)->next = NULL;
    ptr = ptr->next;
    }

  qsort( conf->cashEventArray, nCashEvents, sizeof( _CASH_EVENT ), CompareCashEvents );
  }

void RecordCashEvents( _CONFIG* conf )
  {
  if( conf==NULL )
    {
    Warning( "RecordCashEvents - no conf" );
    return;
    }

  BuildCashEventArray( conf );

  if( conf->baselineWorkDays==NULL )
    {
    Warning( "RecordCashEvents - no baseline work days" );
    return;
    }

  if( conf->initialCashBalance>0 )
    {
    _SINGLE_DAY* firstDay = conf->baselineWorkDays;
    firstDay->cashOnHand = conf->initialCashBalance;
    }

  /* add the one time events to the cash position in each baseline day */
  int eventNum = 0;
  for( _CASH_EVENT* cePtr = conf->cashEventArray;
       eventNum<conf->nCashEvents; ++cePtr, ++eventNum )
    {
    int dayNumber = NumberOfDays( &(conf->simulationFirstDay), &(cePtr->when) ) - 1;
    _SINGLE_DAY* dayPtr = conf->baselineWorkDays + dayNumber;
    switch( cePtr->type )
      {
      case et_investment:
        dayPtr->cashOnHand += cePtr->value;
        break;

      case et_grant:
        dayPtr->cashOnHand += cePtr->value;
        break;

      case et_tax_refund:
        dayPtr->cashOnHand += cePtr->value;
        break;

      case et_one_time_income:
        dayPtr->cashOnHand += cePtr->value;
        break;

      case et_one_time_expense:
        dayPtr->cashOnHand -= cePtr->value;
        break;

      default:
        Error( "Cash event at %04d-%02d-%02d has invalid type",
               cePtr->when.year,
               cePtr->when.month,
               cePtr->when.day );
      }
    }
  }
