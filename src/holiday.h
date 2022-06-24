#ifndef _INCLUDE_HOLIDAY
#define _INCLUDE_HOLIDAY

typedef struct _holiday
  {
  char* id;
  char* name;
  _MMDD firstDay;
  _MMDD lastDay;
  struct _holiday* next;
  } _HOLIDAY;

_HOLIDAY* NewHoliday( char* id, _HOLIDAY* list );
_HOLIDAY* FindHoliday( _HOLIDAY* list, char* id );
void FreeHoliday( _HOLIDAY* list );
int FallsOnHoliday( _HOLIDAY* list, _MMDD* day );
int ValidateSingleHoliday( _HOLIDAY* h );
void PrintHoliday( FILE* f, _HOLIDAY* h );

#endif
