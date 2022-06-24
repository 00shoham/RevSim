#ifndef _INCLUDE_SALES_SIM
#define _INCLUDE_SALES_SIM

void PaySingleRepSalary( _CONFIG* conf, _SALES_REP* s );
void SimulateRep( _CONFIG* conf, _SALES_REP* s );
void SimulateCalls( _CONFIG* conf, int dayNo, time_t tSim );

#endif
