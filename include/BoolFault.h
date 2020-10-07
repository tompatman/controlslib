//
// Created by gmorehead on 11/9/16.
//

#pragma once

#include "FaultBase.h"


namespace PBFault {


/////////////////////////////////////////////////////////////////////////////////////////////////
class BoolFault
        : public FaultBase {

public:

    BoolFault(ITick &ticker, const char *fcode, EventLogger &evh, bool &val, bool tripst, bool &en_bit, fault_init_struct init, iHandleFaultResponse *responseHandler)
            : PBFault::FaultBase(ticker, fcode, evh, en_bit, init, responseHandler)
              , value(val)
    {
      tripstate = tripst;
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

      bool triptest = (tripstate == value);
      retval = DetectLevel(triptest, PBFault::trip, mode);

      if (!retval)
        retval = DetectLevel(triptest, PBFault::warning, mode);

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
      snprintf(buf, MAX_INFO, "%s, , %d", buf, tripstate);
    }

    virtual void writeSpecializedXmlAtribs(PbPoco::PbXML::tAttribs &attribList)
    {
      attribList.push_back(PbPoco::PbXML::tPair("type", "Bool"));
      attribList.push_back(PbPoco::PbXML::tPair("conditions", string("trip when ") + Poco::NumberFormatter::format(tripstate)));
      attribList.push_back(PbPoco::PbXML::tPair("currVal", Poco::NumberFormatter::format(value)));
      attribList.push_back(PbPoco::PbXML::tPair("value", Poco::NumberFormatter::format(value)));
    }

protected:
    bool tripstate;

    bool &value;

};


}

