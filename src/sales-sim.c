#include "base.h"

_SINGLE_DAY* FindSingleDay( _MMDD* date, _SINGLE_DAY* array, int arrayLen )
  {
  if( date==NULL || array==NULL || arrayLen<1 )
    return NULL;

  if( array->date.year==date->year
      && array->date.month==date->month
      && array->date.day==date->day )
    return array;

  time_t t1 = MMDDToTime( date );
  time_t t2 = MMDDToTime( &(array->date) );

  if( t2 > t1 )
    {
    Warning( "FindSingleDay - array starts later than %04d-%02d-%02d",
             date->year, date->month, date->day );
    return NULL;
    }

  int nDays = t1-t2;
  nDays /= DAY_IN_SECONDS;
  if( nDays >= arrayLen )
    {
    /*
    Warning( "FindSingleDay - array ends before %04d-%02d-%02d (ndays %d >= arrayLen %d)",
             date->year, date->month, date->day, nDays, arrayLen );
    */
    return NULL;
    }

  _SINGLE_DAY* ptr = array + nDays;
  return ptr;
  }

_SINGLE_DAY* FindThisDateNextMonth( _SINGLE_DAY* start, _SINGLE_DAY* tombstone, time_t* tTarget )
  {
  if( start==NULL )
    return NULL;
  if( tombstone==NULL )
    return NULL;

  int nextMonth = start->date.month + 1;
  int nextYear = start->date.year;
  if( nextMonth==13 )
    {
    nextMonth = 1;
    ++ nextYear;
    }
  int daysInThisMonth = DaysInMonth( start->date.year, start->date.month );
  int daysInNextMonth = DaysInMonth( nextYear, nextMonth );

  int targetDay = start->date.day;
  while( targetDay > daysInNextMonth )
    --targetDay;

  int nSkip = daysInThisMonth - start->date.day
            + targetDay;

  if( tTarget!=NULL )
    {
    _MMDD targetDate;
    targetDate.year = nextYear;
    targetDate.month = nextMonth;
    targetDate.day = targetDay;
    *tTarget = MMDDToTime( &targetDate );
    }

  _SINGLE_DAY* possible = start + nSkip;
  if( possible >= tombstone )
    { /* past end of array */
    return NULL;
    }

  if( possible->date.day != targetDay )
    { /* something went wrong */
    Warning( "Looking for %04d-%02d-%02d but got %04d-%02d-%02d - abort search",
             nextYear, nextMonth, targetDay,
             possible->date.year, possible->date.month, possible->date.day );
    return NULL;
    }

  return possible;
  }

