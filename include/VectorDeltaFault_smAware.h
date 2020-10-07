//
// Created by gmorehead on 8/29/18.
//


#pragma once

#include <core/StateMach.h>
#include "VectorDeltaFault.h"

namespace PBFault {

template<class states>
class VectorDeltaFault_smAware
        : public VectorDeltaFault {

public:

    VectorDeltaFault_smAware(StateMach<states> &state_machine, const char *fcode, EventLogger &evh,
                             vector<double *> val_vector, bool &en_bit, double &maxw, double &maxt,
                             fault_init_struct init, iHandleFaultResponse *responseHandler, int num_disabled_states, ...)

            : VectorDeltaFault(state_machine.getTicker(), fcode, evh, val_vector, en_bit, maxw, maxt, init, responseHandler)
              , _sm(state_machine)
    {
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
      for (auto disabled_state : _disabled_states_list)
        if (disabled_state == _sm.getCurrState())
        {
          ResetTriggers();
          return false;
        }

      return VectorDeltaFault::Detect(mode);
    }

private:

    StateMach<states> &_sm;

    std::vector<states> _disabled_states_list;

};

}
