#ifndef _INCLUDE_AVAILABILITY
#define _INCLUDE_AVAILABILITY

int CalculateBaselineWorkingDays( _CONFIG* config );
void PrintAvailability( FILE* out, _CONFIG* conf );
int EstablishWorkDays( _SALES_REP* s, _CONFIG* config );
void PrintWorkDays( FILE* out, _SALES_REP* s );
void InitializeMonthlySummaryArray( char* subject,
                                    _MONTHLY_SUMMARY** msPtr,
                                    int* nMonthsPtr,
                                    _SINGLE_DAY* daysArray,
                                    int nDays,
                                    _MMDD* firstDay,
                                    _MMDD* lastDay );

#endif
