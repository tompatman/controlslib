//
// Created by gmorehead on 11/9/16.
//

#pragma once

#include <core/StateMach.h>
#include "fault/FaultBase.h"


namespace PBFault {

/////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * Use this fault for states with a short transitional time.
 *
 * The fault can be set to both warn and fault based on the delay times.
 *
 *   Good examples of usage are:
 *        - "CmdTransit" very short less then 5sec
 *        - "GoingFloat" somewhat short, on the order of 30sec
 *
 */
template<class states>
class StateTransitionTimeoutFault
        : public PBFault::FaultBase {

public:
    StateTransitionTimeoutFault(StateMach<states> &state_machine, const char *fcode, EventLogger &evh,
                                states fault_state, bool &en_bit, fault_init_struct init, iHandleFaultResponse *responseHandler)

            : PBFault::FaultBase(state_machine.getTicker(), fcode, evh, en_bit, init, responseHandler)
              , _sm(state_machine)
              , _fault_state(fault_state)
    {
    }

    virtual bool Detect(fault_op_modes mode)
    {
      bool retval = false;

      if (!enable)
      {
        ResetTriggers();
        return retval;
      }

      bool triptest = _sm.getCurrState() == _fault_state;

      newflt = false;  // flag only set for one detect cycle, then goes away

      retval = DetectLevel(triptest, PBFault::trip, mode);

      if (!retval)
        retval = DetectLevel(triptest, PBFault::warning, mode);

      return (retval);
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
      snprintf(buf, MAX_INFO, "%s, , == %d", buf, _fault_state);
    }

    virtual void writeSpecializedXmlAtribs(PbPoco::PbXML::tAttribs &attribList)
    {
      attribList.push_back(PbPoco::PbXML::tPair("type", "StateTransTimeout"));
      attribList.push_back(PbPoco::PbXML::tPair("conditions", string("When state is " + _sm.getStateTextStr(_fault_state))));
      attribList.push_back(PbPoco::PbXML::tPair("currVal", _sm.getStateTextStr(_sm.getCurrState())));
    }

protected:

    states _fault_state;

    StateMach<states> &_sm;

};


}
