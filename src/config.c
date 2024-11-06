#include "base.h"

void UpdateGlobalParsingLocation( _CONFIG* config )
  {
  FreeIfAllocated( &parsingLocation );
  if( config!=NULL
      && config->parserLocation!=NULL
      && NOTEMPTY( config->parserLocation->tag ) )
    {
    char whereAmI[BUFLEN];
    snprintf( whereAmI, sizeof(whereAmI)-1, "%s::%d ",
              config->parserLocation->tag,
              config->parserLocation->iValue );
    parsingLocation = strdup( whereAmI );
    }
  }

void SetDefaults( _CONFIG* config )
  {
  memset( config, 0, sizeof(_CONFIG) );
  config->daysToAutoReplaceRepAvg = DEFAULT_DAYS_TO_AUTO_REPLACE_SALES_REP;
  config->daysToAutoReplaceRepSDev = DEFAULT_DAYS_TO_AUTO_REPLACE_SALES_REP_SDEV;
  }

void FreeConfig( _CONFIG* config )
  {
  if( config==NULL )
    return;

  if( config->customerCare!=NULL )
    {
    FreeSalesRep( config->customerCare );
    config->customerCare = NULL;
    }

  if( config->configFolder != NULL )
    FREE( config->configFolder );

  if( config->parserLocation != NULL )
    {
    FreeTagValue( config->parserLocation );
    config->parserLocation = NULL;
    }

  if( config->includes != NULL )
    {
    FreeTagValue( config->includes );
    config->includes = NULL;
    }

  if( config->list != NULL )
    {
    FreeTagValue( config->list );
    config->list = NULL;
    }

  if( config->monthlySummary!=NULL )
    {
    for( int i=0; i<config->nMonths; ++i )
      {
      _MONTHLY_SUMMARY* ms = config->monthlySummary + i;
      if( ms->units != NULL )
        FreeMonthlyUnitsList( ms->units );
      }
    FREE( config->monthlySummary );
    }

  FreeHoliday( config->holidays );
  config->holidays = NULL;

  FreeVacation( config->vacations );
  config->vacations = NULL;

  if( config->baselineWorkDays != NULL )
    {
    _SINGLE_DAY* lastDay = config->baselineWorkDays + config->nBaselineWorkDays;
    for( _SINGLE_DAY* day = config->baselineWorkDays; day < lastDay; ++day )
      {
      if( day->dailySales != NULL )
        {
        FreeRevenueEvent( day->dailySales );
        day->dailySales = NULL;
        }
      if( day->fees != NULL )
        {
        FreePayEvent( day->fees );
        day->fees = NULL;
        }
      if( day->cashEvents != NULL )
        {
        FreeCashEventList( day->cashEvents );
        day->cashEvents = NULL;
        }
      }
    FREE( config->baselineWorkDays );
    }

  FreeSalesStage( config->stages );
  config->stages = NULL;

  FreeProduct( config->products );
  config->products = NULL;

  FreeSalesRepClass( config->salesRepClasses );
  config->salesRepClasses = NULL;

  FreeSalesRep( config->salesReps );
  config->salesReps = NULL;
 
  FreeSalesRep( config->customerCare );
  config->customerCare = NULL;

  if( config->monthlySummary != NULL )
    FREE( config->monthlySummary );
 
  if( config->orgs != NULL )
    FREE( config->orgs );

  if( config->cashEvents != NULL )
    {
    FreeCashEventList( config->cashEvents );
    config->cashEvents = NULL;
    }

  if( config->cashEventArray != NULL )
    FREE( config->cashEventArray );

  free( config );
  }

void SetStringPointer( char** ptrPtr, char* value )
  {
  if( ptrPtr==NULL )
    Error( "SetStringPointer passed a NULL arg 1" );
  if( value==NULL )
    Error( "SetStringPointer passed a NULL arg 2" );
  FreeIfAllocated( ptrPtr );
  *ptrPtr = strdup( value );
  }

int ValidProductivityArray( char* str )
  {
  if( EMPTY( str ) )
    return -1;

  for( char* ptr = str; *ptr!=0; ++ptr )
    if( ! ( *ptr==',' || isdigit( *ptr ) || *ptr=='.' ) )
      return -2;

  int nCommas = 0;
  for( char* ptr = str; *ptr!=0; ++ptr )
    if( *ptr==',' )
      ++nCommas;
  if( nCommas>=MONTHS )
    return -3;

  return 0;
  }

void SetProductivityArray( _SALES_REP_CLASS* rt, char* value )
  {
  int i=0;
  for( i=0; i<MONTHS; ++i )
    rt->productivity[i] = 100.0;

  i=0;
  char* ptr = NULL;
  for( char* substr=strtok_r( value, ",", &ptr );
       substr!=NULL && i<MONTHS;
       substr = strtok_r( NULL, ",", &ptr ), ++i )
    {
    double d = atof( substr );
    if( d>=0 && d<=100 )
      rt->productivity[i] = d;
    if( d==100 )
      break;
    }
  }

