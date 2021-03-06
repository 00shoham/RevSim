#ifndef _INCLUDE_SALES_PROCESS
#define _INCLUDE_SALES_PROCESS

typedef struct _classPointer _CLASS_POINTER;

typedef struct _salesStage
  {
  char* id;
  char* name;

  int isInitial;
  int isTerminal;

  struct _salesStage* predecessor;
  double daysDelayAverage;
  double daysDelayStandardDeviation;

  double connectAttemptsAverage;
  double connectAttemptsStandardDeviation;
  double connectRetryDaysAverage;
  double connectRetryDaysStandardDeviation;

  double percentAttrition;

  _CLASS_POINTER* repClasses;

  struct _salesStage* next;
  } _SALES_STAGE;

_SALES_STAGE* NewSalesStage( char* id, _SALES_STAGE* list );
_SALES_STAGE* FindSalesStage( _SALES_STAGE* list, char* id );
void FreeSalesStage( _SALES_STAGE* list );
int ValidateSingleStage( _SALES_STAGE* s );
void PrintSalesStage( FILE* f, _SALES_STAGE* s );
int SalesStagesArray( _SALES_STAGE* list, _SALES_STAGE*** arrayPtr );

#endif
