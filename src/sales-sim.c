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
                      double overrideRevenue )
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
  /* note that targetOrg is permitted to be NULL */

  Event( "CloseSingleSale( rep=%s, firstDay=%04d-%02d-%02d, lastDay(-1)=%04d-%02d-%02d",
          salesRep->id,
          repFirstDay->date.year, repFirstDay->date.month, repFirstDay->date.day,
          (repLastDay-1)->date.year, (repLastDay-1)->date.month, (repLastDay-1)->date.day );

  _SINGLE_DAY* thisDay = repFirstDay;

  ++ (conf->nCustomerWins);

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

  int initialUnits = 0;
  int finalUnits = 0;
  double unitOnboardingFee = 0.0;
  double unitMonthlyFee = 0.0;

  if( overrideRevenue>0 )
    {
    if( product->priceByUnits )
      Error( "Cannot set initial revenue on product sold by units %s", product->id );

    finalRevenue = overrideRevenue;
    monthsToSteadyState = 0;
    monthlyGrowthRate = 0;
    initialRevenue = overrideRevenue; 
    }
  else
    {
    if( product->priceByUnits )
      {
      finalUnits = (int)(round( RandN2( product->averageCustomerSizeUnits, product->sdevCustomerSizeUnits ) ));
      initialUnits = (int)(round( finalUnits / pow( monthlyGrowthRate, (double)monthsToSteadyState ) ));
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
    initialUnits = 0;
    productFixedMonthlyUnitsGrowth = (int)round( RandN2( product->averageMonthlyGrowthRateUnits,
                                                         product->sdevMonthlyGrowthRateUnits ) );
    }

  Event( "Win at customer %d", customer );
  if( product->priceByUnits )
    {
    Event( ".. unitOnboardingFee = %.1lf", unitOnboardingFee );
    Event( ".. unitMonthlyFee = %.1lf", unitMonthlyFee );
    if( initialUnits>0 )
      Event( ".. initialUnits = %d", initialUnits );
    if( finalUnits>0 )
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
      Event( "Customer %d lost at month %04d-%02d (%d)", customer, thisDay->date.year, thisDay->date.month, monthNo );

      if( thisDay->month )
        {
        ++ (thisDay->month->nLosses );
        }

      if( targetOrg != NULL )
        OrgAttrition( conf, targetOrg, thisDay->t );

      break; /* lost the customer */
      }

    /* stats - # of customer-months in sim */
    ++ monthNo;
    ++ (conf->nCustomerMonths);
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
        Event( ".. %04d-%02d-%02d %s sold %d additional units at %.1lf onboarding fee to org %d",
               thisDay->date.year, thisDay->date.month, thisDay->date.day,
               repID, unitsToAdd, unitOnboardingFee, customer );
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

  // Notice( "SimulateInitialCall( rep=%s, thisDay=%d )", salesRep->id, thisDay - salesRep->workDays );

  if( conf->marketSize>0 /* unlimited */
      && conf->nAvailableOrgs<=0 )
    {
    Event( "Tried to make a sales call but already sold to everyone (market=%d, left=%d)", conf->marketSize, conf->nAvailableOrgs );
    return -1;
    }

  _ORG *targetOrg = NULL;
  if( conf->marketSize>0 )
    {
    targetOrg = FindAvailableTargetOrg( conf, thisDay->t );
    if( targetOrg==NULL )
      {
      Event( "Rep %s tried to make a sales call on %04d-%02d-%02d but there is nobody left to call",
             salesRep->id, thisDay->date.year, thisDay->date.month, thisDay->date.day );
      return -1;
      }
    }

  char customerID[100];
  if( targetOrg==NULL )
    strcpy( customerID, "anon" );
  else
    snprintf( customerID, sizeof(customerID)-1, "<%d>", targetOrg->number );

  Event( "Simulate initial call by %s to %s on %04d-%02d-%02d for product %s",
         salesRep->id, customerID, thisDay->date.year, thisDay->date.month, thisDay->date.day, product->id );

  for( int stageNo = 0; stageNo < product->nSalesStages; ++stageNo )
    {
    if( stageNo>0 && thisDay<repLastDay ) /* if stageNo==0, caller to this function did that already */
      {
      ++ (thisDay->nCalls);
      if( thisDay->month )
        ++ (thisDay->month->nCalls);
      }

    _SALES_STAGE* stage = product->stageArray[ stageNo ];
    if( stage->repClasses!=NULL && SalesRepInIndicatedClass( stage->repClasses, salesRep )!=0 )
      { /* need to find a new rep */
      _SALES_REP* newRep = RandomRepFromClassList( conf, stage->repClasses, thisDay->t );
      if( newRep!=NULL )
        {
        Event( "Sales stage %s requires a rep in a different class.  Assigning %s",
               stage->id, newRep->id );

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
          return -2;
          }
        else
          {
          Event( "Rep %s will start on this on %04d-%02d-%02d",
                 newRep->id, thisDay->date.year, thisDay->date.month, thisDay->date.day );
          /*
          Event( "Rep %s starts on %04d-%02d-%02d and ends on %04d-%02d-%02d (%d)",
                 newRep->id,
                 newRep->workDays->date.year, newRep->workDays->date.month, newRep->workDays->date.day,
                 (newRep->endOfWorkDays-1)->date.year, (newRep->endOfWorkDays-1)->date.month, (newRep->endOfWorkDays-1)->date.day,
                 newRep->nWorkDays
                 );
          */
          }
        salesRep = newRep;
        thisDay = FindSingleDay( &(thisDay->date), newRep->workDays, newRep->nWorkDays );
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

    /* possibly delay this event due to a rebooking */
    if( stage->connectAttemptsAverage>0 )
      {
      int nRebookAttempts = (int)(RandN2( stage->connectAttemptsAverage, stage->sdevConnectAttempts ) + 0.5);
      Event( "There will be %d call attempts from %s to %s to complete this task", nRebookAttempts, salesRep->id, customerID );
      for( int callNum=0; callNum<nRebookAttempts; ++callNum )
        {
        if( callNum>0 && thisDay<repLastDay ) /* handled above if it's zero */
          {
          ++ (thisDay->nCalls); /* make a call */
          if( thisDay->month )
            ++ (thisDay->month->nCalls); /* update the monthly summary data */
          }
        int nDaysToNextCall = (int)(RandN2( stage->connectRetryDaysAverage, stage->sdevConnectRetryDays ) + 0.5);
        if( thisDay==NULL )
          Warning( "thisDay==NULL" );
        else if( thisDay >= repLastDay )
          /*Warning( "thisDay>=repLastDay" )*/;
        else
          Event( "Next call attempt after %04d-%02d-%02d in %d days",
                 thisDay->date.year, thisDay->date.month, thisDay->date.day, nDaysToNextCall );
        thisDay += nDaysToNextCall;
        while( thisDay < repLastDay )
          if( thisDay->working==0
              || ( ( thisDay->maxCalls>0 ) && ( thisDay->nCalls >= thisDay->maxCalls ) ) )
            ++thisDay;
          else
            break;
        }

      /* too late! */
      if( thisDay >= repLastDay )
        {
        Event( "Rep %s is gone before sales stage %s - sales process terminated.",
               salesRep->id, stage->id );
        return 0; /* next stage happens after this rep's last day - unlikely to hand off properly */
        }
      }

    /* possibly lose the customer at this stage */
    if( PercentProbabilityEvent( stage->percentAttrition ) )
      {
      Event( "Lost prospect %s at stage %s (%d) by %s.", customerID, stage->id, stageNo, salesRep->id );

      if( thisDay->month ) /* update monthly stats */
        ++ (thisDay->month->nRejections);

      if( targetOrg!=NULL ) /* org won't be called for a cooling period */
        RejectedByOrg( conf, targetOrg, thisDay->t );

      return 0; /* failed - attrition at this stage of the sales process*/
      }
    else
      Event( "Prospect %s proceeds from stage %s (%d) by %s.", customerID, stage->id, stageNo, salesRep->id );

    /* did we win the customer?  if no more stages and we didn't lose, then yes! */
    if( stage->isTerminal )
      {
      Event( "Customer %s win at stage %s (%d) by %s.", customerID, stage->id, stageNo, salesRep->id );

      if( targetOrg!=NULL )
        SaleToOrg( conf, targetOrg, thisDay->t );

      CloseSingleSale( conf,
                       salesRep,
                       targetOrg,
                       thisDay,
                       repLastDay,
                       product,
                       0 );

      return 0;
      }
  
    if( stageNo == product->nSalesStages-1 )
      Warning( "Attempt to proceed to sales stage %d - for product %s but there is no such stage!",
               stageNo+1, product->id );

    /* didn't win, didn't lose - move on to next stage then */
    nDaysToNextStage = (int)RandN2( stage->daysDelayAverage, stage->sdevDaysDelay );
    Event( "Sales stage %s complete on %04d-%02d-%02d.  Next stage in %d+ days",
           stage->id,
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
    Event( "Next sales stage will start on %04d-%02d-%02d", thisDay->date.year, thisDay->date.month, thisDay->date.day );
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

_SINGLE_DAY* FindRepDay( _SALES_REP* s, time_t theTime, _MMDD* theDay )
  {
  if( s==NULL || theTime==0 || theDay==NULL )
    return NULL;

  time_t tFirst = MMDDToTime( &(s->firstDay) );
  time_t tLast = MMDDToTime( &(s->lastDay) );

  if( tFirst > theTime ) /* rep hasn't started yet */
    return NULL;
  if( tLast < theTime )  /* rep is already gone */
    return NULL;
  int dayNo = ( theTime - tFirst ) / DAY_IN_SECONDS;
  if( dayNo < 0 || dayNo >= s->nWorkDays )
    {
    /* seems rare - just end of a given rep.  math/rounding?
       Warning( "Calculated dayNo of %d (<0 or >%d) for %s", dayNo, s->nWorkDays, s->id );
    */
    return NULL;
    }

  _SINGLE_DAY* dPtr = s->workDays + dayNo;
  if( dPtr->date.year==theDay->year
      && dPtr->date.month==theDay->month
      && dPtr->date.day==theDay->day )
    return dPtr;

  Warning( "Calculated dayNo %d (%04d-%02d-%02d) but wanted %04d-%02d-%02d for %s",
           dayNo,
           dPtr->date.year,
           dPtr->date.month,
           dPtr->date.day,
           theDay->year,
           theDay->month,
           theDay->day,
           s->id );

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

  /* skip stat holidays up front */
  if( FallsOnHoliday( conf->holidays, &theDay )==0 )
    return;

  /* go through all the reps */
  for( _SALES_REP* s = conf->salesReps; s!=NULL; s=s->next )
    {
    if( s->class==NULL )
      continue;
    if( s->class->salaryOnly )
      continue;
    if( ! s->class->initiateCalls )
      continue;

    _SINGLE_DAY* repDay = FindRepDay( s, tSim, &theDay );
    if( repDay==NULL )
      continue;
    if( repDay->working==0 )
      {
      Event( "%s not working on %04d-%02d-%02d.", s->id, repDay->date.year, repDay->date.month, repDay->date.day );
      continue;
      }

    Event( "%s making %d more calls (%d already completed) on %04d-%02d-%02d",
           s->id, repDay->maxCalls - repDay->nCalls, repDay->nCalls, repDay->date.year, repDay->date.month, repDay->date.day );

    /* repDay->nCalls starts >0 due to follow up from previous events */
    for( ; repDay->nCalls < repDay->maxCalls; ++ (repDay->nCalls) )
      { /* if selling multiple products, cycle through them */
      if( repDay->month )
        ++ (repDay->month->nCalls);
      _PRODUCT* product = s->class->products[s->productNum];
      if( s->endOfWorkDays==NULL )
        Event( "Rep %s has NULL end of work days", s->id );
      else
        Event( "Rep %s has %d work days", s->id, s->endOfWorkDays - s->workDays );
      int err = SimulateInitialCall( conf, s, repDay, s->endOfWorkDays, product );
      if( err ) /* above function returns non-zero if it couldn't find a free org to call into */
        break; /* nobody left to call today */
      s->productNum = (s->productNum+1) % s->class->nProducts;
      }
    }
  }
