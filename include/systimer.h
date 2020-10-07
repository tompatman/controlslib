/******************************************************************************
 * @file systimer.h
 * @brief Generic system timer object to control time based states.
 *      Used in conjunction with an implementation of Ticker.cpp
 *      to measure time.
 *
 * Jan 18, 2011
 * @author TAlexander
 *
 ******************************************************************************/

#pragma once

/*************************************Include*********************************/

#include "ITick.h"
#include "Poco/Mutex.h"

/*************************************Include*********************************/

/*************************************Class***********************************/
namespace PbPoco {
namespace Core {

class SysTimer
{
public:

    Poco::Int64 i64TimeElapsed;/**<The current time which is updated by CheckTimer().*/
    Poco::Int64 i64dT; /**<time delta since last check.*/

    /******************************************************************************
     *  SysTimer( const int iTimeoutMS )
     *  Default Constructor.
     *
     *  @param[iTimeoutMS]  Timeout, in milliseconds.
     ******************************************************************************/

    SysTimer(const int iTimeoutMS);

    ~SysTimer();

    /******************************************************************************
     *  CheckTimer
     *  Check to see if the system timer has expired.
     *
     *  @return true if the timer has expired.
     ******************************************************************************/

    bool
    CheckTimer();

    /******************************************************************************
     *  bIsTimerRunning()
     *  Returns true if the system timer is running.
     *
     *  @return true if the system timer is running.
     ******************************************************************************/

    bool
    bIsTimerRunning();

    /******************************************************************************
     *  ResetTimer()
     *  Stop and reset the system timer to 0. StartTimer must be called to
     *  start the timer.
     *
     ******************************************************************************/

    void
    ResetTimer();

    /******************************************************************************
     *  ResetTimer( const int iTimeoutMS )
     *  Stop and reset the system timer to 0. Set a new timer interval.
     *
     *  @param[iTimeoutMS]  Timeout, in milliseconds.
     ******************************************************************************/

    void
    ResetTimer( const int iTimeoutMS );

    /******************************************************************************
     *  StartTimer()
     *  Start the system timer.
     *
     ******************************************************************************/

    void
    StartTimer();

    /******************************************************************************
     *  StopTimer()
     *  Stop the system timer.
     *
     ******************************************************************************/

    void
    StopTimer();

private:

    bool bEnable;/**<True when the timer is enabled.*/
    bool bTimedout;/**<True when the timer has timed out.*/
    int m_iTimeoutMS;/**<Timer timeout, in milliseconds.*/
    Poco::Int64 i64TimeStart;/**<The timer start time.*/
    ITick &pTicker;/**<Ticker created if one does not exist.*/

    Poco::Mutex _mutex;
};

}
}

/*************************************Class***********************************/

