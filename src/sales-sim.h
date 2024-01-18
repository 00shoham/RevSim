#ifndef _INCLUDE_SALES_SIM
#define _INCLUDE_SALES_SIM

void PaySingleRepSalary( _CONFIG* conf, _SALES_REP* s );
void SimulateRep( _CONFIG* conf, _SALES_REP* s );
void SimulateCalls( _CONFIG* conf, int dayNo, time_t tSim );
void CloseSingleSale( _CONFIG* conf,
                      _SALES_REP* salesRep,
                      _ORG* targetOrg,
                      _SINGLE_DAY* repFirstDay,
                      _SINGLE_DAY* repLastDay,
                      _PRODUCT* product,
                      double overrideRevenue );

#endif