void CloseSingleSale( _CONFIG* conf,
                      _SALES_REP* salesRep,
                      _ORG* targetOrg,
                      _SINGLE_DAY* repFirstDay,
                      _SINGLE_DAY* repLastDay,
                      _PRODUCT* product,
                      double overrideRevenue,
                      int overrideUnits )
  {
  if( conf==NULL )
    return;
  if( salesRep==NULL )
    return;
  if( salesRep!=conf->customerCare && salesRep->class==NULL )
    return;
  if( repFirstDay==NULL )
    return;
  if( repLastDay==NULL )
    return;
  if( product==NULL )
    return;
  if( EMPTY( product->id ) )
    return;
  if( (product->marketSize>0) && (targetOrg==NULL) )
    Error( "CloseSingleSale() for product %s with market size %d but no target org",
           product->id, product->marketSize );

  if( repFirstDay >= salesRep->workDays
      && repFirstDay <= salesRep->endOfWorkDays )
    {}
  else
    Warning( "CloseSingleSale() with repFirstDay not within salesRep range" );

  if( repLastDay >= salesRep->workDays
      && repLastDay <= salesRep->endOfWorkDays )
    {}
  else
    Warning( "CloseSingleSale() with repLastDay not within salesRep range" );

  /* note that targetOrg is permitted to be NULL */

  Event( "CloseSingleSale( rep=%s, firstDay=%04d-%02d-%02d, lastDay(-1)=%04d-%02d-%02d, product=%s, overrideUnits=%d",
          salesRep->id,
          repFirstDay->date.year, repFirstDay->date.month, repFirstDay->date.day,
          (repLastDay-1)->date.year, (repLastDay-1)->date.month, (repLastDay-1)->date.day,
          product->id, overrideUnits );

  _SINGLE_DAY* thisDay = repFirstDay;

  ++ (conf->nCustomerWins);
  ++ (product->nCustomerWins);

  int customer = 0;
  if( targetOrg!=NULL )
    customer = targetOrg->number;
  else
    {
    ++ (conf->customerNumber );
    customer = conf->customerNumber;
    }

  /* Random if really simulating, fixed if the revenue was provided (i.e., initial revenue): */

  double initialRevenue = 0;
  double finalRevenue = 0;
  double monthlyGrowthRate = 1.0 + product->monthlyGrowthRatePercent/100.0;
  int monthsToSteadyState = RandN2( product->averageMonthsToReachSteadyState, product->sdevMonthsToReachSteadyState );

  int initialUnits = overrideUnits;
  int finalUnits = overrideUnits;
  double unitOnboardingFee = 0.0;
  double unitMonthlyFee = 0.0;

  if( overrideRevenue>0 )
    { /* pre-specified pricing.  likely initial revenue for the sim. */
    if( product->priceByUnits )
      {
      unitMonthlyFee = overrideRevenue;
      monthsToSteadyState = 0;
      monthlyGrowthRate = 1;
      }
    else
      {
      finalRevenue = overrideRevenue;
      monthsToSteadyState = 0;
      monthlyGrowthRate = 1;
      initialRevenue = overrideRevenue; 
      }
    }
  else
    { /* random pricing */
    if( product->priceByUnits )
      {
      finalUnits = (int)(round( RandN2( product->averageCustomerSizeUnits, product->sdevCustomerSizeUnits ) ));
      if( product->averageMonthlyGrowthRateUnits ) /* add this many monthly */
        initialUnits = 0;
      else if( monthlyGrowthRate > 1.0 )
        initialUnits = (int)(round( finalUnits / pow( monthlyGrowthRate, (double)monthsToSteadyState ) ));
      else
        initialUnits = finalUnits;
      unitOnboardingFee = RandN2( product->averageUnitOnboardingFee, product->sdevUnitOnboardingFee );
      unitMonthlyFee = RandN2( product->averageUnitMonthlyRecurringFee, product->sdevUnitMonthlyRecurringFee );
      }
    else
      {
      finalRevenue = RandN2( product->averageMonthlyDealSize, product->sdevDealSize );
      initialRevenue = finalRevenue / pow( monthlyGrowthRate, (double)monthsToSteadyState );
      }
    }

  if( thisDay->month )
    ++ (thisDay->month->nWins);

  int productFixedMonthlyUnitsGrowth = 0;
  if( product->averageMonthlyGrowthRateUnits>0 )
    {
    productFixedMonthlyUnitsGrowth = (int)round( RandN2( product->averageMonthlyGrowthRateUnits,
                                                         product->sdevMonthlyGrowthRateUnits ) );
    }

  char* prodID = "(unknown)";
  if( product!=NULL && NOTEMPTY( product->id ) )
    prodID = product->id;

  Event( "Win selling %s at customer %d on %04d-%02d-%02d",
         prodID, customer,
         thisDay->date.year, thisDay->date.month, thisDay->date.day );

  if( product->priceByUnits )
    {
    Event( ".. unitOnboardingFee = %.1lf", unitOnboardingFee );
    Event( ".. unitMonthlyFee = %.1lf", unitMonthlyFee );
    Event( ".. initialUnits = %d", initialUnits );
    Event( ".. finalUnits = %d", finalUnits );
    }
  else
    {
    if( initialRevenue>0 )
      Event( ".. initialRevenue = %.1lf", initialRevenue );
    Event( ".. finalRevenue = %.1lf", finalRevenue );
    }
  Event( ".. monthsToSteadyState = %d", monthsToSteadyState );
  Event( ".. monthlyGrowthRate = %.1lf", monthlyGrowthRate );
  Event( ".. monthly product attrition is = %.2lf", product->probabilityOfCustomerAttritionPerMonth );

  if( product->averageMonthlyGrowthRateUnits>0 )
    {
    Event( ".. customer will add %d units monthly", productFixedMonthlyUnitsGrowth );
    }

  /* process monthly revenue for this sales rep */
  int units = initialUnits;
  double revenue = initialRevenue;
  int monthNo = 0;
  _SALES_REP* repBeingPaid = salesRep;

  while( thisDay != NULL
         && repBeingPaid != NULL
         && thisDay < repBeingPaid->endOfWorkDays )
    {
    char* repID = "(nobody)";
    if( repBeingPaid!=NULL && NOTEMPTY( repBeingPaid->id ) )
      repID = repBeingPaid->id;

    if( PercentProbabilityEvent( product->probabilityOfCustomerAttritionPerMonth ) )
      {
      if( targetOrg!=NULL )
        targetOrg->lostForever = 1;

      Event( "Lost customer %d on %04d-%02d-%02d (month %d)",
        customer,
        thisDay->date.year, thisDay->date.month, thisDay->date.day,
        monthNo );

      if( thisDay->month )
        {
        ++ (thisDay->month->nLosses );
        -- (thisDay->month->nCustomers );
        }

      if( targetOrg != NULL )
        OrgAttrition( conf, product, targetOrg, thisDay->t );

      break; /* lost the customer */
      }

    /* stats - # of customer-months in sim */
    ++ monthNo;
    ++ (conf->nCustomerMonths);
    ++ (product->nCustomerMonths);
    if( thisDay->month )
      ++ (thisDay->month->nCustomers);

    /* calculate revenue based on number of units */
    if( product->priceByUnits )
      {
      int unitsToAdd = 0;

      if( units< finalUnits )
        {
        if( productFixedMonthlyUnitsGrowth > 0 ) /* fixed */
          unitsToAdd = productFixedMonthlyUnitsGrowth;
        else /* exponential */
          unitsToAdd = (int)round(((double)units * monthlyGrowthRate)) - units;
        }

      units += unitsToAdd;

      revenue = 0;
      if( unitsToAdd > 0 )
        {
        revenue += unitOnboardingFee * unitsToAdd;
        Event( ".. %04d-%02d-%02d %s sold %d additional units (reaching %d) at %.1lf onboarding fee to org %d",
               thisDay->date.year, thisDay->date.month, thisDay->date.day,
               repID, unitsToAdd, units, unitOnboardingFee, customer );
        }

      if( units > 0 )
        {
        revenue += unitMonthlyFee * units;
        Event( ".. %04d-%02d-%02d %s maintained %d units at %.1lf subscription fee to org %d",
               thisDay->date.year, thisDay->date.month, thisDay->date.day,
               repID, units, unitMonthlyFee, customer );
        }

      if( thisDay->month ) /* track stats for the month */
        SetMonthlyUnits( conf, thisDay->month, product, targetOrg, units );
      }

    double actualRevenue = revenue;
    if( conf->percentageForPaymentProcessing>0 )
      actualRevenue *= (100.0 - conf->percentageForPaymentProcessing)/100.0;

    _SINGLE_DAY* payDay = thisDay;

    int nDays = 0;
    if( conf->averageCollectionsDelayDays>0 && conf->sdevCollectionsDelayDays>=0 )
      {
      nDays = (int)RandN2( conf->averageCollectionsDelayDays, conf->sdevCollectionsDelayDays );
      payDay += nDays;
      if( payDay < repLastDay )
        Event( ".. revenue event was on %04d-%02d-%02d but pay day delayed by %d days to %04d-%02d-%02d",
                 thisDay->date.year, thisDay->date.month, thisDay->date.day,
                 nDays,
                 payDay->date.year, payDay->date.month, payDay->date.day );
      else
        Event( ".. revenue event was on %04d-%02d-%02d but pay day delayed by %d days to (too late for this rep)",
                 thisDay->date.year, thisDay->date.month, thisDay->date.day,
                 nDays );
      }

    /* switch pay day to customer care if it's past end of life for the original rep */
    if( payDay >= repLastDay )
      {
      if( repBeingPaid != conf->customerCare )
        {
        Event( ".. %04d-%02d-%02d Switching rep to customer care",
               thisDay->date.year, thisDay->date.month, thisDay->date.day );
        int thisDayNumber = thisDay - repBeingPaid->workDays;
        int payDayNumber = payDay - repBeingPaid->workDays;
        repBeingPaid = conf->customerCare;
        repLastDay = repBeingPaid->endOfWorkDays;
        payDay = repBeingPaid->workDays + payDayNumber; /* switch it to point into the new array */
        thisDay = repBeingPaid->workDays + thisDayNumber;

        /* switch over our date pointers to use customer care's array */
        for( _SINGLE_DAY* trialDate = repBeingPaid->workDays;
             trialDate < repBeingPaid->endOfWorkDays;
             ++trialDate )
          {
          if( SameDay( &(trialDate->date), &(payDay->date) )==0 )
            {
            payDay = trialDate;
            break;
            }
          }
        }
      }

    /* possibly we've over-extended even customer care?  exit then. */
    if( payDay >= repBeingPaid->endOfWorkDays )
      {
      Event( "%04d-%02d-%02d Cannot pay beyond %s last day",
             thisDay->date.year, thisDay->date.month, thisDay->date.day,
             repBeingPaid->id );
      break;
      }


    /* revenue happens regardless of who the rep now is */
    payDay->dailySales = NewRevenueEvent( conf, payDay->date, customer, repBeingPaid, monthNo, actualRevenue, payDay->dailySales );

    /*
    Event( ".. %04d-%02d-%02d repBeingPaid=%s salesRep=%s salesRep->class=%s monthNo=%d repBeingPaid->class->commissionMonths=%d",
            payDay->date.year, payDay->date.month, payDay->date.day,
            ( repBeingPaid==NULL || EMPTY( repBeingPaid->id ) ) ? "nil" : repBeingPaid->id,
            ( salesRep==NULL || EMPTY( salesRep->id ) ) ? "nil" : salesRep->id,
            ( salesRep==NULL || salesRep->class==NULL || EMPTY( salesRep->class->id ) ) ? "nil" : salesRep->class->id,
            monthNo,
            repBeingPaid->class==NULL ? -1 : repBeingPaid->class->commissionMonths );
    */

    /* pay commission if it's the original sales rep, and we have not overrun
       the number of commission months */
    if( repBeingPaid==salesRep
        && salesRep->class!=NULL 
        && monthNo < repBeingPaid->class->commissionMonths )
      {
      payDay->fees = NewPayEvent( conf, payDay->date, salesRep, pt_commission, revenue * salesRep->class->commission / 100.0, payDay->fees );
      Event( ".. %04d-%02d-%02d Rep %s gets paid %.1lf for %.1lf in sales to customer %d (month %d of deal)",
             payDay->date.year, payDay->date.month, payDay->date.day,
             salesRep->id, payDay->fees->amount, payDay->dailySales->revenue, customer, monthNo );
      }
    else
      {
      Event( ".. %04d-%02d-%02d Nobody gets commission on revenue of %.1lf from customer %d (month %d of deal)",
             payDay->date.year, payDay->date.month, payDay->date.day,
             payDay->dailySales->revenue, customer, monthNo );
      }

    /* monthly growth */
    if( ! product->priceByUnits )
      { /* revenue is done by unit if priced by units */
      if( revenue < finalRevenue )
        revenue *= monthlyGrowthRate;
      }

    _SINGLE_DAY* lastGoodDay = thisDay;
    thisDay = FindThisDateNextMonth( thisDay, repLastDay, NULL );

    /* couldn't find the next date, but perhaps we have yet to transition to customer care? */
    if( thisDay==NULL && repBeingPaid != conf->customerCare )
      {
      Event( ".. reached end of available dates for %s", repID );
      thisDay = FindSingleDay( &(lastGoodDay->date),
                               conf->customerCare->workDays,
                               conf->customerCare->nWorkDays );
      if( thisDay==NULL )
        {
        Event( ".. next customer care date (after %04d-%02d-%02d) not found.  Ending this sale.",
               lastGoodDay->date.year, lastGoodDay->date.month, lastGoodDay->date.day );
        break;
        }
      repBeingPaid = conf->customerCare;
      repLastDay = repBeingPaid->endOfWorkDays;
      repID = repBeingPaid->id;
      thisDay = FindThisDateNextMonth( thisDay, repLastDay, NULL );
      }

    if( thisDay==NULL )
      {
      Event( ".. %s date following %04d-%02d-%02d not found.  Ending this sale.",
             repID, lastGoodDay->date.year, lastGoodDay->date.month, lastGoodDay->date.day );
      break;
      }
    }
  }

