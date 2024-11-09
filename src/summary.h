#ifndef _INCLUDE_SUMMARY
#define _INCLUDE_SUMMARY

typedef struct _org _ORG;
typedef struct _product _PRODUCT;

typedef struct _monthlyUnits
  {
  _PRODUCT* product;
  _ORG* customer;
  int nUnits;
  struct _monthlyUnits *next; 
  } _MONTHLY_UNITS;

typedef struct _monthlySummary
  {
  _MMDD monthStart;
  double revenue;
  double commission;
  double salary;
  double expense;
  int nCalls;         /* e.g., made by a rep */
  int nAvailableOrgs; /* e.g., who it is legal to call */
  int nCustomers;     /* e.g., who already signed, hasn't quit yet */
  int nWins;          /* e.g., new deals */
  int nRejections;    /* e.g., did not want to close */
  int nLosses;        /* e.g., stopped using */
  int nTransfers;     /* e.g., opportunities moved from cold-call to AM */
  double taxLossCarryForward; /* from previous year.  only set for month==1 (january) */
  _MONTHLY_UNITS* units; /* if we sell any products by unit, then how many to each org in this month? */
  } _MONTHLY_SUMMARY;

_MONTHLY_SUMMARY* FindSummaryRecord( char* subject,
                                     _MONTHLY_SUMMARY* array, int nEntries,
                                     _MMDD* monthStart );
void AddMonthlySummaries( _MONTHLY_SUMMARY* dst, int nDst, _MONTHLY_SUMMARY* src, int nSrc );
void AddToMonthlySummary( _MONTHLY_SUMMARY* array, int nMonths,
                          int year, int month,
                          double expense, double revenue );
void PrintRevenueSummary( FILE* out, _MONTHLY_SUMMARY* ms, int nMonths, char* title );
void PrintCounters( FILE* f, _CONFIG* conf );
double NetIncomeForYear( int year, _MONTHLY_SUMMARY* array, int nMonths );
void AddMonthlySummariesSingleMonth( _CONFIG* conf, int Y, int M );
_MONTHLY_SUMMARY* FindMonthInArray( _MONTHLY_SUMMARY* array, int len, int Y, int M );
double GetTaxLossCarryForward( _CONFIG* conf, int Y );
void SetTaxLossCarryForward( _CONFIG* conf, int Y, double amount );

void SetMonthlyUnits( _CONFIG* conf, _MONTHLY_SUMMARY* month, _PRODUCT* p, _ORG* o, int n );
void FreeMonthlyUnitsList( _MONTHLY_UNITS* list );
void UnitsReport( FILE* f, _CONFIG* conf );
void CallsReport( FILE* f, _CONFIG* conf );

#endif
