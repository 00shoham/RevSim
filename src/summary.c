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

void AddToMonthlySummary( _MONTHLY_SUMMARY* array, int nMonths,
                          int year, int month,
                          double expense, double revenue )
  {
  if( array==NULL || nMonths==0 )
    return;

  _MONTHLY_SUMMARY* monthPtr = array;
  int i = 0;
  for( i=0; i<nMonths; ++i )
    {
    if( monthPtr->monthStart.year==year
        && monthPtr->monthStart.month==month )
      break;
    ++monthPtr;
    }

  if( i==nMonths )
    return;

  if( expense>0 )
    monthPtr->expense += expense;

  if( revenue>0 )
    monthPtr->revenue += revenue;
  }

double NetIncomeForYear( int year, _MONTHLY_SUMMARY* array, int nMonths )
  {
  if( array==NULL || nMonths==0 )
    return 0;

  double netIncome = 0;
  _MONTHLY_SUMMARY* monthPtr = array;
  for( int i=0; i<nMonths; ++i )
    {
    if( monthPtr->monthStart.year == year )
      {
      netIncome += monthPtr->revenue;
      netIncome -= monthPtr->commission;
      netIncome -= monthPtr->salary;
      netIncome -= monthPtr->expense;
#if 0
  /* QQQ this will work better once we move the process of
     totaling up reps into the overall picture every month,
     rather than at the end of the sim. */
      Notice( "Net income becomes %.2lf after %04d-%02d",
              netIncome, monthPtr->monthStart.year, monthPtr->monthStart.month );
#endif
      }
    else if( monthPtr->monthStart.year > year )
      break;

    ++monthPtr;
    }

  return netIncome;
  }

double GetTaxLossCarryForward( _CONFIG* conf, int Y )
  {
  _MONTHLY_SUMMARY* january = FindMonthInArray( conf->monthlySummary, conf->nMonths, Y, 1 );
  if( january==NULL )
    return 0;
  else
    {
    if( january->taxLossCarryForward > 0 )
      Notice( "Calendar %04d has a tax loss carry forward of %.2lf", Y, january->taxLossCarryForward );
    return january->taxLossCarryForward;
    }
  }