/* return 0 for good call, -1 for no more calls by this rep on this day, please */
int SimulateInitialCall( _CONFIG* conf,
                         _SALES_REP* salesRep,
                         _SINGLE_DAY* thisDay,
                         _SINGLE_DAY* repLastDay,
                         _PRODUCT* product )
  {
  int nDaysToNextStage = 0;

  /*
  Event( "SimulateInitialCall( rep=%s, date=%04d-%02d-%02d, product=%s )",
         salesRep->id,
         thisDay->date.year, thisDay->date.month, thisDay->date.day,
         product->id );
  */

  if( product->marketSize>0 /* limited */
      && product->nAvailableOrgs<=0 )
    {
    Event( "Tried to make a sales call for %s but already sold to everyone (market=%d, left=%d)",
            product->id, product->marketSize, product->nAvailableOrgs );
    return -1;
    }

  _ORG *targetOrg = NULL;
  if( product->marketSize>0 )
    {
    targetOrg = FindAvailableTargetOrg( conf, product, thisDay->t );
    if( targetOrg==NULL )
      {
      Event( "%04d-%02d-%02d: Rep %s tried to make a sales call for %s but there are no orgs left to call (market size==%d)",
             thisDay->date.year, thisDay->date.month, thisDay->date.day,
             salesRep->id, product->id, product->marketSize );
      return -2;
      }
    if( targetOrg->number > product->maxOrgNum )
      product->maxOrgNum = targetOrg->number;
    }

  char customerID[100];
  if( targetOrg!=NULL )
    snprintf( customerID, sizeof(customerID)-1, "<%d>", targetOrg->number );
  else
    strcpy( customerID, "<anon>" );

  Event( "%04d-%02d-%02d: simulate initial call for product %s by %s to %s",
         thisDay->date.year, thisDay->date.month, thisDay->date.day,
         product->id, salesRep->id, customerID );

  for( int stageNo = 0; stageNo < product->nSalesStages; ++stageNo )
    {
    /*
    Event( "%04d-%02d-%02d: call during stage %d for product %s by %s to %s",
           thisDay->date.year, thisDay->date.month, thisDay->date.day,
           stageNo, product->id, salesRep->id, customerID );
    */

    _SALES_STAGE* stage = product->stageArray[ stageNo ];

    if( stage->repClasses!=NULL
        && SalesRepInIndicatedClass( stage->repClasses, salesRep )!=0 )
      { /* need to find a new rep */
      _SALES_REP* newRep = RandomRepFromClassList( conf, stage->repClasses, thisDay->t );
      if( newRep!=NULL )
        {
        Event( "%04d-%02d-%02d: moved %s to product %s stage %d:%s, switched rep from %s to %s",
               thisDay->date.year, thisDay->date.month, thisDay->date.day,
               customerID, product->id, stageNo, stage->id, salesRep->id, newRep->id );

        if( thisDay->month )
          ++ (thisDay->month->nTransfers);

        if( salesRep->handoffFee > 0 )
          {
          Event( "Paying %s %.1lf for a successful lead", salesRep->id, salesRep->handoffFee );
          thisDay->fees = NewPayEvent( conf, thisDay->date, salesRep, pt_commission, salesRep->handoffFee, thisDay->fees );
          }

        int y = thisDay->date.year;
        int m = thisDay->date.month;
        int d = thisDay->date.day;
        thisDay = FindSingleDay( &(thisDay->date), newRep->workDays, newRep->nWorkDays );

        if( thisDay==NULL )
          {
          Event( "Switched sales process from %s to %s but they have already departed by %04d-%02d-%02d",
                  salesRep->id, newRep->id, y, m, d );
          return -3;
          }

        repLastDay = newRep->endOfWorkDays;

        while( thisDay < repLastDay )
          if( thisDay->working==0
              || ( ( thisDay->maxCalls>0 ) && ( thisDay->nCalls >= thisDay->maxCalls ) ) )
            ++thisDay;
          else
            break;

        if( thisDay >= repLastDay )
          {
          thisDay = NULL;
          Event( "Rep %s has left on %04d-%02d-%02d (by %04d-%02d-%02d - %d days ago) - dropping the sales process",
                 salesRep->id,
                 (repLastDay-1)->date.year,
                 (repLastDay-1)->date.month,
                 (repLastDay-1)->date.day,
                 y, m, d,
                 repLastDay - thisDay
                 );
          return 0;
          }

        Event( "%04d-%02d-%02d: Newly assigned rep %s will start selling %s to %s",
               thisDay->date.year, thisDay->date.month, thisDay->date.day,
               newRep->id, product->id, customerID );

        salesRep = newRep;

        Event( "%s will try to connect with customer starting on %04d-%02d-%02d",
               newRep->id, thisDay->date.year, thisDay->date.month, thisDay->date.day );
        }
      else
        {
        Warning( "Sales stage %s requires a rep in a different class than %s (%s) - nobody found.",
                 stage->id, salesRep->id, salesRep->class->id );
        return 0;
        }
      }

    /* moved this block here (was above) because it's the new rep who should get the counters incremented. */
    if( stageNo>0 && thisDay<repLastDay ) /* if stageNo==0, caller to this function did that already */
      {
      ++ (thisDay->nCalls);
      ++ (thisDay->nFollowUpCalls);
      if( thisDay->month )
        {
        ++ (thisDay->month->nCalls);
        ++ (thisDay->month->nFollowUpCalls);
        }
      Event( "%04d-%02d-%02d: incremented calls by %s for %s to %d (pre connection attempts retries)",
             thisDay->date.year, thisDay->date.month, thisDay->date.day,
             salesRep->id, product->id, thisDay->nCalls
             );
      }

    /* possibly delay moving to the next stage due to a rebooking */
    if( stage->connectAttemptsAverage>0 )
      {
      int nRebookAttempts = (int)(RandN2( stage->connectAttemptsAverage, stage->sdevConnectAttempts ) + 0.5);

      Event( "%04d-%02d-%02d: There will be %d call attempts by %s to %s to complete %s",
             thisDay->date.year, thisDay->date.month, thisDay->date.day,
             nRebookAttempts, salesRep->id, customerID,
             stage->id );

      for( int callNum=0; callNum<nRebookAttempts; ++callNum )
        {
        if( callNum>0 && thisDay<repLastDay ) /* handled above if it's zero */
          {
          Event( "%04d-%02d-%02d: rep %s making rebooking call attempt to org %s",
                 thisDay->date.year, thisDay->date.month, thisDay->date.day,
                 salesRep->id, customerID );

          ++ (thisDay->nCalls); /* make a call */
          ++ (thisDay->nFollowUpCalls);
          if( thisDay->month )
            {
            ++ (thisDay->month->nCalls); /* update the monthly summary data */
            ++ (thisDay->month->nFollowUpCalls);
            }

          Event( "%04d-%02d-%02d: incremented calls by %s for %s to %d (retry %d)",
                 thisDay->date.year, thisDay->date.month, thisDay->date.day,
                 salesRep->id, product->id, thisDay->nCalls, callNum
                 );
          }

        int nDaysToNextCall = (int)(RandN2( stage->connectRetryDaysAverage, stage->sdevConnectRetryDays ) + 0.5);

        Event( "Next call attempt after %04d-%02d-%02d in %d days",
               thisDay->date.year, thisDay->date.month, thisDay->date.day,
               nDaysToNextCall );

        thisDay += nDaysToNextCall;
        while( thisDay < repLastDay )
          if( thisDay->working==0
              || ( ( thisDay->maxCalls>0 ) && ( thisDay->nCalls >= thisDay->maxCalls ) ) )
            ++thisDay;
          else
            break;
        }

      if( thisDay >= repLastDay )
        { /* this should never happen.. */
        _SINGLE_DAY* repLastRealDay = repLastDay - 1;
        Event( "Rep %s is gone (on %04d-%02d-%02d) before sales stage %s - sales process terminated.",
               salesRep->id,
               repLastRealDay->date.year, repLastRealDay->date.month, repLastRealDay->date.day,
               stage->id );
        return 0; /* next stage happens after this rep's last day - unlikely to hand off properly */
        }
      }

    /* possibly lose the customer at this stage */
    if( PercentProbabilityEvent( stage->percentAttrition ) )
      {
      Event( "%04d-%02d-%02d: Lost prospect %s at stage %s (%d) by %s (%.1lf attrition).",
             thisDay->date.year, thisDay->date.month, thisDay->date.day,
             customerID, stage->id, stageNo, salesRep->id, stage->percentAttrition );

      if( thisDay->month ) /* update monthly stats */
        ++ (thisDay->month->nRejections);

      if( targetOrg!=NULL ) /* org won't be called for a cooling period */
        RejectedByOrg( conf, product, targetOrg, thisDay->t );

      return 0; /* failed - attrition at this stage of the sales process*/
      }
    else
      Event( "Prospect %s proceeds from stage %s (%d) by %s.", customerID, stage->id, stageNo, salesRep->id );

    /* did we win the customer?  if no more stages and we didn't lose, then yes! */
    if( stage->isTerminal )
      {
      Event( "Customer %s win at stage %s (%d) by %s.", customerID, stage->id, stageNo, salesRep->id );

      if( targetOrg!=NULL )
        SaleToOrg( conf, product, targetOrg, thisDay->t );

      CloseSingleSale( conf,
                       salesRep,
                       targetOrg,
                       thisDay,
                       repLastDay,
                       product,
                       0.0,
                       0 );

      return 0;
      }
    else
      {
      if( stageNo>0 )
        {
        Event( "Stage %d (%s) for product %s not terminal?", stageNo, stage->id, product->id );
        }
      }
  
    if( stageNo == product->nSalesStages-1 )
      Event( "Attempt to proceed to sales stage %d - for product %s but there is no such stage!",
               stageNo+1, product->id );

    /* didn't win, didn't lose - move on to next stage then */
    nDaysToNextStage = (int)RandN2( stage->daysDelayAverage, stage->sdevDaysDelay );
    Event( "Sales stage %s complete by %s on %04d-%02d-%02d.  Next stage in %d+ days",
           stage->id, salesRep->id,
           thisDay->date.year, thisDay->date.month, thisDay->date.day,
           nDaysToNextStage );

    /* increment thisDay to the time of the next sales stage - but
       note that it might fall on a weekend or holiday or vacation,
       or even a day where we're already simulated a full set of calls.. */
    thisDay += nDaysToNextStage;
    while( thisDay < repLastDay ) /* skip weekends and skip days where we already have a full schedule of calls */
      {
      Event( "Testing %s availability on %04d-%02d-%02d (working=%d, nCalls=%d, maxCalls=%d)",
             salesRep->id, thisDay->date.year, thisDay->date.month, thisDay->date.month,
             thisDay->working, thisDay->nCalls, thisDay->maxCalls );
             
      if( thisDay->working==0
          || ( ( thisDay->maxCalls>0 ) && ( thisDay->nCalls >= thisDay->maxCalls ) ) )
        ++thisDay;
      else
        break;
      }
    
    if( thisDay >= repLastDay )
      {
      Event( "Failed to complete sales process with org %s as rep %s quit.",
             customerID, salesRep->id );
      return 0; /* next stage happens after end of simulation */
      }

    /* thisDay now points to the date when the next sales stage
       happens */
    Event( "Next sales stage by %s will start on %04d-%02d-%02d", salesRep->id, thisDay->date.year, thisDay->date.month, thisDay->date.day );
    }

  return 0;
  }