int ProcessKeywordPair( _CONFIG* config, char* variable, char* value )
  {
  if( strcasecmp( variable, "FIRST_DAY" )==0 )
    {
    _MMDD date;
    if( IsValidMMDD( value, &date )!=0 )
      Error( "CONFIG: %s should be followed by a MM-DD day (both integers) - not [%s]", variable, value );
    config->simulationFirstDay = date;
    config->simulationStart = MMDDToTime( &date );
    return 0;
    }

  if( strcasecmp( variable, "DURATION" )==0 )
    {
    int nDays = atoi( value );
    if( nDays<1 || nDays>MAX_SIMULATION_DAYS )
      Error( "CONFIG: %s must be at least 1 and at most %d", variable, MAX_SIMULATION_DAYS );
    if( EmptyMMDD( &(config->simulationFirstDay) )==0 )
      Error( "CONFIG: %s must follow FIRST_DAY", variable );
    config->simulationDurationDays = nDays;
    config->simulationEnd = config->simulationStart + nDays * DAY_IN_SECONDS;
    if( TimeToMMDD( config->simulationEnd, &(config->simulationEndDay) )!=0 )
      Error( "CONFIG: failed to calculate end mm/dd for simulation" );
    return 0;
    }

  if( strcasecmp( variable, "HOLIDAY" )==0 )
    {
    config->holidays = NewHoliday( value, config->holidays );
    return 0;
    }

  if( strcasecmp( variable, "HOLIDAY_NAME" )==0 )
    {
    if( config->holidays==0 )
      Error( "CONFIG: %s must follow HOLIDAY", variable );
    SetStringPointer( &(config->holidays->name), value );
    return 0;
    }

  if( strcasecmp( variable, "HOLIDAY_START" )==0 )
    {
    if( config->holidays==NULL )
      Error( "CONFIG: %s must follow HOLIDAY", variable );
    _MMDD date;
    int err = 0;
    if( (err=IsValidMMDD( value, &date ))!=0 )
      Error( "CONFIG: %s should be followed by a MM-DD day (both integers) - not [%s] - %d", variable, value, err );
    config->holidays->firstDay = date;
    return 0;
    }

  if( strcasecmp( variable, "HOLIDAY_FINISH" )==0 )
    {
    if( config->holidays==NULL )
      Error( "CONFIG: %s must follow HOLIDAY", variable );
    _MMDD date;
    if( IsValidMMDD( value, &date )!=0 )
      Error( "CONFIG: %s should be followed by a MM-DD day (both integers) - not [%s]", variable, value );
    config->holidays->lastDay = date;
    return 0;
    }

  if( strcasecmp( variable, "VACATION" )==0 )
    {
    config->vacations = NewVacation( value, config->vacations );
    return 0;
    }

  if( strcasecmp( variable, "VACATION_NAME" )==0 )
    {
    if( config->vacations==NULL )
      Error( "CONFIG: %s must follow VACATION", variable );
    SetStringPointer( &(config->vacations->name), value );
    return 0;
    }

  if( strcasecmp( variable, "VACATION_DAYS" )==0 )
    {
    if( config->vacations==NULL )
      Error( "CONFIG: %s must follow VACATION", variable );
    int n = atoi( value );
    if( n<1 || n>100 )
      Error( "CONFIG: %s has unreasonable value", variable );
    config->vacations->daysPerYear = n;
    return 0;
    }

  if( strcasecmp( variable, "VACATION" )==0 )
    {
    config->vacations = NewVacation( value, config->vacations );
    return 0;
    }

  if( strcasecmp( variable, "STAGE" )==0 )
    {
    config->stages = NewSalesStage( value, config->stages );
    config->stages->isInitial = 1;
    config->stages->isTerminal = 1;
    return 0;
    }

  if( strcasecmp( variable, "STAGE_NAME" )==0 )
    {
    if( config->stages==NULL )
      Error( "CONFIG: %s must follow STAGE", variable );
    SetStringPointer( &(config->stages->name), value );
    return 0;
    }

  if( strcasecmp( variable, "STAGE_FOLLOWS" )==0 )
    {
    if( config->stages==NULL )
      Error( "CONFIG: %s must follow STAGE", variable );
    _SALES_STAGE* predecessor = FindSalesStage( config->stages, value );
    if( predecessor==NULL )
      Error( "Cannot find earlier sales stage %s", value );
    config->stages->predecessor = predecessor;
    config->stages->isInitial = 0;
    config->stages->predecessor->isTerminal = 0;
    return 0;
    }

  if( strcasecmp( variable, "STAGE_DAYS_AVG" )==0 )
    {
    if( config->stages==NULL )
      Error( "CONFIG: %s must follow STAGE", variable );
    double d = atof( value );
    if( d<1 || d>365 )
      Error( "CONFIG: %s must be from 1 to 365", variable );
    config->stages->daysDelayAverage = d;
    return 0;
    }

  if( strcasecmp( variable, "STAGE_DAYS_SDEV" )==0 )
    {
    if( config->stages==NULL )
      Error( "CONFIG: %s must follow STAGE", variable );
    double d = atof( value );
    if( d<1 || d>100 )
      Error( "CONFIG: %s must be from 1 to 100", variable );
    config->stages->sdevDaysDelay = d;
    return 0;
    }

  if( strcasecmp( variable, "STAGE_ATTRITION_PERCENT" )==0 )
    {
    if( config->stages==NULL )
      Error( "CONFIG: %s must follow STAGE", variable );
    double d = atof( value );
    if( d<1 || d>100 )
      Error( "CONFIG: %s must be from 1 to 100", variable );
    config->stages->percentAttrition = d;
    return 0;
    }

  if( strcasecmp( variable, "STAGE_CONNECT_ATTEMPTS_AVG" )==0 )
    {
    if( config->stages==NULL )
      Error( "CONFIG: %s must follow STAGE", variable );
    double d = atof( value );
    if( d<0 || d>10 )
      Error( "CONFIG: %s must be from 0 to 10", variable );
    config->stages->connectAttemptsAverage = d;
    return 0;
    }

  if( strcasecmp( variable, "STAGE_CONNECT_ATTEMPTS_SDEV" )==0 )
    {
    if( config->stages==NULL )
      Error( "CONFIG: %s must follow STAGE", variable );
    double d = atof( value );
    if( d<0 || d>config->stages->connectAttemptsAverage )
      Error( "CONFIG: %s must be from 0 to %.1lf", config->stages->connectAttemptsAverage );
    config->stages->sdevConnectAttempts = d;
    return 0;
    }

  if( strcasecmp( variable, "STAGE_CONNECT_RETRY_DAYS_AVG" )==0 )
    {
    if( config->stages==NULL )
      Error( "CONFIG: %s must follow STAGE", variable );
    double d = atof( value );
    if( d<0 || d>30 )
      Error( "CONFIG: %s must be from 0 to 30", variable );
    config->stages->connectRetryDaysAverage = d;
    return 0;
    }

  if( strcasecmp( variable, "STAGE_CONNECT_RETRY_DAYS_SDEV" )==0 )
    {
    if( config->stages==NULL )
      Error( "CONFIG: %s must follow STAGE", variable );
    double d = atof( value );
    if( d<0 || d>config->stages->connectRetryDaysAverage )
      Error( "CONFIG: %s must be from 0 to %.1lf", config->stages->connectRetryDaysAverage );
    config->stages->sdevConnectRetryDays = d;
    return 0;
    }

  if( strcasecmp( variable, "PRODUCT" )==0 )
    {
    config->products = NewProduct( value, config->products );
    return 0;
    }

  if( strcasecmp( variable, "PRODUCT_NAME" )==0 )
    {
    if( config->products==NULL )
      Error( "CONFIG: %s must follow PRODUCT", variable );
    SetStringPointer( &(config->products->name), value );
    return 0;
    }

  if( strcasecmp( variable, "PRODUCT_PRICE_BY_UNITS" )==0 )
    {
    if( config->products==NULL )
      Error( "CONFIG: %s must follow PRODUCT", variable );
    if( strcasecmp( value, "true" )==0 )
      config->products->priceByUnits = 1;
    else if( strcasecmp( value, "false" )==0 )
      config->products->priceByUnits = 0;
    else Error( "CONFIG: %s must be true or false", variable );
    return 0;
    }

  if( strcasecmp( variable, "PRODUCT_UNIT_ONBOARDING_FEE_AVG" )==0 )
    {
    if( config->products==NULL )
      Error( "CONFIG: %s must follow PRODUCT", variable );
    if( ! config->products->priceByUnits )
      Error( "CONFIG: %s must follow PRODUCT where PRODUCT_PRICE_BY_UNITS is set to true", variable );
    double d = atof( value );
    config->products->averageUnitOnboardingFee = d;
    return 0;
    }

  if( strcasecmp( variable, "PRODUCT_UNIT_ONBOARDING_FEE_SDEV" )==0 )
    {
    if( config->products==NULL )
      Error( "CONFIG: %s must follow PRODUCT", variable );
    if( ! config->products->priceByUnits )
      Error( "CONFIG: %s must follow PRODUCT where PRODUCT_PRICE_BY_UNITS is set to true", variable );
    double d = atof( value );
    config->products->sdevUnitOnboardingFee = d;
    return 0;
    }

  if( strcasecmp( variable, "PRODUCT_UNIT_MONTHLY_FEE_AVG" )==0 )
    {
    if( config->products==NULL )
      Error( "CONFIG: %s must follow PRODUCT", variable );
    if( ! config->products->priceByUnits )
      Error( "CONFIG: %s must follow PRODUCT where PRODUCT_PRICE_BY_UNITS is set to true", variable );
    double d = atof( value );
    config->products->averageUnitMonthlyRecurringFee = d;
    return 0;
    }

  if( strcasecmp( variable, "PRODUCT_UNIT_MONTHLY_FEE_SDEV" )==0 )
    {
    if( config->products==NULL )
      Error( "CONFIG: %s must follow PRODUCT", variable );
    if( ! config->products->priceByUnits )
      Error( "CONFIG: %s must follow PRODUCT where PRODUCT_PRICE_BY_UNITS is set to true", variable );
    double d = atof( value );
    config->products->sdevUnitMonthlyRecurringFee = d;
    return 0;
    }

  if( strcasecmp( variable, "PRODUCT_CUSTOMER_NUMBER_UNITS_AVG" )==0 )
    {
    if( config->products==NULL )
      Error( "CONFIG: %s must follow PRODUCT", variable );
    if( ! config->products->priceByUnits )
      Error( "CONFIG: %s must follow PRODUCT where PRODUCT_PRICE_BY_UNITS is set to true", variable );
    double d = atof( value );
    config->products->averageCustomerSizeUnits = d;
    return 0;
    }

  if( strcasecmp( variable, "PRODUCT_CUSTOMER_NUMBER_UNITS_SDEV" )==0 )
    {
    if( config->products==NULL )
      Error( "CONFIG: %s must follow PRODUCT", variable );
    if( ! config->products->priceByUnits )
      Error( "CONFIG: %s must follow PRODUCT where PRODUCT_PRICE_BY_UNITS is set to true", variable );
    double d = atof( value );
    config->products->sdevCustomerSizeUnits = d;
    return 0;
    }

  if( strcasecmp( variable, "PRODUCT_M_REVENUE_AVG" )==0 )
    {
    if( config->products==NULL )
      Error( "CONFIG: %s must follow PRODUCT", variable );
    if( config->products->priceByUnits )
      Error( "CONFIG: %s must follow PRODUCT where PRODUCT_PRICE_BY_UNITS is set to false", variable );
    double d = atof( value );
    if( d<0.01 )
      Error( "Invalid PRODUCT_M_REVENUE_AVG (%s)", value );
    config->products->averageMonthlyDealSize = d;
    return 0;
    }

  if( strcasecmp( variable, "PRODUCT_M_REVENUE_SDEV" )==0 )
    {
    if( config->products==NULL )
      Error( "CONFIG: %s must follow PRODUCT", variable );
    if( config->products->priceByUnits )
      Error( "CONFIG: %s must follow PRODUCT where PRODUCT_PRICE_BY_UNITS is set to false", variable );
    double d = atof( value );
    if( d<0.01 )
      Error( "Invalid PRODUCT_M_REVENUE_SDEV (%s)", value );
    config->products->sdevDealSize = d;
    return 0;
    }

  if( strcasecmp( variable, "PRODUCT_M_UNITS_GROWTH_AVG" )==0 )
    {
    if( config->products==NULL )
      Error( "CONFIG: %s must follow PRODUCT", variable );
    if( ! config->products->priceByUnits )
      Error( "CONFIG: %s must follow PRODUCT where PRODUCT_PRICE_BY_UNITS is set to true", variable );
    double d = atof( value );
    if( d<0 )
      Error( "Invalid PRODUCT_M_UNITS_GROWTH_AVG (%s) - must be at least 0", value );
    config->products->averageMonthlyGrowthRateUnits = d;
    if( config->products->monthlyGrowthRatePercent > 0 )
      Error( "Variable %s conflicts with PRODUCT_M_GROWTH_RATE_PERCENT", variable );
    return 0;
    }

  if( strcasecmp( variable, "PRODUCT_M_UNITS_GROWTH_SDEV" )==0 )
    {
    if( config->products==NULL )
      Error( "CONFIG: %s must follow PRODUCT", variable );
    if( ! config->products->priceByUnits )
      Error( "CONFIG: %s must follow PRODUCT where PRODUCT_PRICE_BY_UNITS is set to true", variable );
    double d = atof( value );
    if( d<0 )
      Error( "Invalid PRODUCT_M_UNITS_GROWTH_SDEV (%s) - must be at least 0", value );
    config->products->sdevMonthlyGrowthRateUnits = d;
    return 0;
    }

  if( strcasecmp( variable, "PRODUCT_M_GROWTH_RATE_PERCENT" )==0 )
    {
    if( config->products==NULL )
      Error( "CONFIG: %s must follow PRODUCT", variable );
    double d = atof( value );
    if( d<-100 || d>100 )
      Error( "Invalid PRODUCT_M_GROWTH_RATE_PERCENT (%s)", value );
    config->products->monthlyGrowthRatePercent = d;
    if( config->products->averageMonthlyGrowthRateUnits > 0 )
      Error( "Variable %s conflicts with PRODUCT_M_UNITS_GROWTH_AVG", variable );
    return 0;
    }

  if( strcasecmp( variable, "PRODUCT_MONTHS_TIL_STEADY_STATE_AVG" )==0 )
    {
    if( config->products==NULL )
      Error( "CONFIG: %s must follow PRODUCT", variable );
    double d = atof( value );
    if( d<0 || d>24 )
      Error( "Invalid PRODUCT_MONTHS_TIL_STEADY_STATE_AVG (%s)", value );
    config->products->averageMonthsToReachSteadyState = d;
    return 0;
    }

  if( strcasecmp( variable, "PRODUCT_MONTHS_TIL_STEADY_STATE_SDEV" )==0 )
    {
    if( config->products==NULL )
      Error( "CONFIG: %s must follow PRODUCT", variable );
    double d = atof( value );
    if( d<0 || d>config->products->averageMonthsToReachSteadyState )
      Error( "Invalid PRODUCT_MONTHS_TIL_STEADY_STATE_SDEV (%s)", value );
    config->products->sdevMonthsToReachSteadyState = d;
    return 0;
    }

  if( strcasecmp( variable, "PRODUCT_ATTRITION_PERCENT_PER_MONTH" )==0 )
    {
    if( config->products==NULL )
      Error( "CONFIG: %s must follow PRODUCT", variable );
    double d = atof( value );
    if( d<0 || d>100 )
      Error( "Invalid PRODUCT_ATTRITION_PERCENT_PER_MONTH (%s)", value );
    config->products->probabilityOfCustomerAttritionPerMonth = d;
    return 0;
    }

  if( strcasecmp( variable, "PRODUCT_FIRST_SALE_STAGE" )==0 )
    {
    if( config->products==NULL )
      Error( "CONFIG: %s must follow PRODUCT", variable );
    if( config->products->stageArray!=NULL )
      Error( "CONFIG: Product [%s] already has a first sales stage (%s)",
             config->products->id, config->products->stageArray[0]->id );
    _SALES_STAGE* s = FindSalesStage( config->stages, value );
    if( s==NULL )
      Error( "Cannot find sales stage [%s]", value );
    _SALES_STAGE** stageArray = NULL;
    int nStages = SalesStagesArray( config->stages, &(stageArray) );
    if( nStages<1 )
      Error( "Empty sales stage array?? [%s]", value );
    config->products->nSalesStages = nStages;
    config->products->stageArray = stageArray;
    return 0;
    }

  if( strcasecmp( variable, "PRODUCT_INITIAL_MONTHLY_REVENUE" )==0 )
    {
    if( config->products==NULL )
      Error( "CONFIG: %s must follow PRODUCT", variable );
    double r = atof( value );
    if( r<0 )
      Error( "Initial monthly revenue for %s is negative?  Nope.", config->products->id );
    config->products->initialMonthlyRevenue = r;
    return 0;
    }

  if( strcasecmp( variable, "PRODUCT_INITIAL_MONTHLY_CUSTOMERS" )==0 )
    {
    if( config->products==NULL )
      Error( "CONFIG: %s must follow PRODUCT", variable );
    int n = atoi( value );
    if( n<1 )
      Error( "Initial monthly number of customers for %s must be at least one...", config->products->id );
    config->products->initialMonthlyCustomers = n;
    return 0;
    }

  if( strcasecmp( variable, "PRODUCT_INITIAL_MONTHLY_UNITS" )==0 )
    {
    if( config->products==NULL )
      Error( "CONFIG: %s must follow PRODUCT", variable );
    if( ! config->products->priceByUnits )
      Error( "CONFIG: %s must follow PRODUCT where PRODUCT_PRICE_BY_UNITS is set to true", variable );
    int n = atoi( value );
    if( n<1 )
      Error( "Initial monthly number of units for %s must be at least one...", config->products->id );
    config->products->initialMonthlyUnits= n;
    return 0;
    }

  if( strcasecmp( variable, "REP_CLASS" )==0 )
    {
    config->salesRepClasses = NewSalesRepClass( value, config->salesRepClasses );
    return 0;
    }

  if( strcasecmp( variable, "REP_CLASS_NAME" )==0 )
    {
    if( config->salesRepClasses==NULL )
      Error( "CONFIG: %s must follow REP_CLASS", variable );
    SetStringPointer( &(config->salesRepClasses->name), value );
    return 0;
    }

  if( strcasecmp( variable, "REP_CLASS_PRODUCTIVITY" )==0 )
    {
    if( config->salesRepClasses==NULL )
      Error( "CONFIG: %s must follow REP_CLASS", variable );
    if( ValidProductivityArray( value )!=0 )
      Error( "CONFIG: %s needs a value of the form 0,25,50,75,100 (up to 12 values)", variable );
    SetProductivityArray( config->salesRepClasses, value );
    return 0;
    }

  if( strcasecmp( variable, "REP_CLASS_COMMISSION" )==0 )
    {
    if( config->salesRepClasses==NULL )
      Error( "CONFIG: %s must follow REP_CLASS", variable );
    double d = atof( value );
    if( d<0 || d>=80 )
      Error( "CONFIG: %s must be from 0 to 80", variable );
    config->salesRepClasses->commission = d;
    if( config->salesRepClasses->commissionMonths==0 )
      config->salesRepClasses->commissionMonths = DEFAULT_COMMISSION_MONTHS;

    return 0;
    }

  if( strcasecmp( variable, "REP_CLASS_COMMISSION_MONTHS" )==0 )
    {
    if( config->salesRepClasses==NULL )
      Error( "CONFIG: %s must follow REP_CLASS", variable );
    int i = atoi( value );
    if( i<0 || i>=120 )
      Error( "CONFIG: %s must be from 1 to 120", variable );
    config->salesRepClasses->commissionMonths = i;
    return 0;
    }

  if( strcasecmp( variable, "REP_CLASS_PRODUCT" )==0 )
    {
    if( config->salesRepClasses==NULL )
      Error( "CONFIG: %s must follow REP_CLASS", variable );
    _PRODUCT* p = FindProduct( config->products, value );
    if( p==NULL )
      Error( "CONFIG: Cannot find product %s", value );
    AddProductToRepClass( config->salesRepClasses, p );
    return 0;
    }

  if( strcasecmp( variable, "REP_CLASS_VACATION" )==0 )
    {
    if( config->salesRepClasses==NULL )
      Error( "CONFIG: %s must follow REP_CLASS", variable );
    _VACATION* v = FindVacation( config->vacations, value );
    if( v==NULL )
      Error( "CONFIG: Cannot find vacation %s", value );
    if( config->salesRepClasses->allowance != NULL )
      Error( "CONFIG: Sales rep class %s already has a vacation allowance", config->salesRepClasses->id );
    config->salesRepClasses->allowance = v;
    return 0;
    }

  if( strcasecmp( variable, "REP_CLASS_AVG_MONTHS_EMPLOYMENT" )==0 )
    {
    if( config->salesRepClasses==NULL )
      Error( "CONFIG: %s must follow REP_CLASS", variable );
    double d = atof( value );
    if( d<1 || d>1000 )
      Error( "CONFIG: %s must be from 1 to 1000", variable );
    config->salesRepClasses->averageEmploymentMonths = d;
    return 0;
    }

  if( strcasecmp( variable, "REP_CLASS_SDEV_MONTHS_EMPLOYMENT" )==0 )
    {
    if( config->salesRepClasses==NULL )
      Error( "CONFIG: %s must follow REP_CLASS", variable );
    double d = atof( value );
    if( d<1 || d>config->salesRepClasses->averageEmploymentMonths )
      Error( "CONFIG: %s must be from 1 to %.1lf", config->salesRepClasses->averageEmploymentMonths );
    config->salesRepClasses->sdevEmploymentMonths = d;
    return 0;
    }

  if( strcasecmp( variable, "REP_CLASS_ANNUAL_INCREASE_PERCENT" )==0 )
    {
    if( config->salesRepClasses==NULL )
      Error( "CONFIG: %s must follow REP_CLASS", variable );
    double d = atof( value );
    if( d<0 || d>99 )
      Error( "CONFIG: %s must be from 0 to 99", variable );
    config->salesRepClasses->annualPayIncreasePercent = d;
    return 0;
    }

  if( strcasecmp( variable, "REP_CLASS_SALARY_ONLY" )==0 )
    {
    if( config->salesRepClasses==NULL )
      Error( "CONFIG: %s must follow REP_CLASS", variable );
    int isTrue = 0;
    if( strcasecmp( value, "true" )==0
        || strcasecmp( value, "yes" )==0 )
      isTrue = 1;
    else
      Error( "%s should be set to 'true' or 'yes'", variable );
    config->salesRepClasses->salaryOnly = isTrue;
    return 0;
    }

  if( strcasecmp( variable, "REP_CLASS_INITIATE_CALLS" )==0 )
    {
    if( config->salesRepClasses==NULL )
      Error( "CONFIG: %s must follow REP_CLASS", variable );
    int isTrue = 0;
    if( strcasecmp( value, "true" )==0
        || strcasecmp( value, "yes" )==0 )
      isTrue = 1;
    else
      Error( "%s should be set to 'true' or 'yes'", variable );
    config->salesRepClasses->initiateCalls = isTrue;
    return 0;
    }

  if( strcasecmp( variable, "REP_CLASS_AUTO_REPLACE" )==0 )
    {
    if( config->salesRepClasses==NULL )
      Error( "CONFIG: %s must follow REP_CLASS", variable );
    int isTrue = 0;
    if( strcasecmp( value, "true" )==0
        || strcasecmp( value, "yes" )==0 )
      isTrue = 1;
    else
      Error( "%s should be set to 'true' or 'yes'", variable );
    config->salesRepClasses->autoReplace = isTrue;
    return 0;
    }

  if( strcasecmp( variable, "DAYS_TO_REPLACE_REP_AVG" )==0 )
    {
    int n = atoi( value );
    if( n<1 || n>100 )
      Error( "%s must be from 1 to 100", variable );
    config->daysToAutoReplaceRepAvg = n;
    return 0;
    }

  if( strcasecmp( variable, "DAYS_TO_REPLACE_REP_SDEV" )==0 )
    {
    int n = atoi( value );
    if( n<0 || n>config->daysToAutoReplaceRepAvg )
      Error( "%s must be from 0 to %d", variable, config->daysToAutoReplaceRepAvg );
    config->daysToAutoReplaceRepSDev = n;
    return 0;
    }

  if( strcasecmp( variable, "SALES_REP" )==0 )
    {
    int seq = 0;
    if( config->salesReps!=NULL )
      seq = config->salesReps->seq + 1;

    config->salesReps = NewSalesRep( value, config->salesReps );
    config->salesReps->seq = seq;
    ++ (config->nSalesReps);
    return 0;
    }

  if( strcasecmp( variable, "SALES_REP_NAME" )==0 )
    {
    if( config->salesReps==NULL )
      Error( "CONFIG: %s must follow SALES_REP", variable );
    SetStringPointer( &(config->salesReps->name), value );
    return 0;
    }

  if( strcasecmp( variable, "SALES_REP_CLASS" )==0 )
    {
    if( config->salesReps==NULL )
      Error( "CONFIG: %s must follow SALES_REP", variable );
    _SALES_REP_CLASS* rt = FindSalesRepClass( config->salesRepClasses, value );
    if( rt==NULL )
      Error( "Cannot find sales rep class %s", value );
    config->salesReps->class = rt;
    config->salesReps->salaryOnly = rt->salaryOnly;

    return 0;
    }


  if( strcasecmp( variable, "SALES_REP_START" )==0 )
    {
    if( config->salesReps==NULL )
      Error( "CONFIG: %s must follow SALES_REP", variable );
    if( strcasecmp( value, "start" )==0 )
      {
      config->salesReps->firstDay = config->simulationFirstDay;
      }
    else
      {
      _MMDD date;
      if( IsValidMMDD( value, &date )!=0 )
        Error( "CONFIG: %s should be followed by a MM-DD day (both integers) - not [%s]", variable, value );
      config->salesReps->firstDay = date;
      }
    return 0;
    }

  if( strcasecmp( variable, "SALES_REP_FINISH" )==0 )
    {
    if( config->salesReps==NULL )
      Error( "CONFIG: %s must follow SALES_REP", variable );
    if( strcasecmp( value, "end-of-sim" )==0 )
      {
      config->salesReps->lastDay = config->simulationEndDay;
      /*
      Notice( "rep %s has last date end-of-sim which is %04d-%02d-%02d",
              config->salesReps->id,
              config->salesReps->lastDay.year,
              config->salesReps->lastDay.month,
              config->salesReps->lastDay.day );
      */
      }
    else if( strcasecmp( value, "random" )==0 )
      {
      _SALES_REP* r = config->salesReps;
      _SALES_REP_CLASS* class = r->class;
      if( class==NULL )
        Error( "You cannot assign %s=random before assigning rep to a class", variable );
      if( class->averageEmploymentMonths<=0 )
        Error( "You cannot assign %s=random if rep class has no REP_CLASS_AVG_MONTHS_EMPLOYMENT", variable );
      int nMonths = (int)RandN2( class->averageEmploymentMonths, class->sdevEmploymentMonths );
      time_t t1 = MMDDToTime( &(r->firstDay) );
      time_t t2 = t1 + 30 * nMonths * DAY_IN_SECONDS;
      if( t2 > config->simulationEnd )
        t2 = config->simulationEnd;
      if( TimeToMMDD( t2, &(r->lastDay) )!=0 )
        Error( "Failed to set end date for rep %s", r->id );
      }
    else
      {
      _MMDD date;
      if( IsValidMMDD( value, &date )!=0 )
        Error( "CONFIG: %s should be followed by a MM-DD day (both integers) - not [%s]", variable, value );
      config->salesReps->lastDay = date;
      }
    return 0;
    }

  if( strcasecmp( variable, "SALES_REP_ANNUAL_SALARY" )==0 )
    {
    if( config->salesReps==NULL )
      Error( "CONFIG: %s must follow SALES_REP", variable );
    double d = atof( value );
    if( d>100000000 )
      Error( "CONFIG: %s cannot exceed 100000000", variable );
    if( d<1 )
      Warning( "CONFIG: %s == %s --- unpaid worker?", variable, value );
    config->salesReps->annualPay = d;
    config->salesReps->monthlyPay = d / 12.0;
    return 0;
    }

  if( strcasecmp( variable, "SALES_REP_DAILY_CALLS" )==0 )
    {
    if( config->salesReps==NULL )
      Error( "CONFIG: %s must follow SALES_REP", variable );
    int n = atoi( value );
    if( n<1 || n>200 )
      Error( "CONFIG: %s must be from 1 to 200", variable );
    config->salesReps->dailyCalls = n;
    return 0;
    }

  if( strcasecmp( variable, "SALES_REP_SALARY_ONLY" )==0 )
    {
    if( config->salesReps==NULL )
      Error( "CONFIG: %s must follow SALES_REP", variable );
    int isTrue = 0;
    if( strcasecmp( value, "true" )==0
        || strcasecmp( value, "yes" )==0 )
      isTrue = 1;
    else
      Error( "SALES_REP_SALARY_ONLY should be set to 'true' or 'yes'" );
    config->salesReps->salaryOnly = isTrue;
    return 0;
    }

  if( strcasecmp( variable, "SALES_REP_HANDOFF_FEE" )==0 )
    {
    if( config->salesReps==NULL )
      Error( "CONFIG: %s must follow SALES_REP", variable );
    double d = atof( value );
    if( d<0 || d>100000 )
      Error( "%s has a value that's too high or negative", variable );
    config->salesReps->handoffFee = d;
    return 0;
    }

  if( strcasecmp( variable, "PAYMENT_PROCESSING_PERCENT" )==0 )
    {
    double p = atof( value );
    if( p<0 || p>30 )
      Error( "CONFIG: %s must be above 0 and less than 30\%", variable );
    config->percentageForPaymentProcessing = p;
    return 0;
    }

  if( strcasecmp( variable, "COLLECTIONS_DELAY_CALENDAR_DAYS_AVG" )==0 )
    {
    double p = atof( value );
    if( p<0 || p>365 )
      Error( "CONFIG: %s must be from 0 to 365\%", variable );
    config->averageCollectionsDelayDays = p;
    return 0;
    }

  if( strcasecmp( variable, "COLLECTIONS_DELAY_CALENDAR_DAYS_SDEV" )==0 )
    {
    double p = atof( value );
    if( p<0 || p>config->averageCollectionsDelayDays )
      Error( "CONFIG: %s must be from 0 to %.1lf\%", variable, p>config->averageCollectionsDelayDays );
    config->sdevCollectionsDelayDays = p;
    return 0;
    }

  if( strcasecmp( variable, "MARKET_SIZE" )==0 )
    {
    int s = atoi( value );
    if( s<0 || s>MAX_MARKET_SIZE )
      Error( "CONFIG: %s must be from 1 to %d", variable, MAX_MARKET_SIZE );
    config->marketSize = s;
    return 0;
    }

  if( strcasecmp( variable, "ORG_COOLING_PERIOD_DAYS" )==0 )
    {
    int d = atoi( value );
    if( d<0 || d>365 )
      Error( "CONFIG: %s must be from 1 to 365", variable );
    config->orgCoolingPeriodDays = d;
    return 0;
    }

  if( strcasecmp( variable, "LINK_STAGE_CLASS" )==0 )
    {
    char* ptr = NULL;
    char* stageName = strtok_r( value, " \r\t\n", &ptr );
    char* className = strtok_r( NULL, " \r\t\n", &ptr );
    if( EMPTY( className ) || EMPTY( stageName ) )
      Error( "LINK_CLASS_STAGE must specify a stage and a rep class" );
    _SALES_STAGE* stage = FindSalesStage( config->stages, stageName );
    if( stage==NULL )
      Error( "%s specified a non-existing sales stage (%s)", variable, stageName );
    _SALES_REP_CLASS* class = FindSalesRepClass( config->salesRepClasses, className );
    if( class==NULL )
      Error( "%s specified a non-existing sales stage (%s)", variable, className );
    stage->repClasses = NewClassPointer( class, stage->repClasses );
    return 0;
    }

  /* super simple model for now */
  if( strcasecmp( variable, "TAX_RATE_PERCENT" )==0 )
    {
    double r = atof( value );
    if( r<=0 || r>=50 )
      Error( "A tax rate of [%s] is not believable (0-50 is real?)", value );
    config->taxRate = r;
    return 0;
    }

  if( strcasecmp( variable, "INITIAL_CASH_BALANCE" )==0 )
    {
    double b = atof( value );
    if( b<0 || b>100000000 )
      Error( "The initial cash balance of %s is not credible", value );
    config->initialCashBalance = b;
    return 0;
    }

  enum eventType eventType = et_invalid;

  if( strcasecmp( variable, "INVESTMENT" )==0 )
    eventType = et_investment;
  else if( strcasecmp( variable, "GRANT" )==0 )
    eventType = et_grant;
  else if( strcasecmp( variable, "TAX_REFUND" )==0 )
    eventType = et_tax_refund;
  else if( strcasecmp( variable, "ONE_TIME_INCOME" )==0 )
    eventType = et_one_time_income;
  else if( strcasecmp( variable, "ONE_TIME_EXPENSE" )==0 )
    eventType = et_one_time_expense;

  if( eventType != et_invalid )
    {
    char* ptr = NULL;
    char* valueStr = strtok_r( value, "@ \r\n", &ptr );
    char* dateStr = strtok_r( NULL, "@ \r\n", &ptr );
    if( EMPTY( dateStr ) || EMPTY( valueStr ) )
      Error( "%s should have a value number@date (date==CCYY-MM-DD)", variable );
    double v = atof( valueStr );
    if( v<0 || v>10000000 )
      Error( "Value of %.1lf not plausible", v );
    _MMDD when = { 0 };
    if( IsValidMMDD( dateStr, &when )!=0 )
      Error( "Date of %s does not parse - should be CCYY-MM-DD", dateStr );
    config->cashEvents = NewCashEvent( eventType,
                                       when,
                                       v,
                                       config->cashEvents );
    return 0;
    }

  Warning( "Unrecognized keyword in config: [%s]", variable );

  return -1;
  }

