//
// Created by gmorehead on 11/9/16.
//

#pragma once

#include <core/StateMach.h>
#include "fault/FaultBase.h"


namespace PBFault {

/////////////////////////////////////////////////////////////////////////////////////////////////
/*
 * Use this fault to indicate a "fault" or "warning" entrance into a state.
 *
 * Basically intended to detect the entrance into a state machines "Faulted" state.
 *   - Usually triggers will be set to zero
 *
 *  ** Requires the fault be "re-triggered".  Reduces the tendency for double fault detection.
 */
template<class states>
class StateTriggeredFault
        : public PBFault::FaultBase {

public:
    StateTriggeredFault(StateMach<states> &state_machine, const char *fcode, EventLogger &evh,
                        states trigger_state, bool &en_bit, fault_init_struct init, iHandleFaultResponse *responseHandler, bool trigger_enabled = false)

            : PBFault::FaultBase(state_machine.getTicker(), fcode, evh, en_bit, init, responseHandler)
              , _sm(state_machine)
              , _trigger_state(trigger_state)
              , _trigger_enabled(trigger_enabled)
    {
    }

    virtual bool Detect(fault_op_modes mode)
    {
      bool retval = false;

      if (!enable || !_trigger_enabled)
      {
        ResetTriggers();
        return retval;
      }

      bool triptest = _sm.getCurrState() == _trigger_state;

      newflt = false;  // flag only set for one detect cycle, then goes away

      retval = DetectLevel(triptest, PBFault::trip, mode);

      if (retval)
        _trigger_enabled = false;   // Must be re-triggered

      if (!retval)
        retval = DetectLevel(triptest, PBFault::warning, mode);

      return (retval);
    }

    void enableTrigger()
    {
      _trigger_enabled = true;
    }

    virtual string createEventText() override
    {
      string etxt = _sm.getStatusText();

      return etxt;
    }

    void GenCode(char *buf)
    {
      char codebuf[MAX_CODE];
      FaultBase::GenCode(codebuf);
      snprintf(buf, MAX_CODE, "%s", codebuf); //add the value to the end of the code
    }

    void sprintinfo(char *buf)
    {
      FaultBase::sprintinfo(buf, "int");
      snprintf(buf, MAX_INFO, "%s, , == %d", buf, _trigger_state);
    }

    virtual void writeSpecializedXmlAtribs(PbPoco::PbXML::tAttribs &attribList)
    {
      attribList.push_back(PbPoco::PbXML::tPair("type", "StateTransTimeout"));
      attribList.push_back(PbPoco::PbXML::tPair("conditions", string("When state is " + _sm.getStateTextStr(_trigger_state))));
      attribList.push_back(PbPoco::PbXML::tPair("currVal", _sm.getStateTextStr(_sm.getCurrState())));
    }

protected:

    states _trigger_state;

    StateMach<states> &_sm;

    bool _trigger_enabled;

};


}
