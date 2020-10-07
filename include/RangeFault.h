//
// Created by gmorehead on 11/9/16.
//

#pragma once

#include "FaultBase.h"
#include "FaultContainer.h"


namespace PBFault {


////////////////////////////////////////////////////////////////////////////////////////////////////
template<class rtype>
class RangeFault
        : public FaultBase {

public:

    RangeFault(ITick &ticker, const char *fcode, IEventLogger &evh, rtype &val,
               rtype &mint, rtype &minw, rtype &maxw, rtype &maxt, bool &en_bit, fault_init_struct init, iHandleFaultResponse *responseHandler)
            : PBFault::FaultBase(ticker, fcode, evh, en_bit, init, responseHandler)
              , mintrip(mint), minwarn(minw), maxwarn(maxw), maxtrip(maxt), value(val)
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

      newflt = false;  // flag only set for one detect cycle, then goes away

      bool triptest = ((value <= mintrip) || (value >= maxtrip));
      retval = DetectLevel(triptest, PBFault::trip, mode);

      bool warntest = ((value <= minwarn) || (value >= maxwarn));
      if (!retval)
        retval = DetectLevel(warntest, PBFault::warning, mode);

      return (retval);
    }

    virtual string createEventText() override
    {
      string etxt = "value " + NumberFormatter::format(value, 2);

      if (level == PBFault::warning)
      {
        if (value >= maxwarn)
          etxt += " >= " + Poco::NumberFormatter::format(maxwarn, display_precision);
        else
          etxt += " <= " + Poco::NumberFormatter::format(minwarn, display_precision);
      }
      else if (level >= PBFault::trip)
      {
        if (value >= maxwarn)
          etxt += " >= " + Poco::NumberFormatter::format(maxtrip, display_precision);
        else
          etxt += " <= " + Poco::NumberFormatter::format(mintrip, display_precision);
      }

      return etxt;
    }

    void sprintinfo(char *buf)
    {
      if (std::is_same<rtype, double>::value)
      {
        FaultBase::sprintinfo(buf, "double");
        snprintf(buf, MAX_INFO, "%s, , %.3f, %.3f, %.3f, %.3f", buf, (double) mintrip, (double) minwarn, (double) maxwarn, (double) maxtrip);
      }
      else if (std::is_same<rtype, float>::value)
      {
        FaultBase::sprintinfo(buf, "float");
        snprintf(buf, MAX_INFO, "%s, , %.1f, %.1f, %.1f, %.1f", buf, (float) mintrip, (float) minwarn, (float) maxwarn, (float) maxtrip);
      }
      else if (std::is_same<rtype, int>::value)
      {
        FaultBase::sprintinfo(buf, "int");
        snprintf(buf, MAX_INFO, "%s, , %d, %d, %d, %d", buf, (int) mintrip, (int) minwarn, (int) maxwarn, (int) maxtrip);
      }
    }

    void GenCode(char *buf)
    {
      char codebuf[MAX_CODE];
      FaultBase::GenCode(codebuf);

      if (std::is_same<rtype, double>::value)
      {
        snprintf(buf, MAX_CODE, "%s{%.3f}", codebuf, (double) value); //add the value to the end of the code
      }
      else if (std::is_same<rtype, float>::value)
      {
        snprintf(buf, MAX_CODE, "%s{%.1f}", codebuf, (float) value); //add the value to the end of the code
      }
      else if (std::is_same<rtype, int>::value)
      {
        snprintf(buf, MAX_CODE, "%s{%d}", codebuf, (int) value); //add the value to the end of the code
      }
    }

    virtual void writeSpecializedXmlAtribs(PbPoco::PbXML::tAttribs &attribList)
    {
      if (std::is_same<rtype, double>::value)
      {
        attribList.push_back(PbPoco::PbXML::tPair("type", "RangeFault<double>"));
      }
      else if (std::is_same<rtype, float>::value)
      {
        attribList.push_back(PbPoco::PbXML::tPair("type", "RangeFault<float>"));
      }
      else if (std::is_same<rtype, int>::value)
      {
        attribList.push_back(PbPoco::PbXML::tPair("type", "RangeFault<int>"));
      }

      string cond_str = "";

      if (mintrip != -INFINITY) cond_str += string("Trip:") + Poco::NumberFormatter::format(mintrip, display_precision) + " > ";
      if (minwarn != -INFINITY) cond_str += string("Warn:") + Poco::NumberFormatter::format(minwarn, display_precision) + " > ";
      cond_str += "value ";
      if (maxwarn != INFINITY) cond_str += string("> Warn:") + Poco::NumberFormatter::format(maxwarn, display_precision) + " ";
      if (maxtrip != INFINITY) cond_str += string("> Trip:") + Poco::NumberFormatter::format(maxtrip, display_precision);
      attribList.push_back(PbPoco::PbXML::tPair("conditions", cond_str));

      attribList.push_back(PbPoco::PbXML::tPair("currVal", Poco::NumberFormatter::format(value, display_precision)));
    }

protected:
    rtype &mintrip, &minwarn, &maxwarn, &maxtrip;

    rtype &value;
};


}

