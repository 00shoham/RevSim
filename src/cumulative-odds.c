#include "base.h"

FILE* events = NULL;

int main( int argc, char** argv )
  {
  double p = 0;
  double t = 0;

  for( int i=1; i<argc; ++i )
    {
    if( strcmp( argv[i], "-p" )==0 && i+1<argc )
      {
      ++i;
      p = atof( argv[i] );
      }
    else if( strcmp( argv[i], "-t" )==0 && i+1<argc )
      {
      ++i;
      t = atof( argv[i] );
      }
    else
      {
      fprintf( stderr, "ARG [%s] not recognized\n", argv[i] );
      fprintf( stderr, "USAGE: %s -p 0.0167 -t 0.5\n", argv[0] );
      fprintf( stderr, "i.e., how many types do we have to run the 1.67%% event to get a 50%% result?\n" );
      exit(0);
      }

    }

  if( p<0.01 || p>1 )
    {
    fprintf( stderr, "ERROR: -p must be from 0.01 to 1\n" );
    exit(1);
    }

  if( t<0.01 || t>1 )
    {
    fprintf( stderr, "ERROR: -t must be from 0.01 to 1\n" );
    exit(1);
    }

  double n = log( 1.0 - t ) / log( 1.0 - p );

  printf( "The probabilty of the desired outcome is %.2lf per attempt\n", p );
  printf( "The probabilty that the desired outcome happens at least once reaches %.2lf after %.2lf attempts\n", t, n );

  return 0;
  }

