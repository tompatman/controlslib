//
// Created by gmorehead on 11/9/16.
//

#include "FaultTrigger.h"


namespace PBFault {



//****************************************************

FaultTrigger::FaultTrigger(ITick &ticker, Poco::Int64 & delay_ms)
        : _ticker(ticker)
, trigger_mSeconds(delay_ms)
{
  accumlated_mSeconds = 0;
  lasttest = false;
  last_step_ts = _ticker.get_num_mSeconds();
  process_reset = true;
}

FaultTrigger::~FaultTrigger()
{
};


bool FaultTrigger::Step(bool test)
{
  int delta_ms = (int) (_ticker.get_num_mSeconds() - last_step_ts);

  if (process_reset)
  {
    Reset();    // Step can be done along time after actual reset.  Need to reset before read detection begins
    process_reset = false;
    delta_ms = 0;
  }

  last_step_ts = _ticker.get_num_mSeconds();

  if (test)
    accumlated_mSeconds += delta_ms;
  else
    accumlated_mSeconds -= delta_ms;

  if (accumlated_mSeconds > trigger_mSeconds)
  {
    accumlated_mSeconds = trigger_mSeconds;
    lasttest = true;
  }
  else if (accumlated_mSeconds < 0)
  {
    accumlated_mSeconds = 0;
    lasttest = false;
  }

  return lasttest;
}

void FaultTrigger::Reset(void)
{
  process_reset = true;
  last_step_ts = _ticker.get_num_mSeconds();
  accumlated_mSeconds = 0;
  lasttest = false;
}


}
