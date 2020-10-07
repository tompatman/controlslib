/******************************************************************************
 * @file systimer.cc
 *
 * @brief Generic system timer object to control time based states.
 *      Used in conjunction with an implementation of Ticker.cpp
 *      to measure time.
 *
 * Jan 19, 2011
 * @author TAlexander
 *
 ******************************************************************************/

/*************************************Include*********************************/

//#include "ppsignals.h"
#include "core/systimer.h"

/*************************************Include*********************************/

/********************************Public Methods*******************************/

namespace PbPoco {
namespace Core {

/******************************************************************************
 *  SysTimer( const int iTimeoutMS )
 *  Default Constructor.
 *
 *  @param[iTimeoutMS]  Description of first function argument.
 ******************************************************************************/

SysTimer::SysTimer(const int iTimeoutMS)
        :
        pTicker(ITick::getTicker())
{
  //pTicker = &ITick::getTicker();    // This will throw an exception if the object does not exist.

  //Initialize class members
  m_iTimeoutMS = iTimeoutMS;
  i64TimeStart = 0;
  i64TimeElapsed = 0;
  bEnable = false;
  bTimedout = false;
  i64dT = 0;
}

SysTimer::~SysTimer()
{
}

/******************************************************************************
 *  CheckTimer
 *  Check to see if the system timer has expired.
 *
 *  @return true if the timer has expired.
 ******************************************************************************/

bool
SysTimer::CheckTimer()
{
  _mutex.lock();

  //The timer must be enabled, calling this without enabled timer is invalid
  if (!bEnable)
  {
    _mutex.unlock();
    return (false);
  }

  i64dT = (pTicker.mSecondsSince(i64TimeStart) - i64TimeElapsed);
  //Update the timer
  i64TimeElapsed += i64dT;

  //Update bTimedout
  if (i64TimeElapsed >= m_iTimeoutMS)
  {
    bTimedout = true;
  }

  _mutex.unlock();
  return (bTimedout);
}

/******************************************************************************
 *  bIsTimerRunning()
 *  Returns true if the system timer is running.
 *
 *  @return true if the system timer is running.
 ******************************************************************************/

bool
SysTimer::bIsTimerRunning()
{
  return (bEnable);
}

/******************************************************************************
 *  ResetTimer()
 *  Stop and reset the system timer to 0. StartTimer must be called to
 *  start the timer.
 *
 ******************************************************************************/

void
SysTimer::ResetTimer()
{
  _mutex.lock();

  i64TimeStart = 0;
  i64TimeElapsed = 0;
  bEnable = false;
  bTimedout = false;

  _mutex.unlock();
}


void
SysTimer::ResetTimer(const int iTimeoutMS)
{
  _mutex.lock();

  SysTimer::ResetTimer();
  m_iTimeoutMS = iTimeoutMS;

  _mutex.unlock();
}

/******************************************************************************
 *  StartTimer()
 *  Start the system timer.
 *
 ******************************************************************************/

void
SysTimer::StartTimer()
{
  _mutex.lock();

  //Get the current time
  ResetTimer();
  i64TimeStart = pTicker.get_num_mSeconds();
  bEnable = true;

  _mutex.unlock();

}

/******************************************************************************
 *  StopTimer()
 *  Stop the system timer.
 *
 ******************************************************************************/

void
SysTimer::StopTimer()
{
  _mutex.lock();

  bEnable = false;

  _mutex.unlock();
}

}
}

/********************************Public Methods*******************************/