void PayFirstMonthPartialSalary( _CONFIG* conf, _SALES_REP* s )
  {
  _SINGLE_DAY* repLastDay = s->workDays + s->nWorkDays;

  /* salary for first month is an odd case.. */
  int nDaysFirstMonth = 0;
  for( _SINGLE_DAY* testDay = s->workDays;
       testDay<repLastDay && testDay->date.month==s->workDays->date.month;
       ++testDay )
    ++nDaysFirstMonth;
 
  /* estimate pay per day (annual / available work days) */
  double salaryPerDay = s->monthlyPay * 12.0;
  salaryPerDay /= 365.0;
  double salaryFirstMonth = salaryPerDay * (double)nDaysFirstMonth;

  s->workDays->fees = NewPayEvent( conf, s->workDays->date, s, pt_salary, salaryFirstMonth, s->workDays->fees );
  Event( "Pay rep %s initial salary of %.1lf/mo at %04d-%02d-%02d",
         s->id, salaryFirstMonth, s->workDays->date.year, s->workDays->date.month, s->workDays->date.day );
  }

void AdjustSalaryLastMonth( _CONFIG* conf, _SALES_REP* s, _SINGLE_DAY* lastDay, double monthlySalary, int lastMonthWorkDays )
  {
  double salaryPerDay = ( monthlySalary * 12.0 ) / 365.0; /* may have changed due to annual increases */
  double salaryLastMonth = salaryPerDay * (double)(lastMonthWorkDays - 30); /* approximation of 30 days/month is good enough */
  lastDay->fees = NewPayEvent( conf, lastDay->date, s, pt_salary, salaryLastMonth, lastDay->fees );
  Event( "Adjust rep %s salary by %.1lf at %04d-%02d-%02d due to end of employment",
         s->id, salaryLastMonth, lastDay->date.year, lastDay->date.month, lastDay->date.day );
  }

