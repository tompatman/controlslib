/*
 * RealTick.cpp
 *
 *  Created on: Aug 19, 2013
 *      Author: service
 */

#include "RealTick.h"


#include "ppcTimer.h"


AtomicCounter ITick::created;
AtomicCounter ITick::started;
ITick *ITick::theTicker = nullptr;

Poco::Int64 RealTick::_num_ticks = 0;

int RealTick::_period = 10;     // 10 mSeconds
int RealTick::_rate = 100;      // 100 ticks per second

Poco::Int64 RealTick::_gmt_mS_offset = 0;

using Poco::Timestamp;


RealTick::RealTick(int mSec, bool realtime, zmq::context_t &context, Poco::UInt16 watchdogCmd_pushpull_port, string local_ip_bind)
        : ppcRunnable("RealTick", mSec, realtime, mSec)
          , _bAllowMixedSimMode(false)
{
    _period = mSec;
    this->enableWatchdogSignal(context, watchdogCmd_pushpull_port, local_ip_bind);
    init();
}

RealTick::RealTick(int mSec, bool realtime, const bool bAllowMixedSimMode)
        : ppcRunnable("RealTick", mSec, realtime, mSec), _bAllowMixedSimMode(bAllowMixedSimMode)
{
    _period = mSec;

    init();
}

RealTick::~RealTick()
{
  ITick::created--;
  ITick::theTicker = nullptr;
    delete _log;
}

bool RealTick::starting()
{
  if (ITick::started.value() > 0)
    {
        _log->log(LG_INFO, "RealTick::Start() RealTick thread started multiple times!");
        return false;
    }

  _log->log(LG_INFO, "RealTick::Start()");
  ITick::started++;
    return true;
}

void RealTick::process()
{
    _num_ticks++;

	_log->log(LG_VERBOSE, "RealTick::process() num_tics = %ld.", _num_ticks);
}

void RealTick::exiting()
{
  if (ITick::started.value() <= 0)
    {
        _log->log(LG_INFO, "RealTick::exiting() RealTick thread started multiple times!");
    }

    _log->log(LG_INFO, "RealTick::exiting()");
  ITick::started--;
}

Poco::Int64 RealTick::get_num_mSeconds()
{
    return _num_ticks * _period;
}

Poco::Int64 RealTick::mSecondsSince(const Poco::Int64 start_tm)
{
    return TimeDiff_msec(get_num_mSeconds(), start_tm);
}

Poco::Int64 RealTick::secondsSince(const Poco::Int64 start_tm)
{
    return TimeDiff_msec(get_num_mSeconds(), start_tm) / 1000;
}

Poco::Int64 RealTick::TimeDiff_msec(const Poco::Int64 stop_tm, const Poco::Int64 start_tm)
{
  return stop_tm - start_tm;
}


Poco::Int64 RealTick::mSecondsSinceDateTime_lt(int year, int month, int dayofmonth, int hr, int min, int sec)
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

void RealTick::increment(Poco::UInt16 uiMSec) {
  if (_bAllowMixedSimMode)
  {
    _num_ticks += (uiMSec / _period);

  }
  else
  {
    throw Poco::NotImplementedException("Can't advance time in real ticker.");
  }
}

void RealTick::increment_seconds(Poco::UInt16 uiSec) {
  if (_bAllowMixedSimMode)
  {
    _num_ticks += (1000 * uiSec / _period);
  }
  else
  {
    throw Poco::NotImplementedException("Can't advance time in real ticker.");
  }

}


int RealTick::daysSince1970_lt()
{
    return ((int) ( mSecondsSince1970_lt() / ((Poco::Int64)1000 * 60 * 60 * 24)));
}

int RealTick::daysSince1970_lt(Poco::Int64 ms_since_epoch_lt)
{
    return ((int) ( ms_since_epoch_lt / ((Poco::Int64)1000 * 60 * 60 * 24)));
}

Poco::Int64 RealTick::mSecondsSincefirstOfTheDay_lt()
{
    return ( (Poco::Int64) daysSince1970_lt() * ((Poco::Int64)1000 * 60 * 60 * 24) );
}

void RealTick::init()
{
    _rate = 1000 / _period;

    _log = new ppcLogger("RealTick");
    _log->setLevel(LG_INFO);

  if (ITick::created.value() > 0)
        _log->log(LG_INFO, "RealTick::RealTick() called multiple times!");
    else
    {
      ITick::created++;
      ITick::theTicker = this;
    }
}





