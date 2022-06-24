#include "base.h"

_MONTHLY_SUMMARY* FindSummaryRecord( char* subject,
                                     _MONTHLY_SUMMARY* array, int nEntries,
                                     _MMDD* monthStart )
  {
  if( EMPTY( subject ) )
    return NULL;
  if( array==NULL )
    return NULL;
  if( nEntries<=0 )
    return NULL;
  if( monthStart==NULL )
    return NULL;

  /* inefficient but we don't really care as the array is short */
  for( int i=0; i<nEntries; ++i )
    {
    _MONTHLY_SUMMARY* entry = array + i;
    if( entry->monthStart.year == monthStart->year
        && entry->monthStart.month == monthStart->month
        && entry->monthStart.day == monthStart->day )
      return entry;
    }

  Warning( "Tried to find summary record for %s @ %04d-%02d-%02d but could not",
           subject, monthStart->year, monthStart->month, monthStart->day );

  return NULL;
  }

void AddMonthlySummaries( _MONTHLY_SUMMARY* dst, int nDst, _MONTHLY_SUMMARY* src, int nSrc )
  {
  int i = 0;
  if( dst==NULL || nDst<1 || src==NULL || nSrc<1 )
    return;

  int srcY = src->monthStart.year;
  int srcM = src->monthStart.month;

  _MONTHLY_SUMMARY* d = dst;
  int gotIt = 0;
  for( i=0; i<nDst; ++i )
    {
    if( d->monthStart.year == srcY
        && d->monthStart.month == srcM )
      {
      gotIt = 1;
      break;
      }
    ++d;
    }

  if( gotIt==0 )
    {
    Warning( "Copying monthly summary data failed - no overlap in month range" );
    return;
    }

  int dMax = nDst - i;
  int sMax = nSrc;
  if( dMax < sMax )
    sMax = dMax;

  _MONTHLY_SUMMARY* s = src;
  for( i=0; i<sMax; ++i )
    {
    d->revenue += s->revenue;
    d->commission += s->commission;
    d->salary += s->salary;
    ++s;
    ++d;
    }
  }

void PrintRevenueSummary( FILE* out, _MONTHLY_SUMMARY* ms, int nMonths, char* title )
  {
  if( ms==NULL || nMonths<1 || out==NULL || EMPTY( title ) )
    return;

  double revTotal = 0;
  double comTotal = 0;
  double salTotal = 0;
  double net = 0;
  double netTotal = 0;

  for( int i=0; i<70; ++i )
    fputc( '=', out );
  fputc( '\n', out );

  fprintf( out, "%s\n", title );
  for( int i=0; i<70; ++i )
    fputc( '-', out );
  fputc( '\n', out );

  fprintf( out, "CCYY-MM %11s %11s %11s %11s %11s\n",
           "Revenue", "Commission", "Salary", "This month", "Total" );
  fprintf( out, "        %11s %11s %11s %11s %11s\n",
           "-----------", "-----------", "-----------", "-----------", "-----------" );

  for( int k=0; k < nMonths; ++k )
    {
    net = ms->revenue - ms->salary - ms->commission;
    netTotal += net;

    revTotal += ms->revenue;
    salTotal += ms->salary;
    comTotal += ms->commission;
    fprintf( out, "%04d-%02d % 11.2f % 11.2f % 11.2f % 11.2f % 11.2f\n",
                  ms->monthStart.year, ms->monthStart.month,
                  ms->revenue, ms->commission, ms->salary,
                  net, netTotal );
    ++ms;
    }

  fprintf( out, "        %11s %11s %11s %11s %11s\n",
           "-----------", "-----------", "-----------", "-----------", "-----------" );

  fprintf( out, " Total: % 11.2f % 11.2f % 11.2f %11s % 11.2f\n",
                revTotal, comTotal, salTotal, " ",
                netTotal );

  fprintf( out, "\n" );
  }

void PrintCounters( FILE* f, _CONFIG* conf )
  {
  if( f==NULL )
    return;
  if( conf==NULL )
    return;
  if( conf->nMonths<=0 )
    return;
  if( conf->monthlySummary==NULL )
    return;
 
  fprintf( f, "%3s %7s %6s %6s %6s %6s %6s %6s %6s\n",
           "Mon", "YYYY-MM",
           "Calls",
           "Custs",
           "A.Orgs",
           "Wins",
           "Rej's",
           "Losses",
           "XFers"
           );
  fprintf( f, "%3s %7s %6s %6s %6s %6s %6s %6s %6s\n",
           "===", "=======",
           "=====",
           "======",
           "======",
           "====",
           "=====",
           "======",
           "====="
           );

  int totalCalls = 0;
  int totalWins = 0;
  int totalRejections = 0;
  int totalLosses = 0;
  int totalTransfers = 0;

  _MONTHLY_SUMMARY* month = conf->monthlySummary;
  for( int i=0; i<conf->nMonths; ++i )
    {
    fprintf( f, "%03d %04d-%02d %6d %6d %6d %6d %6d %6d %6d\n",
             i, month->monthStart.year, month->monthStart.month,
             month->nCalls,
             month->nCustomers,
             month->nAvailableOrgs,
             month->nWins,
             month->nRejections,
             month->nLosses,
             month->nTransfers );

    totalCalls += month->nCalls;
    totalWins += month->nWins;
    totalRejections += month->nRejections;
    totalLosses += month->nLosses;
    totalTransfers += month->nTransfers;

    ++month;
    }

  fprintf( f, "%3s %7s %6s %6s %6s %6s %6s %6s %6s\n",
           "===", "=======",
           "=====",
           "======",
           "======",
           "====",
           "=====",
           "======",
           "====="
           );

  fprintf( f, "%3s %4s-%2s %6d %6s %6s %6d %6d %6d %6d\n",
           "   ", "    ", "  ",
           totalCalls,
           "      ",
           "      ",
           totalWins,
           totalRejections,
           totalLosses,
           totalTransfers );
  }