void ProcessConfigLine( char* ptr, char* equalsChar, _CONFIG* config )
  {
  *equalsChar = 0;

  char* variable = TrimHead( ptr );
  TrimTail( variable );
  char* value = TrimHead( equalsChar+1 );
  TrimTail( value );

  char* hashPtr = strchr( value, '#' );
  if( hashPtr!=NULL )
    {
    *hashPtr = 0;
    TrimTail( value );
    }

  /* indicates that we used strdup() to recompute the value ptr */
  int allocatedValue = 0;

  if( NOTEMPTY( variable ) && NOTEMPTY( value ) )
    {
    char valueBuf[BUFLEN];

    /* expand any macros in the value */
    if( strchr( value, '$' )!=NULL )
      {
      int loopMax = 10;
      while( loopMax>0 )
        {
        int n = ExpandMacros( value, valueBuf, sizeof( valueBuf ), config->list );
        if( n>0 )
          {
          if( allocatedValue )
            FREE( value );
          value = strdup( valueBuf );
          allocatedValue = 1;
          }
        else
          {
          break;
          }
        --loopMax;
        }
      }

    config->list = NewTagValue( variable, value, config->list, 1 );

    if( ProcessKeywordPair( config, variable, value )!=0 )
      Error( "Failed to process CONFIG: %s=%s", NULLPROTECT( variable ), NULLPROTECT( value ) );
    }

  if( allocatedValue )
    FREE( value );
  }

