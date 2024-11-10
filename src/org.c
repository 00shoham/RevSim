#include "base.h"

void SaleToOrg( _CONFIG* conf, _PRODUCT* p, _ORG* o, time_t saleDate )
  {
  if( conf==NULL )
    return;

  if( p==NULL )
    return;

  if( o==NULL )
    return;

  if( saleDate==0 )
    return;

  o->isCustomer = 1;
  o->saleDate = saleDate;

  // one less available to sell to
  -- (p->nAvailableOrgs);
  }

void RejectedByOrg( _CONFIG* conf, _PRODUCT* p, _ORG* o, time_t callDate )
  {
  if( conf==NULL )
    return;

  if( callDate==0 )
    return;
  
  if( o==NULL )
    return;

  o->isCustomer = 0;
  o->saleDate = 0;
  o->earliestLegalCall = callDate + p->orgCoolingPeriodDays * DAY_IN_SECONDS;

  // number of available orgs unchanged, but available date pushed out
  }

void OrgAttrition( _CONFIG* conf, _PRODUCT* p, _ORG* o, time_t callDate )
  {
  if( conf==NULL )
    return;

  if( p==NULL )
    return;

  if( callDate==0 )
    return;
  
  if( o==NULL )
    return;

  o->isCustomer = 0;
  o->saleDate = 0;
  o->earliestLegalCall = callDate + p->orgCoolingPeriodDays * DAY_IN_SECONDS;

  // number of available orgs incremented, but available date pushed out
  ++ (p->nAvailableOrgs);
  }

_ORG* FindAvailableTargetOrg( _CONFIG* conf, _PRODUCT* p, time_t callTime )
  {
  if( conf==NULL )
    return NULL;

  if( p==NULL )
    return NULL;

  if( p->nAvailableOrgs<=0 )
    return NULL;

  if( p->orgs==NULL || p->marketSize==0 )
    return NULL;

  _ORG* o = p->orgs;
  for( int i=0; i<p->marketSize; ++i )
    {
    if( o->isCustomer==0
        && o->lostForever==0
        && callTime >= o->earliestLegalCall )
      return o;

    ++o;
    }

  return NULL;
  }

int CountAvailableOrgs( _CONFIG* conf, _PRODUCT* p, time_t callTime, int maxOrgNum )
  {
  if( conf==NULL )
    return -1;

  if( p==NULL )
    return -2;

  if( p->orgs==NULL || p->marketSize==0 )
    return -3;

  int n = 0;
  _ORG* o = p->orgs;

  int max = p->marketSize;
  if( maxOrgNum )
    max = maxOrgNum;
  for( int i=0; i<max; ++i )
    {
    if( o->isCustomer==0 && o->lostForever==0 && callTime >= o->earliestLegalCall )
      ++n;
    ++o;
    }

  return p->marketSize - maxOrgNum + n;
  }
