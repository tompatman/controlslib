//
// Created by gmorehead on 11/9/16.
//

#pragma once

#include "FaultBase.h"


namespace PBFault {


/////////////////////////////////////////////////////////////////////////////////////////////////
class BoolWarn
        : public FaultBase {

public:
    BoolWarn(ITick &ticker, const char *fcode, EventLogger &evh, bool &val, bool warn_state,
             bool &en_bit, fault_init_struct init, iHandleFaultResponse *responseHandler, bool auto_clear = false)
            : PBFault::FaultBase(ticker, fcode, evh, en_bit, init, responseHandler)
              , value(val)
              , _auto_clear(auto_clear)
    {
      warnstate = warn_state;
    }

    virtual bool Detect(fault_op_modes mode)
    {
      bool retval = false;

      if (!enable)
      {
        ResetTriggers();
        return retval;
      }

      newflt = false;  // flag only set for one detect cycle, then goes away

      bool warntest = (warnstate == value);
      retval = DetectLevel(warntest, PBFault::warning, mode);   // Only detecting warnings!

      if (_auto_clear && !retval && !warntest && level == PBFault::warning)   // Auto clear warning states.
      {
        ClearFault(PBFault::warning);
      }


      return (retval);
    }

    void GenCode(char *buf)
    {
      char codebuf[MAX_CODE];
      FaultBase::GenCode(codebuf);
      snprintf(buf, MAX_CODE, "%s{%d}", codebuf, value); //add the value to the end of the code
    }

    void sprintinfo(char *buf)
    {
      FaultBase::sprintinfo(buf, "int");
      snprintf(buf, MAX_INFO, "%s, , %d", buf, warnstate);
    }

    virtual void writeSpecializedXmlAtribs(PbPoco::PbXML::tAttribs &attribList)
    {
      attribList.push_back(PbPoco::PbXML::tPair("type", "Bool"));
      attribList.push_back(PbPoco::PbXML::tPair("conditions", string("warn when ") + Poco::NumberFormatter::format(warnstate)));
      attribList.push_back(PbPoco::PbXML::tPair("currVal", Poco::NumberFormatter::format(value)));
      attribList.push_back(PbPoco::PbXML::tPair("value", Poco::NumberFormatter::format(value)));
    }

protected:
    bool warnstate;

    bool &value;

    const bool _auto_clear;

};


}