void PrintConfig( FILE* f, _CONFIG* config )
  {
  if( f==NULL )
    {
    Error("Cannot print configuration to NULL file");
    }

  fprintf( f, "# Timeline\n" );
  if( EmptyMMDD( &(config->simulationFirstDay) )!=0 )
    fprintf( f, "FIRST_DAY=%04d-%02d-%02d\n",
             config->simulationFirstDay.year,
             config->simulationFirstDay.month,
             config->simulationFirstDay.day );

  if( config->simulationDurationDays > 0 )
    fprintf( f, "DURATION=%d\n", config->simulationDurationDays );
  fprintf( f, "\n" );

  fprintf( f, "# Stat holidays\n" );
  for( _HOLIDAY* h = config->holidays; h!=NULL; h=h->next )
    PrintHoliday( f, h );
  fprintf( f, "\n" );

  fprintf( f, "# Vacation categories\n" );
  for( _VACATION* v = config->vacations; v!=NULL; v=v->next )
    PrintVacation( f, v );
  fprintf( f, "\n" );

  fprintf( f, "# Sales stages\n" );
  _SALES_STAGE** stageArray = NULL;
  int nStages = SalesStagesArray( config->stages, &(stageArray) );
  for( int i=0; i<nStages; ++i )
    {
    _SALES_STAGE* s = stageArray[i];
    PrintSalesStage( f, s );
    }
  free( stageArray );
  fprintf( f, "\n" );

  fprintf( f, "# Products\n" );
  for( _PRODUCT* p = config->products; p!=NULL; p=p->next )
    PrintProduct( f, p );
  fprintf( f, "\n" );

  fprintf( f, "# Classes of sales reps\n" );
  for( _SALES_REP_CLASS* rt = config->salesRepClasses; rt!=NULL; rt=rt->next )
    PrintSalesRepClass( f, rt );
  fprintf( f, "\n" );

  fprintf( f, "# Individual sales reps\n" );
  for( _SALES_REP* r = config->salesReps; r!=NULL; r=r->next )
    PrintSalesRep( f, r );
  fprintf( f, "\n" );

  if( config->daysToAutoReplaceRepAvg != DEFAULT_DAYS_TO_AUTO_REPLACE_SALES_REP )
    fprintf( f, "DAYS_TO_REPLACE_REP_AVG=%d\n", config->daysToAutoReplaceRepAvg );
  if( config->daysToAutoReplaceRepSDev != DEFAULT_DAYS_TO_AUTO_REPLACE_SALES_REP_SDEV )
    fprintf( f, "DAYS_TO_REPLACE_REP_SDEV=%d\n", config->daysToAutoReplaceRepSDev );
  fprintf( f, "\n" );

  fprintf( f, "# Cost of collecting from customers - either payment processing or invoice/payment delay:\n" );
  if( config->percentageForPaymentProcessing > 0 )
    fprintf( f, "PAYMENT_PROCESSING_PERCENT=%.1lf\n", config->percentageForPaymentProcessing );

  if( config->averageCollectionsDelayDays > 0 )
    fprintf( f, "COLLECTIONS_DELAY_CALENDAR_DAYS_AVG=%.1lf\n", config->averageCollectionsDelayDays );

  if( config->sdevCollectionsDelayDays > 0 )
    fprintf( f, "COLLECTIONS_DELAY_CALENDAR_DAYS_SDEV=%.1lf\n", config->sdevCollectionsDelayDays );
  fprintf( f, "\n" );

  if( config->marketSize > 0 || config->orgCoolingPeriodDays > 0 )
    fprintf( f, "# Variables related to saturating the target market:\n" );

  if( config->marketSize > 0 )
    fprintf( f, "MARKET_SIZE=%d\n", config->marketSize );

  if( config->orgCoolingPeriodDays > 0 )
    fprintf( f, "ORG_COOLING_PERIOD_DAYS=%d\n", config->orgCoolingPeriodDays );
  fprintf( f, "\n" );

  fprintf( f, "# Which rep classes can work on which sales stages?\n" );
  for( _SALES_STAGE* stage = config->stages; stage!=NULL; stage=stage->next )
    for( _CLASS_POINTER* cp=stage->repClasses; cp!=NULL; cp=cp->next )
      fprintf( f, "LINK_STAGE_CLASS %s %s\n", NULLPROTECT( stage->id ), NULLPROTECT( cp->class->id ) );
  fprintf( f, "\n" );

  fprintf( f, "# cashflow modeling related parameters (if any):\n" );
  if( config->taxRate>0 )
    {
    fprintf( f, "TAX_RATE_PERCENT=%.1lf\n", config->taxRate );
    fprintf( f, "\n" );
    }

  if( config->initialCashBalance>0 )
    {
    fprintf( f, "INITIAL_CASH_BALANCE=%.1lf\n", config->initialCashBalance );
    fprintf( f, "\n" );
    }

  /* all configured cash events */
  PrintAllCashEvents( config, f );
  }


