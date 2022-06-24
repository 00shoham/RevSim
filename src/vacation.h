#ifndef _INCLUDE_VACATION
#define _INCLUDE_VACATION

typedef struct _vacation
  {
  char* id;
  char* name;
  int daysPerYear;
  struct _vacation* next;
  } _VACATION;

_VACATION* NewVacation( char* id, _VACATION* list );
_VACATION* FindVacation( _VACATION* list, char* id );
void FreeVacation( _VACATION* list );
int ValidateSingleVacation( _VACATION* v );
void PrintVacation( FILE* f, _VACATION* v );

#endif
