#ifndef _INCLUDE_SALES_REP
#define _INCLUDE_SALES_REP

typedef struct _salesRepClass
  {
  char* id;
  char* name;
  double productivity[MONTHS]; /* 0=none, 1=100% */

  int autoReplace;   /* if a rep quits, do we automatically hire a replacement?   */
  int salaryOnly;    /* not really a sales rep. just used to model salary expense */

  int initiateCalls; /* cold call customers or only take warm leads? */

  double commission;           /* 0.0 to 1.0 - fraction of each sale */
  int commissionMonths;        /* how many months of revenue trigger commission? */

  double annualPayIncreasePercent;

  double averageEmploymentMonths;
  double sdevEmploymentMonths;

  int nProducts;
  _PRODUCT** products;          /* what does this rep type sell? */

  _VACATION* allowance;        /* how many vacation days are allowed? */

  struct _salesRepClass* next;
  } _SALES_REP_CLASS;

_SALES_REP_CLASS* NewSalesRepClass( char* id, _SALES_REP_CLASS* list );
_SALES_REP_CLASS* FindSalesRepClass( _SALES_REP_CLASS* list, char* id );
void FreeSalesRepClass( _SALES_REP_CLASS* rt );
int ValidateSingleSalesRepClass( _SALES_REP_CLASS* rt );
void PrintSalesRepClass( FILE* f, _SALES_REP_CLASS* rt );
void AddProductToRepClass( _SALES_REP_CLASS* rt, _PRODUCT* p );

typedef struct _salesRep
  {
  char* id;
  char* name;
  _SALES_REP_CLASS* class;

  int seq;

  int salaryOnly;    /* not really a sales rep. just used to model salary expense */

  double handoffFee; /* if an opportunity is handed off to another rep, what is this one paid? */

  /* employment */
  _MMDD firstDay;
  _MMDD lastDay;
  double annualPay;
  double monthlyPay; /* calculated */
  int dailyCalls;

  int nWorkDays;
  _SINGLE_DAY* workDays;         /* calculated */
  _SINGLE_DAY* endOfWorkDays;          /* calculated */
  int productNum;               /* simulation state variable */

  int nMonths;
  _MONTHLY_SUMMARY* monthlySummary;

  struct _salesRep* next;
  } _SALES_REP;

_SALES_REP* NewSalesRep( char* id, _SALES_REP* list );
_SALES_REP* FindSalesRep( _SALES_REP* list, char* id );
void FreeSalesRep( _SALES_REP* r );
int ValidateSingleSalesRep( _SALES_REP* r, _CONFIG* config );
void PrintSalesRep( FILE* f, _SALES_REP* r );
void PrintRevenueSummaryForRep( FILE* out, _SALES_REP* s );
_SALES_REP* RandomRepInClass( _CONFIG* conf, _SALES_REP_CLASS* class );
_SALES_REP* RandomRepFromClassList( _CONFIG* conf, _CLASS_POINTER* repClasses, time_t tWhen );
_SALES_REP** SalesRepArray( _SALES_REP* list );

typedef struct _classPointer
  {
  _SALES_REP_CLASS* class;
  struct _classPointer* next;
  } _CLASS_POINTER;

_CLASS_POINTER* NewClassPointer( _SALES_REP_CLASS* class, _CLASS_POINTER* list );
void FreeClassPointer( _CLASS_POINTER* list );
int SalesRepInIndicatedClass( _CLASS_POINTER* cp, _SALES_REP* rep );

typedef struct _repPointer
  {
  _SALES_REP* rep;
  struct _repPointer* next;
  } _REP_POINTER;

_REP_POINTER* SRPushOnStack( _REP_POINTER* stack, _SALES_REP* newRep );
_SALES_REP* SRPeekFromStack( _REP_POINTER* stack );
_REP_POINTER* SRPopFromStack( _REP_POINTER* stack, _SALES_REP** topOfStack );
void SRFreeStack( _REP_POINTER* stack );

#endif
