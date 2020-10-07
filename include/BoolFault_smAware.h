//
// Created by gmorehead on 8/29/18.
//


#pragma once

#include <core/StateMach.h>
#include "BoolFault.h"

namespace PBFault {

template<class states>
class BoolFault_smAware
        : public BoolFault {

public:

    BoolFault_smAware(StateMach<states> &state_machine, const char *fcode, EventLogger &evh, bool &val, bool tripst, bool &en_bit,
                      fault_init_struct init, iHandleFaultResponse *responseHandler, int num_disabled_states, ...)
            : BoolFault(state_machine.getTicker(), fcode, evh, val, tripst, en_bit, init, responseHandler)
              , _sm(state_machine)
    {
      tripstate = tripst;

      va_list arg_list;
      va_start(arg_list, num_disabled_states);

      for (int i = 0; i < num_disabled_states; ++i)
      {
        int state = va_arg(arg_list, int);

        if (state < 0 || state >= _sm.get_numStates())
        {
          throw Poco::InvalidArgumentException(string(fcode) + " - Invalid state item.  Arg:" + std::to_string(i) + "  state:" + std::to_string(state));
        }

        _disabled_states_list.emplace_back((states) state);
      }
      va_end(arg_list);
    }

    virtual bool Detect(fault_op_modes mode)
    {
      for (auto dis_state : _disabled_states_list)
        if (dis_state == _sm.getCurrState())
        {
          ResetTriggers();
          return false;
        }

      return BoolFault::Detect(mode);
    }

private:

    StateMach<states> &_sm;

    std::vector<states> _disabled_states_list;

};

}
