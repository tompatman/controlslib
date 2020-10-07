/*
 * SimTick.cpp
 *
 *  Created on: Aug 19, 2013
 *      Author: service
 */


#include "SimTick.h"



Poco::Int64 SimTick::_num_ticks = 0;

int SimTick::_period = 10;     // 10 mSeconds
int SimTick::_rate = 100;      // 100 ticks per second

Poco::Int64 SimTick::_gmt_mS_offset = 0;


using Poco::Timestamp;

SimTick::SimTick(int mSec)
{
    _period = mSec;
    _rate = 1000 / _period;

    _log = new ppcLogger("SimTick");
    _log->setLevel(LG_INFO);

  if (created.value() > 0)
  {
    _log->log(LG_ERR, "SimTick::SimTick() called multiple times!");
    throw Poco::InvalidAccessException("Invalid ticker");
  }
    else
  {
    created++;
    theTicker = this;
  }

    _num_ticks = 0;
}

SimTick::~SimTick()
{
  created--;
  theTicker = nullptr;
}

//From ppcRunTImed
#if 0
bool SimTick::starting()
{
    if(_started.value() > 0)
    {
        _log->log(LG_INFO, "SimTick::Start() SimTick thread started multiple times!");
        return false;
    }

    _log->log(LG_INFO, "SimTick::Start() ");
    _started++;
    return true;
}

void SimTick::process()
{
    _num_ticks++;

	_log->log(LG_VERBOSE, "SimTick::process() num_tics = %ld.", _num_ticks);
}
#endif


void SimTick::increment(Poco::UInt16 uiMSec)
{
	_num_ticks += (uiMSec/_period);
}

void SimTick::increment_seconds(Poco::UInt16 uiSec)
{
	_num_ticks += (1000*uiSec/_period);
}

Poco::Int64 SimTick::get_num_mSeconds()
{
    return _num_ticks * _period;
}

Poco::Int64 SimTick::mSecondsSince(const Poco::Int64 start_tm)
{
    return TimeDiff_msec(get_num_mSeconds(), start_tm);
}

Poco::Int64 SimTick::secondsSince(const Poco::Int64 start_tm)
{
    return TimeDiff_msec(get_num_mSeconds(), start_tm) / 1000;
}

Poco::Int64 SimTick::TimeDiff_msec(const Poco::Int64 stop_tm, const Poco::Int64 start_tm)
{
  return stop_tm - start_tm;
}

Poco::Int64 SimTick::mSecondsSince1970()
{
    return (Timestamp().epochMicroseconds() / 1000);
}

Poco::Int64 SimTick::mSecondsSince1970_lt()
{
    time_t timer = time(NULL);
    const tm *tmptr = localtime(&timer);
    _gmt_mS_offset = 1000 * tmptr->tm_gmtoff;   // Convert local time offset to mSeconds
    return (mSecondsSince1970() + _gmt_mS_offset);
}

Poco::Int64 SimTick::mSecondsSinceDateTime_lt(int year, int month, int dayofmonth, int hr, int min, int sec)
{
    tm t;
    memset(&t, 0, sizeof(t));
    t.tm_sec    = sec;
    t.tm_min    = min;
    t.tm_hour   = hr;
    t.tm_mday   = dayofmonth;
    t.tm_mon    = month - 1;
    t.tm_year   = year - 1900;
    ppclog(LG_INFO, " --- REQ_TIME: %s", asctime(&t));
    time_t requested_time = mktime(&t);
    const tm *tmptr = localtime(&requested_time);
    _gmt_mS_offset = 1000 * tmptr->tm_gmtoff;   // Convert local time offset to mSeconds
    memset(&t, 0, sizeof(t));
    t.tm_sec    = 0;
    t.tm_min    = 0;
    t.tm_hour   = 0;
    t.tm_mday   = 1;
    t.tm_mon    = 0;
    t.tm_year   = 70;
    ppclog(LG_INFO, " --- EPOCH_TIME: %s  - gmt_offset:%lld", asctime(&t), _gmt_mS_offset);
    time_t epoch_time = mktime(&t);
    Poco::Int64 msecs_since = (Poco::Int64)(1000.0 * difftime(requested_time, epoch_time));
    return (msecs_since); // + _gmt_mS_offset);
}


int SimTick::daysSince1970_lt()
{
    return ((int) ( mSecondsSince1970_lt() / ((Poco::Int64)1000 * 60 * 60 * 24)));
}

int SimTick::daysSince1970_lt(Poco::Int64 ms_since_epoch_lt)
{
    return ((int) ( ms_since_epoch_lt / ((Poco::Int64)1000 * 60 * 60 * 24)));
}

Poco::Int64 SimTick::mSecondsSincefirstOfTheDay_lt()
{
    return ( (Poco::Int64) daysSince1970_lt() * ((Poco::Int64)1000 * 60 * 60 * 24) );
}

