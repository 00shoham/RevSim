#ifndef _INCLUDE_PRODUCT
#define _INCLUDE_PRODUCT

typedef struct _product
  {
  char* id;
  char* name;
  double averageMonthlyDealSize;
  double dealSizeStandardDeviation;
  double monthlyGrowthRatePercent;
  double averageMonthsToReachSteadyState;
  double sdevMonthsToReachSteadyState;
  double probabilityOfCustomerAttritionPerMonth;

  int nSalesStages;
  _SALES_STAGE** stageArray;
  struct _product* next;
  } _PRODUCT;

_PRODUCT* NewProduct( char* id, _PRODUCT* list );
_PRODUCT* FindProduct( _PRODUCT* list, char* id );
void FreeProduct( _PRODUCT* p );
int ValidateSingleProduct( _PRODUCT* p );
void PrintProduct( FILE* f, _PRODUCT* p );

#endif
