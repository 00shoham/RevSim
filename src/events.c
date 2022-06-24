#include "base.h"

extern FILE* events;

void Event( const char* fmt, ... )
  {
  if( events==NULL )
    return;

  va_list arglist;
  char buf[BIGBUF];

  va_start( arglist, fmt );
  vsnprintf( buf, sizeof(buf), fmt, arglist );
  va_end( arglist );

  fputs( buf, events );
  fputs( "\n", events );
  }

