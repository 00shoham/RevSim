#include "base.h"

void SaleToOrg( _CONFIG* conf, _ORG* o, time_t saleDate )
  {
  if( conf==NULL )
    return;

  if( o==NULL )
    return;

  if( saleDate==0 )
    return;

  o->isCustomer = 1;
  o->saleDate = saleDate;

  // one less available to sell to
  --conf->nAvailableOrgs;
  }

void RejectedByOrg( _CONFIG* conf, _ORG* o, time_t callDate )
  {
  if( conf==NULL )
    return;

  if( callDate==0 )
    return;
  
  if( o==NULL )
    return;

  o->isCustomer = 0;
  o->saleDate = 0;
  o->earliestLegalCall = callDate + conf->orgCoolingPeriodDays * DAY_IN_SECONDS;

  // number of available orgs unchanged, but available date pushed out
  }

void OrgAttrition( _CONFIG* conf, _ORG* o, time_t callDate )
  {
  if( conf==NULL )
    return;

  if( callDate==0 )
    return;
  
  if( o==NULL )
    return;

  o->isCustomer = 0;
  o->saleDate = 0;
  o->earliestLegalCall = callDate + conf->orgCoolingPeriodDays * DAY_IN_SECONDS;

  // number of available orgs incremented, but available date pushed out
  ++conf->nAvailableOrgs;
  }

_ORG* FindAvailableTargetOrg( _CONFIG* conf, time_t callTime )
  {
  if( conf==NULL )
    return NULL;

  if( conf->nAvailableOrgs<=0 )
    return NULL;

  if( conf->orgs==NULL || conf->marketSize==0 )
    return NULL;

  _ORG* o = conf->orgs;
  for( int i=0; i<conf->marketSize; ++i )
    {
    if( o->isCustomer==0
        && callTime >= o->earliestLegalCall )
      return o;

    ++o;
    }

  return NULL;
  }

int CountAvailableOrgs( _CONFIG* conf, time_t callTime )
  {
  if( conf==NULL )
    return -1;

  if( conf->nAvailableOrgs<=0 )
    return 0;

  if( conf->orgs==NULL || conf->marketSize==0 )
    return -2;

  int n = 0;
  _ORG* o = conf->orgs;
  for( int i=0; i<conf->marketSize; ++i )
    {
    if( o->isCustomer==0 && callTime >= o->earliestLegalCall )
      ++n;
    ++o;
    }

  return n;
  }
