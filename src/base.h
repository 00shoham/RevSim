#ifndef _INCLUDE_BASE
#define _INCLUDE_BASE

#include "utils.h"

#define SafeCalloc( N, S, W ) calloc( N, S );

struct _config;
typedef struct _config _CONFIG;
typedef struct _salesRep _SALES_REP;

#include "random.h"
#include "summary.h"
#include "revenue.h"

#define MONTHS 12
#define MAX_SIMULATION_DAYS 10000
#define DEFAULT_DAYS_TO_AUTO_REPLACE_SALES_REP 14
#define DEFAULT_DAYS_TO_AUTO_REPLACE_SALES_REP_SDEV 3
#define DEFAULT_COMMISSION_MONTHS 99

#include "cash-event.h"
#include "org.h"
#include "holiday.h"
#include "vacation.h"
#include "sales-stage.h"
#include "product.h"
#include "sales-rep.h"
#include "config.h"
#include "availability.h"
#include "sales-sim.h"
#include "events.h"

#endif
