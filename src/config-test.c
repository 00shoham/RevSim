#include "base.h"

FILE* events = NULL;

int main( int argc, char** argv )
  {
  char* confDir = ".";
  char* confFile = "config.ini";
  char* outFile = NULL;

  for( int i=1; i<argc; ++i )
    {
    if( strcmp( argv[i], "-c" )==0 && i+1<argc )
      {
      confFile = argv[++i];
      }
    else if( strcmp( argv[i], "-d" )==0 && i+1<argc )
      {
      confDir = argv[++i];
      }
    else if( strcmp( argv[i], "-o" )==0 && i+1<argc )
      {
      outFile = argv[++i];
      }
    else if( strcmp( argv[i], "-h" )==0 )
      {
      printf("USAGE: %s [-c configFile] [-d configDir] [-o outFile]\n", argv[0] );
      exit(0);
      }
    else
      {
      printf("ERROR: unknown argument [%s]\n", argv[i] );
      exit(1);
      }
    }

  char* confPath = MakeFullPath( confDir, confFile );
  _CONFIG* conf = (_CONFIG*)calloc( 1, sizeof( _CONFIG ) );
  if( conf==NULL ) Error( "Cannot allocate CONFIG object" );

  SetDefaults( conf );
  ReadConfig( conf, confPath );
  ValidateConfig( conf );

  free( confPath );
  confPath = NULL;

  FILE* out = stdout;
  if( NOTEMPTY( outFile ) )
    {
    out = fopen( outFile, "w" );
    if( out==NULL )
      Error( "Cannot open %s", outFile );
    }

  PrintConfig( out, conf );

  FreeConfig( conf );

  return 0;
  }
