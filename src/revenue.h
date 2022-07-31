#ifndef _INCLUDE_REVENUE
#define _INCLUDE_REVENUE

typedef struct _revenueEvent
  {
  int customerNumber;
  int eventNumber;
  double revenue;
  struct _revenueEvent* next;
  } _REVENUE_EVENT;

_REVENUE_EVENT* NewRevenueEvent( _CONFIG* conf, _MMDD when, int customerNumber, _SALES_REP* s, int eventNumber, double revenue, _REVENUE_EVENT* list );
void FreeRevenueEvent( _REVENUE_EVENT* r );

enum pay_type { pt_invalid, pt_commission, pt_salary, pt_revenue };

typedef struct _salesRep _SALES_REP;
typedef struct _cashEvent _CASH_EVENT;

typedef struct _payEvent
  {
  _SALES_REP* rep;
  enum pay_type type;
  double amount;
  struct _payEvent* next;
  } _PAY_EVENT;

_PAY_EVENT* NewPayEvent( _CONFIG* conf, _MMDD when, _SALES_REP* s, enum pay_type type, double amount, _PAY_EVENT* list );
void FreePayEvent( _PAY_EVENT* p );

typedef struct _singleDay
  {
  _MMDD date;
  time_t t;
  int working;
  int nCalls;
  int maxCalls;
  _REVENUE_EVENT* dailySales;
  _PAY_EVENT* fees;
  _CASH_EVENT* cashEvents;
  _MONTHLY_SUMMARY* month;
  double cashOnHand;
  } _SINGLE_DAY;

char* PayTypeName( enum pay_type type );
_SINGLE_DAY* FindSingleDay( _MMDD* date, _SINGLE_DAY* array, int arrayLen );

#endif
