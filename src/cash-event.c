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