void ReadConfig( _CONFIG* config, char* filePath )
  {
  char folder[BUFLEN];
  folder[0] = 0;
  (void)GetFolderFromPath( filePath, folder, sizeof( folder )-1 );

  /* Notice( "Config is being read from folder [%s]", folder ); */

  char* oldConfigFolder = NULL;
  if( EMPTY( folder ) )
    config->configFolder = NULL;
  else
    {
    if( NOTEMPTY( config->configFolder ) )
      oldConfigFolder = config->configFolder;
    config->configFolder = strdup( folder );
    }

  if( EMPTY( filePath ) )
    {
    Error( "Cannot read configuration file with empty/NULL name");
    }

  FILE* f = fopen( filePath, "r" );
  if( f==NULL )
    {
    Error( "Failed to open configuration file %s", filePath );
    }

  config->parserLocation = NewTagValue( filePath, "", config->parserLocation, 0 );
  config->parserLocation->iValue = 0;
  UpdateGlobalParsingLocation( config );
  ++ ( config->currentlyParsing );

  /* this is wrong if we have #include's
  SetDefaults( config );
  */

  char buf[BUFLEN];
  char* endOfBuf = buf + sizeof(buf)-1;
  while( fgets(buf, sizeof(buf)-1, f )==buf )
    {
    ++ (config->parserLocation->iValue);
    UpdateGlobalParsingLocation( config );

    char* ptr = TrimHead( buf );
    TrimTail( ptr );

    while( *(ptr + strlen(ptr) - 1)=='\\' )
      {
      char* startingPoint = ptr + strlen(ptr) - 1;
      if( fgets(startingPoint, endOfBuf-startingPoint-1, f )!=startingPoint )
        {
        ++ (config->parserLocation->iValue);
        UpdateGlobalParsingLocation( config );
        break;
        }
      ++ (config->parserLocation->iValue);
      UpdateGlobalParsingLocation( config );
      TrimTail( startingPoint );
      }

    if( *ptr==0 )
      {
      continue;
      }

    if( *ptr=='#' )
      {
      ++ptr;
      if( strncmp( ptr, "include", 7 )==0 )
        { /* #include */
        ptr += 7;
        while( *ptr!=0 && ( *ptr==' ' || *ptr=='\t' ) )
          {
          ++ptr;
          }
        if( *ptr!='"' )
          {
          Error("#include must be followed by a filename in \" marks.");
          }
        ++ptr;
        char* includeFileName = ptr;
        while( *ptr!=0 && *ptr!='"' )
          {
          ++ptr;
          }
        if( *ptr=='"' )
          {
          *ptr = 0;
          }
        else
          {
          Error("#include must be followed by a filename in \" marks.");
          }

        int redundantInclude = 0;
        for( _TAG_VALUE* i=config->includes; i!=NULL; i=i->next )
          {
          if( NOTEMPTY( i->tag ) && strcmp( i->tag, includeFileName )==0 )
            {
            redundantInclude = 1;
            break;
            }
          }

        if( redundantInclude==0 )
          {
          config->includes = NewTagValue( includeFileName, "included", config->includes, 1 );

          if( config->listIncludes )
            {
            if( config->includeCounter )
              {
              fputs( " ", stdout );
              }
            fputs( includeFileName, stdout );
            ++ (config->includeCounter);
            }

          char* confPath = SanitizeFilename( CONFIGDIR, NULL, includeFileName, 0 );
          if( FileExists( confPath )==0 )
            {
            ReadConfig( config, confPath );
            FREE( confPath );
            }
          else
            {
            FREE( confPath );
            confPath = SanitizeFilename( folder, NULL, includeFileName, 0 );
            if( FileExists( confPath )==0 )
              {
              ReadConfig( config, confPath );
              FREE( confPath );
              }
            else
              {
              Warning( "Cannot open #include \"%s\" -- skipping.",
                       confPath );
              FREE( confPath );
              }
            }
          }
        }
      else if( strncmp( ptr, "print", 5 )==0 )
        { /* #print */
        ptr += 5;
        while( *ptr!=0 && ( *ptr==' ' || *ptr=='\t' ) )
          {
          ++ptr;
          }
        if( *ptr!='"' )
          {
          Error("#include must be followed by a filename in \" marks.");
          }
        ++ptr;
        char* printFileName = ptr;
        while( *ptr!=0 && *ptr!='"' )
          {
          ++ptr;
          }
        if( *ptr=='"' )
          {
          *ptr = 0;
          }
        else
          {
          Error("#print must be followed by a filename in \" marks.");
          }

        FILE* printFile = fopen( printFileName, "w" );
        if( printFile==NULL )
          {
          Error( "Could not open/create %s to print configuration.",
                 printFileName );
          }
        PrintConfig( printFile, config );
        fclose( printFile );
        Notice( "Printed configuration to %s.", printFileName );
        }
      else if( strncmp( ptr, "exit", 4 )==0 )
        { /* #exit */
        ptr += 4;
        ValidateConfig( config );
        Notice( "Exit program due to command in config file." );
        exit(0);
        }

      /* not #include or #include completely read by now */
      continue;
      }

    /* printf("Processing [%s]\n", ptr ); */
    char* equalsChar = NULL;
    for( char* eolc = ptr; *eolc!=0; ++eolc )
      {
      if( equalsChar==NULL && *eolc == '=' )
        {
        equalsChar = eolc;
        }

      if( *eolc == '\r' || *eolc == '\n' )
        {
        *eolc = 0;
        break;
        }
      }

    if( *ptr!=0 && equalsChar!=NULL && equalsChar>ptr )
      {
      ProcessConfigLine( ptr, equalsChar, config );
      }
    }

  /* unroll the stack of config filenames after ReadConfig ended */
  _TAG_VALUE* tmp = config->parserLocation->next;
  if( config->parserLocation->tag!=NULL ) { FREE( config->parserLocation->tag ); }
  if( config->parserLocation->value!=NULL ) { FREE( config->parserLocation->value ); }
  FREE( config->parserLocation );
  config->parserLocation = tmp;
  UpdateGlobalParsingLocation( config );
  -- ( config->currentlyParsing );

  fclose( f );

  if( NOTEMPTY( config->configFolder ) )
    {
    free( config->configFolder );
    config->configFolder = NULL;
    }
  if( oldConfigFolder != NULL )
    {
    config->configFolder = oldConfigFolder;
    }

  /*
  This is wrong if we have #include's !
  FreeTagValue( config->list );
  config->list = NULL;
  */
  }

