#ifndef _CASH_EVENT_INCLUDE
#define _CASH_EVENT_INCLUDE

typedef struct _config _CONFIG;

/* more later.  e.g., repayable loans */
enum eventType { et_invalid,
                 et_investment,
                 et_grant,
                 et_tax_refund,
                 et_one_time_income,
                 et_one_time_expense
               };

typedef struct _cashEvent
  {
  enum eventType type;
  _MMDD when;
  double value;
  struct _cashEvent* next;
  } _CASH_EVENT;

_CASH_EVENT* NewCashEvent( enum eventType type,
                           _MMDD when,
                           double value,
                           struct _cashEvent* list );
void FreeCashEventList( _CASH_EVENT* list );
void RecordCashEvents( _CONFIG* conf );
void PrintCashEvent( _CASH_EVENT* ce, FILE* f );
void PrintAllCashEvents( _CONFIG* conf, FILE* f );

#endif
