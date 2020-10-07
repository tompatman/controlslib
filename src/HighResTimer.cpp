//
// Created by service on 10/7/16.
//

#include <cstring>
#include <time.h>
#include "HighResTimer.h"


PbPoco::Core::HighResTimer::HighResTimer() {
  _bStop = true;
}

PbPoco::Core::HighResTimer::~HighResTimer() {

}

__syscall_slong_t PbPoco::Core::HighResTimer::getElapsedMs()
{

  __syscall_slong_t elapsedTime = 0;

  if (_bStop)
  {
    return (_elapsedMs);
  }
  else
  {
    clock_gettime(CLOCK_REALTIME, &_requestEnd);

    if ((_requestEnd.tv_sec - _requestStart.tv_sec) >= 1)
    {
      _elapsedTime.tv_sec = ((_requestEnd.tv_sec - _requestStart.tv_sec) - 1);
      _elapsedTime.tv_nsec = (1000000000 - _requestStart.tv_nsec) + (_requestEnd.tv_nsec);
    }
    else
    {
      /**
       * time elapsed within a 1 sec. barrier
       */
      _elapsedTime.tv_sec = 0;
      _elapsedTime.tv_nsec = _requestEnd.tv_nsec - _requestStart.tv_nsec;
    }

    _elapsedMs = (_elapsedTime.tv_nsec / 1000000) + (_elapsedTime.tv_sec * 1000);
  }

  return (_elapsedMs);
}

void PbPoco::Core::HighResTimer::reset() {
  _bStop = true;
  memset(&_requestStart, '\0', sizeof(_requestStart));
  memset(&_requestEnd, '\0', sizeof(_requestEnd));
}

void PbPoco::Core::HighResTimer::start() {
  _bStop = false;
  clock_gettime(CLOCK_REALTIME, &_requestStart);

}

struct timespec PbPoco::Core::HighResTimer::getElapsedTime() {
  //update structs by calling get elapsedMS
  getElapsedMs();
  return (_elapsedTime);
}

void PbPoco::Core::HighResTimer::stop() {

  _elapsedMs = getElapsedMs();
  _bStop = true;

}


