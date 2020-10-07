/*
 * RealTick.h
 *
 *  Created on: Aug 19, 2013
 *      Author: service
 */

#pragma once

#include "Poco/Types.h"
#include "ITick.h"
#include "Poco/AtomicCounter.h"
#include "core/ppcLogger.h"
#include "ppcRunnable.h"

using Poco::AtomicCounter;


class RealTick: public ITick, public ppcRunnable {
public:
    RealTick(int mSec, bool realtime, bool bAllowMixedSimMode = false);

		RealTick(int mSec, bool realtime, zmq::context_t &context, Poco::UInt16 watchdogCmd_pushpull_port, string local_ip_bind);

		virtual ~RealTick();

    Poco::Int64 get_num_mSeconds();

    bool starting();

    void process();

    void exiting();

    Poco::Int64 secondsSince(const Poco::Int64 start_tm);

    Poco::Int64 mSecondsSince(const Poco::Int64 start_tm);

    Poco::Int64 TimeDiff_msec(const Poco::Int64 stop_tm, const Poco::Int64 start_tm);

    Poco::Int64 mSecondsSinceDateTime_lt(int year, int month, int dayofmonth, int hr = 0, int min = 0, int sec = 0); // Local time

    int daysSince1970_lt(Poco::Int64 ms_since_epoch_lt);          // Local time

    int daysSince1970_lt();          // Local time

    Poco::Int64 mSecondsSincefirstOfTheDay_lt();

    void increment(Poco::UInt16 uiMSec);

    void increment_seconds(Poco::UInt16 uiSec);

private:

    static Poco::Int64 _num_ticks;

    static int _period;         // mSeconds
    static int _rate;           // ticks per second.

    static Poco::Int64 _gmt_mS_offset;

    ppcLogger * _log;

    bool _bAllowMixedSimMode = false;

		void init();
};

