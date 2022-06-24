#include "base.h"

#define ITERATIONS 10

FILE* events = NULL;

void Test( double avg, double sdev )
  {
  double samples[ITERATIONS];
  printf( "Mean = %.1lf, SDev=%.1lf\n", avg, sdev );
  for( int i=0; i<ITERATIONS; ++i )
    {
    double n = RandN2( avg, sdev );
    samples[i] = n;
    printf( "   %.1lf\n", n );
    }

  double mean = 0.0;
  for( int i=0; i<ITERATIONS; ++i )
    {
    mean += samples[i];
    }
  mean /= (double)ITERATIONS;
  printf( "Actual mean: %.1lf\n", mean );

  double sum = 0;
  for( int i=0; i<ITERATIONS; ++i )
    {
    sum += pow( samples[i] - mean, 2.0 );
    }

  sum /= (double)ITERATIONS;
  sum = sqrt( sum );
  printf( "Actual sdev: %.1lf\n\n", sum );
  }


int main()
  {
  printf("Startup - seed RNG\n");
  RandomSeed();

  printf("Tests..\n");
  Test( 10.0, 2.0 );
  Test( 100.0, 50.0 );

  printf("ProbabilityEvent loop..\n");
  int n = 0;
  for( int i=0; i<100000; ++i )
    {
    if( PercentProbabilityEvent( 1.25 ) )
      ++n;
    }
  double p = (double)n / 100000.0 * 100.0;
  printf( "Ran %d tests, got %d hits (%.2lf percent) - expected 1.25%%\n",
          100000, n, p );

  return 0;
  }
