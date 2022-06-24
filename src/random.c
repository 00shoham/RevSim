#include "base.h"

#include <float.h>

const double DBL_EPS_COMP = 1 - DBL_EPSILON; // DBL_EPSILON is defined in <limits.h>.

inline double RandU()
  {
  return DBL_EPSILON + ((double) rand()/RAND_MAX);
  }

inline double RandN2( double mu, double sigma )
  {
  return mu + (rand()%2 ? -1.0 : 1.0) * sigma * pow( -log( DBL_EPS_COMP*RandU() ), 0.5);
  }

inline double RandN()
  {
  return RandN2(0, 1.0);
  }

inline int PercentProbabilityEvent( double percent )
  {
  int random = lrand48() % 100000;
  if( random <= (int)(percent * 1000.0 + 0.5) )
    return 1;
  else
    return 0;
  }

void RandomSeed()
  {
  FILE* f = fopen( "/dev/urandom", "rb" );
  if( f==NULL )
    {
    srand( time( NULL ) );
    srand48( time( NULL ) );
    }
  else
    {
    long seed = (long)time(NULL);
    for( int i=0; i<16; ++i )
      {
      int c = fgetc( f );
      if( c==EOF )
        break;
      seed <<= 8;
      seed |= c;
      }
    srand( seed );
    srand48( seed );
    fclose( f );
    }
  }
