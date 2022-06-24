#include "base.h"

_HOLIDAY* NewHoliday( char* id, _HOLIDAY* list )
  {
  if( EMPTY( id ) )
    Error( "NewHoliday(NULL)" );
  _HOLIDAY* h = SafeCalloc( 1, sizeof( _HOLIDAY ), "_HOLIDAY" );
  if( NOTEMPTY( id ) )
    h->id = strdup( id );
  h->next = list;
  return h;
  }

_HOLIDAY* FindHoliday( _HOLIDAY* list, char* id )
  {
  if( EMPTY( id ) )
    return NULL;
  for( _HOLIDAY* h = list; h!=NULL; h=h->next )
    if( NOTEMPTY( h->id ) && strcasecmp( h->id, id )==0 )
      return h;
  return NULL;
  }

void FreeHoliday( _HOLIDAY* h )
  {
  if( h==NULL )
    return;
  if( h->next!=NULL )
    {
    FreeHoliday( h->next );
    h->next = NULL;
    }
  FreeIfAllocated( &(h->id) );
  FreeIfAllocated( &(h->name) );

  free( h );
  }

/* assumes that no holidays start on or before 12/31 and end on or after 1/1 */
int FallsOnHoliday( _HOLIDAY* h, _MMDD* day )
  {
  if( h==NULL )
    return -1;

  if( FallsOnHoliday( h->next, day )==0 )
    return 0;

  int first = 0;
  int question = 0;
  int last = 0;

  if( h->firstDay.year==0 || day->year==0 )
    {
    first = h->firstDay.month * 100 + h->firstDay.day;
    question = day->month * 100 + day->day;
    last = h->lastDay.month * 100 + h->lastDay.day;
    }
  else
    {
    first = h->firstDay.year * 10000 + h->firstDay.month * 100 + h->firstDay.day;
    question = day->year * 10000 + day->month * 100 + day->day;
    last = h->lastDay.year * 10000 + h->lastDay.month * 100 + h->lastDay.day;
    }

  if( question<first )
    return -2;

  if( question>last )
    return -3;

  return 0;
  }

int ValidateSingleHoliday( _HOLIDAY* h )
  {
  if( h==NULL || EMPTY( h->id ) )
    return -100;

  if( EMPTY( h->name ) )
    {
    Warning( "Holiday %s has no name", h->id );
    return -1;
    }

  if( h->firstDay.month==0 || h->firstDay.day==0 )
    {
    Warning( "Holiday %s has no start date", h->id );
    return -2;
    }

  if( h->lastDay.month==0 || h->lastDay.day==0 )
    {
    Warning( "Holiday %s has no finish date", h->id );
    return -3;
    }

  if( h->firstDay.day<1 || h->firstDay.day>31 )
    {
    Warning( "Holiday %s has weird day for start", h->id );
    return -4;
    }

  if( h->lastDay.day<1 || h->lastDay.day>31 )
    {
    Warning( "Holiday %s has weird day for finish", h->id );
    return -5;
    }

  if( h->firstDay.month<1 || h->firstDay.month>12 )
    {
    Warning( "Holiday %s has weird day for start", h->id );
    return -6;
    }

  if( h->lastDay.month<1 || h->lastDay.month>12 )
    {
    Warning( "Holiday %s has weird day for finish", h->id );
    return -7;
    }

  if( h->firstDay.month * 100 + h->firstDay.day
      > h->lastDay.month * 100 + h->lastDay.day )
    {
    Warning( "Holiday %s has out-of-order start and finish (perhaps split in two?)", h->id );
    return -8;
    }

  if( h->firstDay.year!=0 && h->lastDay.year==0 )
    {
    Warning( "Holiday %s has a year on the start date but no year on the end date", h->id );
    return -9;
    }

  if( h->firstDay.year==0 && h->lastDay.year!=0 )
    {
    Warning( "Holiday %s has a year on the end date but no year on the start date", h->id );
    return -10;
    }

  return 0;
  }

void PrintHoliday( FILE* f, _HOLIDAY* h )
  {
  if( f==NULL || h==NULL || EMPTY( h->id ) )
    return;

  fprintf( f, "HOLIDAY=%s\n", h->id );
  if( NOTEMPTY( h->name ) )
    fprintf( f, "HOLIDAY_NAME=%s\n", h->name );
  if( EmptyMMDD( &(h->firstDay) )!=0 )
    {
    if( h->firstDay.year!=0 )
      fprintf( f, "HOLIDAY_START=%04d-%02d-%02d\n", h->firstDay.year, h->firstDay.month, h->firstDay.day );
    else
      fprintf( f, "HOLIDAY_START=%02d-%02d\n", h->firstDay.month, h->firstDay.day );
    }
  if( EmptyMMDD( &(h->lastDay) )!=0 )
    {
    if( h->lastDay.year!=0 )
      fprintf( f, "HOLIDAY_START=%04d-%02d-%02d\n", h->lastDay.year, h->lastDay.month, h->lastDay.day );
    else
      fprintf( f, "HOLIDAY_START=%02d-%02d\n", h->lastDay.month, h->lastDay.day );
    }
  fprintf( f, "\n" );
  }