void ValidateConfig( _CONFIG* config )
  {
  if( config==NULL )
    Error( "Cannot validate a NULL configuration" );

  /* QQQ new location */
  printf( "Initializing monthly summary array for the config as a whole.\n" );
  InitializeMonthlySummaryArray( "Simulation",
                                 &(config->monthlySummary),
                                 &(config->nMonths),
                                 NULL,
                                 0,
                                 &(config->simulationFirstDay),
                                 &(config->simulationEndDay) );

  /* avoid doing this twice */
  if( config->baselineWorkDays==NULL )
    CalculateBaselineWorkingDays( config );

  /* create the customer care department - modeled as a 'magical' sales rep */
  if( config->customerCare==NULL )
    {
    /* Notice( "Creating customer care" ); */
    config->customerCare = NewSalesRep( "customer-care", NULL );
    config->customerCare->class = NULL;
    config->customerCare->name = strdup( "Customer Care" );
    config->customerCare->firstDay = config->simulationFirstDay;
    config->customerCare->lastDay = config->simulationEndDay;
    config->customerCare->annualPay = 0;
    config->customerCare->monthlyPay = 0;
    config->customerCare->dailyCalls = 0;
    }

  if( EmptyMMDD( &(config->simulationFirstDay) )==0 )
    Error( "Simulation must have a FIRST_DAY (%04d-%02d-%02d)",
           config->simulationFirstDay.year,
           config->simulationFirstDay.month,
           config->simulationFirstDay.day );
  if( config->simulationDurationDays<=0 )
    Error( "Simulation must have a DURATION" );
  if( EmptyMMDD( &(config->simulationEndDay) )==0 )
    Error( "Simulation must have a FIRST_DAY and DURATION" );
  if( config->holidays==NULL )
    Error( "Simulation must specify holidays (statutory non-work days)" );
  if( config->vacations==NULL )
    Error( "Simulation must specify vacations (classes of time off given to people)" );
  if( config->nBaselineWorkDays<=0 || config->baselineWorkDays==NULL )
    Error( "Something went wrong calculating the number of baseline work days (%d:%p)",
           config->nBaselineWorkDays, config->baselineWorkDays );
  if( config->stages==NULL )
    Error( "Simulation must specify at least one sales STAGE" );
  if( config->products==NULL )
    Error( "Simulation must specify at least one sales PRODUCT" );
  if( config->salesReps==NULL )
    Error( "Simulation must specify at least one sales SALES_REP" );
  if( config->customerCare==NULL )
    Error( "Something went wrong - there is no CUSTOMER_CARE built-in sales rep" );

  if( ( config->sdevCollectionsDelayDays==0 
      && config->averageCollectionsDelayDays!=0 )
      || ( config->sdevCollectionsDelayDays!=0 
           && config->averageCollectionsDelayDays==0 ) )
    Error( "If you specify one of COLLECTIONS_DELAY_CALENDAR_DAYS_AVG, COLLECTIONS_DELAY_CALENDAR_DAYS_SDEV, you must specify both" );

  if( config->sdevCollectionsDelayDays > config->averageCollectionsDelayDays )
    Error( "COLLECTIONS_DELAY_CALENDAR_DAYS_AVG must be greater than COLLECTIONS_DELAY_CALENDAR_DAYS_SDEV" );

  for( _HOLIDAY* h = config->holidays; h!=NULL; h=h->next )
    {
    int err = ValidateSingleHoliday( h );
    if( err )
      Error( "Holiday does not validate - %s (error %d)", h->id, err );
    }

  for( _VACATION* v = config->vacations; v!=NULL; v=v->next )
    {
    int err = ValidateSingleVacation( v );
    if( err )
      Error( "Vacation does not validate - %s (error %d)", v->id, err );
    }

  _SALES_STAGE** stageArray = NULL;
  int nStages = SalesStagesArray( config->stages, &(stageArray) );
  if( nStages<1 )
    Error( "Config must specify at least one SALES_STAGE" );

  int nStagesNoDeps = 0;
  for( int i=0; i<nStages; ++i )
    {
    _SALES_STAGE* s = stageArray[i];
    if( ValidateSingleStage( s )!=0 )
      Error( "Sales stage does not validate - %s", s->id );
    if( s->predecessor==NULL )
      ++nStagesNoDeps;
    }
  free( stageArray );

  /* not true - at least one per product...
  if( nStagesNoDeps!=1 )
    Error( "There must be exactly one SALES_STAGE with no predecessor" );
  */

  for( _PRODUCT* p = config->products; p!=NULL; p=p->next )
    {
    if( ValidateSingleProduct( p )!=0 )
      Error( "Product does not validate - %s", p->id );
    }

  for( _SALES_REP_CLASS* rt = config->salesRepClasses; rt!=NULL; rt=rt->next )
    {
    if( ValidateSingleSalesRepClass( rt )!=0 )
      Error( "Sales rep class does not validate - %s", rt->id );
    }

  for( _SALES_REP* r = config->salesReps; r!=NULL; r=r->next )
    {
    if( ValidateSingleSalesRep( r, config )!=0 )
      Error( "Sales rep does not validate - %s", r->id );
    }

  if( ( config->marketSize!=0 && config->orgCoolingPeriodDays==0 )
      || ( config->marketSize==0 && config->orgCoolingPeriodDays!=0 ) )
    Error( "If either MARKET_SIZE or ORG_COOLING_PERIOD_DAYS is specified, the other must also be set." );

  if( config->marketSize > 0 )
    {
    config->orgs = (_ORG*)SafeCalloc( config->marketSize, sizeof(_ORG), "orgs - to track cooling period" );
    _ORG* o = config->orgs;
    for( int i=0; i<config->marketSize; ++i )
      {
      o->number = i;
      ++o;
      }
    config->nAvailableOrgs = config->marketSize;
    }
  }