void PaySingleRepSalary( _CONFIG* conf, _SALES_REP* s )
  {
  _SALES_REP_CLASS* class = s->class;

  _SINGLE_DAY* prevDay = NULL;
  _SINGLE_DAY* repDay = s->workDays;
  _SINGLE_DAY* repLastDay = repDay + s->nWorkDays;

  int monthNo = 0;
  int yearNo = 0;
  double salary = s->monthlyPay;

  PayFirstMonthPartialSalary( conf, s );

  int nDaysSinceSalary = 0;
  for( int i=0; (i < s->nWorkDays)
                && (repDay < repLastDay)/*safety*/
                && (repDay->date.year!=0 )/* real safe */; ++i )
    {
    /* did we just start a new month? */
    if( prevDay!=NULL && repDay->date.month != prevDay->date.month )
      {
      ++monthNo;

      /* did we hit a new year? */
      if( monthNo % 12 == 0 )
        {
        ++yearNo;
        double p = class->annualPayIncreasePercent;
        if( p>0 )
          {
          salary *= (100.0 + class->annualPayIncreasePercent)/100.0;
          Event( "Pay rep %s increased salary of %.1lf on %04d-%02d-%02d",
                 s->id, salary, repDay->date.year, repDay->date.month, repDay->date.day );
          }
        }

      repDay->fees = NewPayEvent( conf, repDay->date, s, pt_salary, salary, repDay->fees );
      /*
      Event( "Pay rep %s salary of %.1lf at %04d-%02d-%02d",
             s->id, salary, repDay->date.year, repDay->date.month, repDay->date.day );
      */
      nDaysSinceSalary = 0;
      }
    else
      ++nDaysSinceSalary;

    prevDay = repDay;
    ++repDay;
    }

  if( prevDay==NULL )
    Warning( "Trying to adjust last month salary for %s but no 'last good day'", s->id );
  else
    AdjustSalaryLastMonth( conf, s, prevDay, salary, nDaysSinceSalary );
  }

