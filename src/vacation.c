#include "base.h"

_VACATION* NewVacation( char* id, _VACATION* list )
  {
  if( EMPTY( id ) )
    Error( "NewVacation(NULL)" );
  _VACATION* v = (_VACATION*)SafeCalloc( 1, sizeof( _VACATION ), "_VACATION" );
  if( NOTEMPTY( id ) )
    v->id = strdup( id );
  v->next = list;
  return v;
  }

_VACATION* FindVacation( _VACATION* list, char* id )
  {
  if( EMPTY( id ) )
    return NULL;
  for( _VACATION* v = list; v!=NULL; v=v->next )
    if( NOTEMPTY( v->id ) && strcasecmp( v->id, id )==0 )
      return v;
  return NULL;
  }

void FreeVacation( _VACATION* v )
  {
  if( v==NULL )
    return;
  if( v->next!=NULL )
    {
    FreeVacation( v->next );
    v->next = NULL;
    }
  FreeIfAllocated( &(v->id) );
  FreeIfAllocated( &(v->name) );

  free( v );
  }

int ValidateSingleVacation( _VACATION* v )
  {
  if( v==NULL )
    return -1;
  if( EMPTY( v->id ) )
    {
    Warning( "Vacation with no ID" );
    return -2;
    }
  if( EMPTY( v->name ) )
    {
    Warning( "Vacation %s has no name", v->id );
    return -3;
    }
  if( v->daysPerYear<1 )
    {
    Warning( "Vacation %s has too few days", v->id );
    return -4;
    }
  if( v->daysPerYear>100 )
    {
    Warning( "Vacation %s has too many days", v->id );
    return -5;
    }
  return 0;
  }

void PrintVacation( FILE* f, _VACATION* v )
  {
  if( f==NULL || v==NULL || EMPTY( v->id ))
    return;

  fprintf( f, "VACATION=%s\n", v->id );
  if( NOTEMPTY( v->name ) )
    fprintf( f, "VACATION_NAME=%s\n", v->name );
  fprintf( f, "VACATION_DAYS=%d\n", v->daysPerYear );
  fprintf( f, "\n" );
  }
