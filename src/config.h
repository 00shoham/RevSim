#ifndef _INCLUDE_CONFIG
#define _INCLUDE_CONFIG

#define CONFIGDIR "/usr/local/etc/"
#define CONFIGFILE "revsim.ini"
#define DEFAULT_PORT 80

typedef struct _config
  {
  char* configFolder;

  int currentlyParsing;
  _TAG_VALUE* parserLocation;

  /* to avoid duplicate includes */
  _TAG_VALUE *includes;

  /* macro expansion */
  _TAG_VALUE *list;

  /* used in installers and diagnostics */
  int listIncludes;
  int includeCounter;

  _MMDD simulationFirstDay;
  time_t simulationStart;        /* calculated */
  int simulationDurationDays;
  time_t simulationEnd;          /* calculated */
  _MMDD simulationEndDay;        /* calculated */

  _HOLIDAY* holidays;            /* stat holidays for everyone */
  _VACATION* vacations;          /* classes of vacation allocation */
  _SINGLE_DAY* baselineWorkDays;  /* calculated */
  int nWorkDaysPerYear;

  _SALES_STAGE* stages;

  _PRODUCT* products;
  _SALES_REP_CLASS* salesRepClasses;
  _SALES_REP* salesReps;
  int nSalesReps;
  _SALES_REP* customerCare;
  int daysToAutoReplaceRepAvg;
  int daysToAutoReplaceRepSDev;

  int nMonths;
  _MONTHLY_SUMMARY* monthlySummary;

  double percentageForPaymentProcessing;
  double averageCollectionsDelayDays;
  double sdevCollectionsDelayDays;

  /* if the market size is fixed/limited, use this stuff */
  int marketSize;
  int orgCoolingPeriodDays;
  int nAvailableOrgs;
  _ORG* orgs;

  /* stats */
  int nCustomerMonths;
  int nCustomerWins;

  /* if the market size is unbounded, just use this: */
  int customerNumber;

  /* cashflow modeling */
  double taxRate;
  double initialCashBalance;
  _CASH_EVENT* cashEvents;
  int nCashEvents;
  _CASH_EVENT* cashEventArray;
  } _CONFIG;

void SetDefaults( _CONFIG* config );
void ReadConfig( _CONFIG* config, char* filePath );
void PrintConfig( FILE* f, _CONFIG* config );
void FreeConfig( _CONFIG* config );
void PrintVariable( _CONFIG* config, char* varName );
void ValidateConfig( _CONFIG* config );

#endif