/* QQQ this function is either buggy or the data structure is not properly initialized. */
enum badDayReasons { bdr_before_start, bdr_after_end, bdr_error };
_SINGLE_DAY* FindRepDay( _SALES_REP* s, time_t theTime, _MMDD* theDay,
                         enum badDayReasons* reason )
  {
  if( s==NULL || theTime==0 || theDay==NULL )
    {
    *reason = bdr_error;
    return NULL;
    }

  time_t tFirst = MMDDToTime( &(s->firstDay) );
  time_t tLast = MMDDToTime( &(s->lastDay) );

  if( tFirst > theTime ) /* rep hasn't started yet */
    {
    *reason = bdr_before_start;
    return NULL;
    }

  if( tLast < theTime )  /* rep is already gone */
    {
    *reason = bdr_after_end;
    return NULL;
    }

  int dayNo = ( theTime - tFirst ) / DAY_IN_SECONDS;
  if( dayNo < 0 || dayNo >= s->nWorkDays )
    {
    /* seems rare - just end of a given rep.  math/rounding?
       Warning( "Calculated dayNo of %d (<0 or >%d) for %s", dayNo, s->nWorkDays, s->id );
    */
    *reason = bdr_error;
    return NULL;
    }

  for( int n = dayNo>1 ? dayNo-1 : dayNo; n < dayNo+2 && n < s->nWorkDays; ++n )
    {
    _SINGLE_DAY* dPtr = s->workDays + n;
    if( dPtr->date.year==theDay->year
        && dPtr->date.month==theDay->month
        && dPtr->date.day==theDay->day )
      return dPtr;
    }

  _SINGLE_DAY* dPtr = s->workDays + dayNo;
  Event( "Calculated dayNo %d (%04d-%02d-%02d) but wanted %04d-%02d-%02d for %s",
         dayNo,
         dPtr->date.year,
         dPtr->date.month,
         dPtr->date.day,
         theDay->year,
         theDay->month,
         theDay->day,
         s->id );

  *reason = bdr_error;
  return NULL;
  }