void SetTaxLossCarryForward( _CONFIG* conf, int Y, double amount )
  {
  _MONTHLY_SUMMARY* january = FindMonthInArray( conf->monthlySummary, conf->nMonths, Y, 1 );
  if( january==NULL )
    Warning( "Cannot find %04d-01 to set tax loss carry forward", Y );
  else
    {
    /* Notice( "Setting the tax loss carry forward for calendar %04d to %.2lf", Y, amount ); */
    january->taxLossCarryForward = amount;
    }
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

_MONTHLY_SUMMARY* FindMonthInArray( _MONTHLY_SUMMARY* array, int len, int Y, int M )
  {
  if( array==NULL || len==0 )
    return NULL;
  for( int i=0; i<len; ++i )
    {
    if( array->monthStart.year==Y
        && array->monthStart.month==M )
      return array;
    ++array;
    }
  return NULL;
  }

void AddSummaryRecords( _MONTHLY_SUMMARY* dest, _MONTHLY_SUMMARY* src )
  {
  if( dest==NULL || src==NULL )
    return;

  dest->revenue += src->revenue;
  dest->commission += src->commission;
  dest->salary += src->salary;
  dest->expense += src->expense;

  dest->nCalls += src->nCalls;
  dest->nAvailableOrgs += src->nAvailableOrgs;
  dest->nCustomers += src->nCustomers;
  dest->nWins += src->nWins;
  dest->nRejections += src->nRejections;
  dest->nLosses += src->nLosses;
  dest->nTransfers += src->nTransfers;
  }

void AddMonthlySummariesSingleMonth( _CONFIG* conf, int Y, int M )
  {
  if( conf==NULL )
    {
    Warning( "AddMonthlySummariesSingleMonth - NULL configuration" );
    return;
    }

  if( Y<=0 || M<1 || M>12 )
    {
    Warning( "AddMonthlySummariesSingleMonth - bad date" );
    return;
    }

  if( conf->monthlySummary==NULL )
    {
    Warning( "AddMonthlySummariesSingleMonth - NULL summary data" );
    return;
    }

  _MONTHLY_SUMMARY* destMonth = FindMonthInArray( conf->monthlySummary, conf->nMonths, Y, M );
  if( destMonth==NULL )
    {
    Warning( "AddMonthlySummariesSingleMonth - cannot find output %04d-%02d", Y, M );
    return;
    }

  if( conf->customerCare==NULL
      || conf->customerCare->monthlySummary==NULL )
    {
    Warning( "No customer care?" );
    return;
    }

  _MONTHLY_SUMMARY* cCareMonth = FindMonthInArray( conf->customerCare->monthlySummary, conf->nMonths, Y, M );
  if( cCareMonth==NULL )
    {
    Warning( "AddMonthlySummariesSingleMonth - cannot find customer care for %04d-%02d", Y, M );
    return;
    }

  AddSummaryRecords( destMonth, cCareMonth );

  for( _SALES_REP* rep = conf->salesReps; rep!=NULL; rep=rep->next )
    {
    _MONTHLY_SUMMARY* repMonth = NULL;
    if( rep->monthlySummary==NULL )
      Warning( "Rep %s has no monthly summary", NULLPROTECT( rep->id ) );
    else
      repMonth = FindMonthInArray( rep->monthlySummary, rep->nMonths, Y, M );
    if( repMonth!=NULL )
      AddSummaryRecords( destMonth, repMonth );
    }
  }

void PrintRevenueSummary( FILE* out, _MONTHLY_SUMMARY* ms, int nMonths, char* title )
  {
  if( ms==NULL || nMonths<1 || out==NULL || EMPTY( title ) )
    return;

  double revTotal = 0;
  double comTotal = 0;
  double salTotal = 0;
  double expTotal = 0;
  double net = 0;
  double netTotal = 0;

  for( int i=0; i<70; ++i )
    fputc( '=', out );
  fputc( '\n', out );

  fprintf( out, "%s\n", title );
  for( int i=0; i<70; ++i )
    fputc( '-', out );
  fputc( '\n', out );

  fprintf( out, "CCYY-MM %11s %11s %11s %11s %11s %11s\n",
           "Revenue", "Commission", "Salary", "Expense", "This month", "Total" );
  fprintf( out, "        %11s %11s %11s %11s %11s %11s\n",
           "-----------", "-----------", "-----------", "-----------", "-----------", "-----------" );

  for( int k=0; k < nMonths; ++k )
    {
    net = ms->revenue - ms->salary - ms->commission - ms->expense;
    netTotal += net;

    revTotal += ms->revenue;
    salTotal += ms->salary;
    comTotal += ms->commission;
    expTotal += ms->expense;

    fprintf( out, "%04d-%02d % 11.0f % 11.0f % 11.0f % 11.0f % 11.0f % 11.0f\n",
                  ms->monthStart.year, ms->monthStart.month,
                  ms->revenue, ms->commission, ms->salary, ms->expense,
                  net, netTotal );
    ++ms;
    }

  fprintf( out, "        %11s %11s %11s %11s %11s %11s\n",
           "-----------", "-----------", "-----------", "-----------", "-----------", "-----------" );

  fprintf( out, " Total: % 11.0f % 11.0f % 11.0f % 11.0f %11s % 11.0f\n",
                revTotal, comTotal, salTotal, expTotal, " ",
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

void SetMonthlyUnits( _CONFIG* conf, _MONTHLY_SUMMARY* month, _PRODUCT* p, _ORG* o, int n )
  {
  if( month==NULL )
    Error( "AddMonthlyUnits() - NULL month" );

  if( p==NULL )
    Error( "AddMonthlyUnits() - NULL product" );

  /*
  if( o==NULL )
    Error( "AddMonthlyUnits() - NULL org" );
  */

  _MONTHLY_UNITS* mu = (_MONTHLY_UNITS*)SafeCalloc( 1, sizeof(_MONTHLY_UNITS), "Monthly units struct" );
  mu->product = p;
  mu->customer = o;
  mu->nUnits = n;
  mu->next = month->units;
  month->units = mu;
  }

void FreeMonthlyUnitsList( _MONTHLY_UNITS* list )
  {
  if( list==NULL )
    return;
  if( list->next!=NULL )
    {
    FreeMonthlyUnitsList( list->next );
    list->next = NULL;
    }
  free( list );
  }

void UnitsReport( FILE* f, _CONFIG* conf )
  {
  if( f==NULL )
    Error( "UnitsReport() - No output specified" );
  if( conf==NULL )
    Error( "UnitsReport() - No configuration object" );
  if( conf->monthlySummary==NULL || conf->nMonths<=0 )
    Error( "UnitsReport() - No monthly data available" );

  int nProducts = 0;
  for( _PRODUCT* p = conf->products; p!=NULL; p=p->next )
    if( p->priceByUnits )
      ++nProducts;

  if( nProducts==0 )
    {
    Warning( "UnitsReport() - No products priced per unit" );
    return;
    }

  _MONTHLY_SUMMARY* ms = conf->monthlySummary;
  for( int monthNo=0; monthNo<conf->nMonths; ++monthNo, ++ms )
    {
    if( ms->units==NULL )
      continue;

    fprintf( f, "Month %04d-%02d\n", ms->monthStart.year, ms->monthStart.month );
    for( _PRODUCT* p = conf->products; p!=NULL; p=p->next )
      {
      if( ! p->priceByUnits )
        continue;

      int nOrgs = 0;
      int nUnits = 0;
      for( _MONTHLY_UNITS* u = ms->units; u!=NULL; u=u->next )
        {
        if( u->product==p )
          {
          ++nOrgs;
          nUnits += u->nUnits;
          }
        }

      int avg = -1;
      if( nOrgs>0 )
        avg = (int)round((double)nUnits / (double)nOrgs);
      fprintf( f, "  %s: %d units at %d customers (average %d/customer)\n", p->name, nUnits, nOrgs, avg );
      }
    }
  }

