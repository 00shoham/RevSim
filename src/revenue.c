#include "base.h"

char* RevTypeName( enum pay_type type )
  {
  switch( type )
    {
    case pt_invalid: return "INVALID";
    case pt_commission: return "Commission";
    case pt_salary: return "Salary";
    case pt_revenue: return "Revenue";
    default: return "INVALID-2";
    }
  }

void AddToSummary( enum pay_type type, _MMDD when, _SALES_REP* s, double amount )
  {
  if( s==NULL )
    {
    Warning( "Cannot add revenue to NULL rep" );
    return;
    }

  if( s->id==NULL )
    {
    Warning( "Cannot add revenue to rep with no ID" );
    return;
    }

  if( s->monthlySummary==NULL )
    {
    Warning( "Cannot add revenue to rep %s with no summary revenue table", s->id );
    return;
    }

  int gotIt = 0;
  _MONTHLY_SUMMARY* ms = NULL;
  for( int k=0; k < s->nMonths; ++k )
    {
    ms = s->monthlySummary + k;
    if( ms->monthStart.year==when.year && ms->monthStart.month==when.month )
      {
      gotIt = 1;
      break;
      }
    }

  if( gotIt==0 )
    {
    Warning( "%s event for unexpected CCYY-MM (%04d-%02d) for %s (scanned %d months from %04d-%02d to %04d-%02d)",
             RevTypeName( type ),
             when.year, when.month,
             s->id,
             s->nMonths,
             s->monthlySummary->monthStart.year,
             s->monthlySummary->monthStart.month,
             (s->monthlySummary + s->nMonths - 1)->monthStart.year,
             (s->monthlySummary + s->nMonths - 1)->monthStart.month
             );
// #if 0 /* used to trigger debugger */
    char* ptr = NULL;
    *ptr = 0;
// #endif
    return;
    }

  switch( type )
    {
    case pt_commission:
      ms->commission += amount;
      break;

    case pt_salary:
      ms->salary += amount;
      break;

    case pt_revenue:
      ms->revenue += amount;
      break;

    default:
      Error( "Called AddToSummary() with bad type" );
    }
  }

_REVENUE_EVENT* NewRevenueEvent( _CONFIG* conf,
                                 _MMDD when,
                                 int customerNumber,
                                 _SALES_REP* s,
                                 int eventNumber,
                                 double revenue,
                                 _REVENUE_EVENT* list )
  {
  _REVENUE_EVENT* p = SafeCalloc( 1, sizeof( _REVENUE_EVENT ), "_REVENUE_EVENT" );
  p->customerNumber = customerNumber;
  p->eventNumber = eventNumber;
  p->revenue = revenue;
  p->next = list;

  AddToSummary( pt_revenue, when, s, revenue );

  RecordDailyIncome( conf, &when, revenue );

  return p;
  }

void FreeRevenueEvent( _REVENUE_EVENT* r )
  {
  if( r==NULL )
    return;
  if( r->next!=NULL )
    {
    FreeRevenueEvent( r->next );
    r->next = NULL;
    }
  free( r );
  }

_PAY_EVENT* NewPayEvent( _CONFIG* conf,
                         _MMDD when,
                         _SALES_REP* s,
                         enum pay_type type, double amount,
                         _PAY_EVENT* list )
  {
  _PAY_EVENT* p = SafeCalloc( 1, sizeof( _PAY_EVENT ), "_PAY_EVENT" );
  p->rep = s;
  p->type = type;
  p->amount = amount;
  p->next = list;

  AddToSummary( type, when, s, amount );

  RecordDailyExpense( conf, &when, amount );

  return p;
  }

void FreePayEvent( _PAY_EVENT* p )
  {
  if( p==NULL )
    return;
  if( p->next!=NULL )
    {
    FreePayEvent( p->next );
    p->next = NULL;
    }
  free( p );
  }