void SimulateCalls( _CONFIG* conf, int dayNo, time_t tSim )
  {
  if( conf == NULL
      || dayNo < 0
      || dayNo > conf->simulationDurationDays
      || tSim < conf->simulationStart
      || tSim > conf->simulationEnd )
    return;

  _MMDD theDay;
  if( TimeToMMDD( tSim, &theDay )!=0 )
    {
    Warning( "Failed to convert time (%d days from sim start) to MMDD",
             (tSim - conf->simulationStart) / DAY_IN_SECONDS );
    return;
    }

  Event( "Simulating call sequences that start on on %04d-%02d-%02d", theDay.year, theDay.month, theDay.day );

  /* skip stat holidays up front */
  if( FallsOnHoliday( conf->holidays, &theDay )==0 )
    {
    Event( "%04d-%02d-%02d is a holiday - skipping.", theDay.year, theDay.month, theDay.day );
    return;
    }

  /* go through all the reps */
  for( _SALES_REP* s = conf->salesReps; s!=NULL; s=s->next )
    {
    if( s->class==NULL )
      { Event( "Rep %s has no class - skipping", NULLPROTECT( s->id ) ); continue; }
    if( s->class->salaryOnly )
      { Event( "Rep %s in class %s is only salaried - does not make calls - skipping", NULLPROTECT( s->id ), NULLPROTECT( s->class->id ) ); continue; }
    if( ! s->class->initiateCalls )
      { Event( "Rep %s in class %s does not initiate calls - skipping", NULLPROTECT( s->id ), NULLPROTECT( s->class->id ) ); continue; }

    /* has this rep started and not yet left the org on this day? */
    time_t tThis = MMDDToTime( &theDay );
    time_t tFirst = MMDDToTime( &(s->firstDay) );
    time_t tLast = MMDDToTime( &(s->lastDay) );
    if( tThis < tFirst )
      {
      /*
      Event( "Rep %s has not yet started as of %04d-%02d-%02d",
             s->id, theDay.year, theDay.month, theDay.day );
      */
      continue;
      }
    if( tThis > tLast )
      {
      /*
      Event( "Rep %s has already finished by %04d-%02d-%02d",
             s->id, theDay.year, theDay.month, theDay.day );
      */
      continue;
      }

    Event( "Call sequences starting %04d-%02d-%02d by %s",
           theDay.year, theDay.month, theDay.day, s->id );

    enum badDayReasons notFoundReason;
    _SINGLE_DAY* repDay = FindRepDay( s, tSim, &theDay, &notFoundReason );
    if( repDay==NULL )
      {
      if( notFoundReason == bdr_before_start || notFoundReason == bdr_after_end )
        { /* fine */
        Event( "Rep %s not found for date 04d-%02d-%02d - reason %d",
               s->id, theDay.year, theDay.month, theDay.day, (int)notFoundReason );
        }
      else
        {
        Event( "Rep %s -- cannot find %04d-%02d-%02d in their schedule",
               s->id, theDay.year, theDay.month, theDay.day );
        }
      continue;
      }

    if( repDay->working==0 )
      {
      Event( "%s not working on %04d-%02d-%02d.", s->id, repDay->date.year, repDay->date.month, repDay->date.day );
      continue;
      }

    Event( "%s making %d more calls (%d already completed) on %04d-%02d-%02d",
           s->id, repDay->maxCalls - repDay->nCalls, repDay->nCalls, repDay->date.year, repDay->date.month, repDay->date.day );

    /* repDay->nCalls starts >0 due to follow up from previous events */
    while( repDay->nCalls < repDay->maxCalls )
      {
      /* redundant?  misplaced?
      if( s->endOfWorkDays==NULL )
        Event( "Rep %s has NULL end of work days", s->id );
      */

      Event( "%04d-%02d-%02d: %s making call number %d",
             repDay->date.year, repDay->date.month, repDay->date.day,
             s->id, repDay->nCalls + 1 );
 
      /* if selling multiple products, cycle through them */
      _PRODUCT* product = s->class->products[s->productNum];

      int couldFindOrgToCall = SimulateInitialCall( conf, s, repDay, s->endOfWorkDays, product );
      if( couldFindOrgToCall ) /* above function returns non-zero if it couldn't find a free org to call into */
        Event( "%04d-%02d-%02d: %s initial call returned error %d - try another product?",
               repDay->date.year, repDay->date.month, repDay->date.day,
               s->id, couldFindOrgToCall );

      ++ (repDay->nCalls);
      ++ (repDay->nFreshCalls);

      if( repDay->month )
        {
        ++ (repDay->month->nCalls);
        ++ (repDay->month->nFreshCalls);
        }

      int prevProductNumber = s->productNum;
      s->productNum = (s->productNum+1) % s->class->nProducts;

      if( couldFindOrgToCall && ( s->productNum == prevProductNumber ) )
        { /* market saturated and no more products */
        break;
        }
      }
    }
  }
