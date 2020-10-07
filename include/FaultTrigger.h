//
// Created by gmorehead on 11/9/16.
//

#pragma once

#include <core/ITick.h>

namespace PBFault {


////////////////////////////////////////////////////////////////////////////////////////////////////
class FaultTrigger {
public:

    FaultTrigger(ITick &ticker, Poco::Int64 & delay_ms);

    ~FaultTrigger();

    bool Step(bool test);

    void Reset(void);

    int get_trigticks()
    {
      return trigger_mSeconds;
    }

private:

    ITick &_ticker;

    int accumlated_mSeconds;
    bool lasttest;
    Poco::Int64 &trigger_mSeconds;

    Poco::Int64 last_step_ts;

    bool process_reset;
};


}
