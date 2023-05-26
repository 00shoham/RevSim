#ifndef _INCLUDE_ORG
#define _INCLUDE_ORG

#define MAX_MARKET_SIZE 250000

typedef struct _org
  {
  int number;
  int isCustomer;
  time_t saleDate;
  time_t earliestLegalCall;
  } _ORG;

void SaleToOrg( _CONFIG* conf, _ORG* o, time_t saleDate );
void RejectedByOrg( _CONFIG* conf, _ORG* o, time_t callDate );
void OrgAttrition( _CONFIG* conf, _ORG* o, time_t callDate );
_ORG* FindAvailableTargetOrg( _CONFIG* conf, time_t callTime );
int CountAvailableOrgs( _CONFIG* conf, time_t callTime );

#endif
