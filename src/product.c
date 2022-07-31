#include "base.h"

_PRODUCT* NewProduct( char* id, _PRODUCT* list )
  {
  if( EMPTY( id ) )
    Error( "NewProduct(NULL)" );
  _PRODUCT* p = (_PRODUCT*)SafeCalloc( 1, sizeof( _PRODUCT ), "_PRODUCT" );
  if( NOTEMPTY( id ) )
    p->id = strdup( id );
  p->next = list;
  return p;
  }

_PRODUCT* FindProduct( _PRODUCT* list, char* id )
  {
  if( EMPTY( id ) )
    return NULL;
  for( _PRODUCT* p = list; p!=NULL; p=p->next )
    if( NOTEMPTY( p->id ) && strcasecmp( p->id, id )==0 )
      return p;
  return NULL;
  }

void FreeProduct( _PRODUCT* p )
  {
  if( p==NULL )
    return;
  if( p->next!=NULL )
    {
    FreeProduct( p->next );
    p->next = NULL;
    }
  FreeIfAllocated( &(p->id) );
  FreeIfAllocated( &(p->name) );
  if( p->stageArray != NULL )
    free( p->stageArray );
  free( p );
  }

int ValidateSingleProduct( _PRODUCT* p )
  {
  if( p==NULL || EMPTY( p->id ) )
    return -100;

  if( EMPTY( p->name ) )
    {
    Warning( "Product %s has no name", p->id );
    return -1;
    }

  if( p->averageMonthlyDealSize<=0 )
    {
    Warning( "Product %s has no monthly average deal size", p->id );
    return -2;
    }

  if( p->dealSizeStandardDeviation<=0 )
    {
    Warning( "Product %s has no standard deviation for deal size", p->id );
    return -3;
    }

  if( p->monthlyGrowthRatePercent<=0 )
    {
    Warning( "Product %s has no monthly per-deal revenue growth rate", p->id );
    return -4;
    }

  if( p->averageMonthsToReachSteadyState<=0 )
    {
    Warning( "Product %s has no # of months to reach per-deal revenue steady state", p->id );
    return -5;
    }

  if( p->sdevMonthsToReachSteadyState<=0 )
    {
    Warning( "Product %s has no # of months to reach per-deal revenue steady state", p->id );
    return -6;
    }

  if( p->probabilityOfCustomerAttritionPerMonth<=0 )
    {
    Warning( "Product %s has no monthly attrition probability per customer/deal", p->id );
    return -7;
    }

  if( p->nSalesStages<1 || p->stageArray==NULL )
    {
    Warning( "Product %s has no first sales stage", p->id );
    return -8;
    }

  return 0;
  }

void PrintProduct( FILE* f, _PRODUCT* p )
  {
  if( f==NULL || p==NULL || EMPTY( p->id ) )
    return;

  fprintf( f, "PRODUCT=%s\n", p->id );
  if( NOTEMPTY( p->name ) )
    fprintf( f, "PRODUCT_NAME=%s\n", p->name );
  if( p->stageArray!=NULL )
    fprintf( f, "PRODUCT_FIRST_SALE_STAGE=%s\n", p->stageArray[0]->id );
  if( p->averageMonthlyDealSize >0 )
    fprintf( f, "PRODUCT_M_REVENUE_AVG=%.1lf\n", p->averageMonthlyDealSize );
  if( p->dealSizeStandardDeviation >0 )
    fprintf( f, "PRODUCT_M_REVENUE_SDEV=%.1lf\n", p->dealSizeStandardDeviation );
  if( p->monthlyGrowthRatePercent >0 )
    fprintf( f, "PRODUCT_M_GROWTH_RATE_PERCENT=%.1lf\n", p->monthlyGrowthRatePercent );
  if( p->averageMonthsToReachSteadyState >0 )
    fprintf( f, "PRODUCT_MONTHS_TIL_STEADY_STATE_AVG=%.1lf\n", p->averageMonthsToReachSteadyState );
  if( p->sdevMonthsToReachSteadyState >0 )
    fprintf( f, "PRODUCT_MONTHS_TIL_STEADY_STATE_SDEV=%.1lf\n", p->sdevMonthsToReachSteadyState );
  if( p->probabilityOfCustomerAttritionPerMonth >0 )
    fprintf( f, "PRODUCT_ATTRITION_PERCENT_PER_MONTH=%.1lf\n", p->probabilityOfCustomerAttritionPerMonth );

  fprintf( f, "\n" );
  }

