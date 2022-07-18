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
  _CASH_EVENT** eventAPtr = (_CASH_EVENT**)a;
  _CASH_EVENT** eventBPtr = (_CASH_EVENT**)b;
  _CASH_EVENT* eventA = *eventAPtr;
  _CASH_EVENT* eventB = *eventBPtr;

  return NumberOfDays( &(eventA->when), &(eventB->when) );
  }

char* CashEventTypeName( enum eventType et )
  {
  switch( et )
    {
    case et_invalid:       return "INVALID-CASH-EVENT-TYPE";
    case et_investment:    return "INVESTMENT";
    case et_grant:         return "GRANT";
    case et_tax_payment:   return "TAX_PAYMENT";
    case et_other_payment: return "OTHER_PAYMENT";
    default:               return "INVALID-CE-TYPE";
    }
  }

void PrintCashEvent( _CASH_EVENT* ce, FILE* f )
  {
  if( ce==NULL || f==NULL )
    return;

  fprintf( f, "%s=%.1lf@%04d-%02d-%02d\n",
           CashEventTypeName( ce->type ),
           ce->value,
           ce->when.year,
           ce->when.month,
           ce->when.day );
  }

void BuildCashEventArray( _CONFIG* conf )
  { 
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

  /* QQQ once we confirm it's alright, move to printconfig */
  ptr = conf->cashEventArray;
  for( int i=0; i<conf->nCashEvents; ++i )
    {
    PrintCashEvent( ptr, stdout );
    ++ptr;
    }
  }

void RecordCashEvents( _CONFIG* conf )
  {
  if( conf==NULL )
    {
    Warning( "RecordCashEvents - no conf" );
    return;
    }

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

  BuildCashEventArray( conf );
  /* QQQ */
  }
