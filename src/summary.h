#ifndef _INCLUDE_SUMMARY
#define _INCLUDE_SUMMARY

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

#endif
