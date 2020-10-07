/*
 * ITick.h
 *
 *  Created on: Aug 16, 2013
 *      Author: service
 */

#pragma once

#include "Poco/Types.h"
#include "Poco/Timestamp.h"
#include "Poco/Exception.h"
#include "ppcLogger.h"

using Poco::AtomicCounter;

class ITick {
public:
    virtual ~ITick() {};
    //virtual void run();

    //virtual Poco::Int64 getCurrentTime() = 0;
#if 0
    {
        return get_num_mSeconds();
    };
#endif

    //These were static
    virtual Poco::Int64 get_num_mSeconds() = 0;

    //virtual bool created() = 0;

    //virtual bool started() = 0;

    virtual Poco::Int64 secondsSince(const Poco::Int64 start_tm) = 0;

    virtual Poco::Int64 mSecondsSince(const Poco::Int64 start_tm) = 0;

    virtual Poco::Int64 TimeDiff_msec(const Poco::Int64 stop_tm, const Poco::Int64 start_tm) = 0;

    virtual Poco::Int64 mSecondsSinceDateTime_lt(int year, int month, int dayofmonth, int hr = 0, int min = 0, int sec = 0) = 0; // Local time

    virtual int daysSince1970_lt(Poco::Int64 ms_since_epoch_lt) = 0;          // Local time

    virtual int daysSince1970_lt() = 0;          // Local time

    virtual Poco::Int64 mSecondsSincefirstOfTheDay_lt() = 0;

    static ITick &getTicker()
    {
      if (theTicker == NULL)
      {
        ppclog(LG_ERR, "No ticker has been created!");
        throw Poco::InvalidAccessException("No ticker has been created!");
      }

      return *theTicker;
    }


    static Poco::Int64 mSecondsSince1970()
    {
      return (Poco::Timestamp().epochMicroseconds() / 1000);
    }

    static Poco::Int64 mSecondsSince1970_lt()   // Local time
    {
      time_t timer = time(NULL);
      const tm *tmptr = localtime(&timer);
      Poco::Int64 gmt_mS_offset = 1000 * tmptr->tm_gmtoff;   // Convert local time offset to mSeconds
      return (mSecondsSince1970() + gmt_mS_offset);
    }

    virtual void increment(Poco::UInt16 uiMSec) = 0;

	virtual void increment_seconds(Poco::UInt16 uiSec) = 0;

protected:
    static AtomicCounter created;
    static AtomicCounter started;
    static ITick *theTicker;
};

