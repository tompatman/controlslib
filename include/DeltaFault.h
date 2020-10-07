//
// Created by gmorehead on 11/9/16.
//

#pragma once

#include "FaultBase.h"


namespace PBFault {


////////////////////////////////////////////////////////////////////////////////////////////////////
template<class rtype>
class DeltaFault
        : public FaultBase {

public:

    DeltaFault(ITick &ticker, const char *fcode, EventLogger &evh, rtype &val1, rtype &val2, rtype &maxw, rtype &maxt, bool &en_bit, fault_init_struct init, iHandleFaultResponse *responseHandler)
            : PBFault::FaultBase(ticker, fcode, evh, en_bit, init, responseHandler)
              , maxwarn(maxw), maxtrip(maxt), value1(val1), value2(val2)
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

      bool triptest = fabs(value1 - value2) >= maxtrip;
      retval = DetectLevel(triptest, PBFault::trip, mode);

      bool warntest = fabs(value1 - value2) >= maxwarn;
      if (retval == false)
        retval = DetectLevel(warntest, PBFault::warning, mode);

      return (retval);
    }

    virtual string createEventText() override
    {
      string etxt = "range: " + NumberFormatter::format(fabs(value1 - value2), display_precision);

      if (level == PBFault::warning)
      {
        etxt += " >= " + Poco::NumberFormatter::format(maxwarn, 2);
      }
      else if (level >= PBFault::trip)
      {
        etxt += " >= " + Poco::NumberFormatter::format(maxtrip, 2);
      }

      return etxt;
    }

    void sprintinfo(char *buf)
    {
      if (std::is_same<rtype, double>::value)
      {
        FaultBase::sprintinfo(buf, "double");
        snprintf(buf, MAX_INFO, "%s, , %.3f, %.3f", buf, (double) maxwarn, (double) maxtrip);
      }
      else if (std::is_same<rtype, float>::value)
      {
        FaultBase::sprintinfo(buf, "float");
        snprintf(buf, MAX_INFO, "%s, , %.1f, %.1f", buf, (float) maxwarn, (float) maxtrip);
      }
      else if (std::is_same<rtype, int>::value)
      {
        FaultBase::sprintinfo(buf, "int");
        snprintf(buf, MAX_INFO, "%s, , %d, %d", buf, (int) maxwarn, (int) maxtrip);
      }
    }

    void GenCode(char *buf)
    {
      char codebuf[MAX_CODE];
      FaultBase::GenCode(codebuf);

      if (std::is_same<rtype, double>::value)
      {
        snprintf(buf, MAX_CODE, "%s{%.3f, %.3f}", codebuf, (double) value1, (double) value2); //add the value to the end of the code
      }
      else if (std::is_same<rtype, float>::value)
      {
        snprintf(buf, MAX_CODE, "%s{%.1f, %.1f}", codebuf, (float) value1, (float) value2); //add the value to the end of the code
      }
      else if (std::is_same<rtype, int>::value)
      {
        snprintf(buf, MAX_CODE, "%s{%d, %d}", codebuf, (int) value1, (int) value2); //add the value to the end of the code
      }
    }

    virtual void writeSpecializedXmlAtribs(PbPoco::PbXML::tAttribs &attribList)
    {
      if (std::is_same<rtype, double>::value)
      {
        attribList.push_back(PbPoco::PbXML::tPair("type", "DeltaFault<double>"));
      }
      else if (std::is_same<rtype, float>::value)
      {
        attribList.push_back(PbPoco::PbXML::tPair("type", "DeltaFault<float>"));
      }
      else if (std::is_same<rtype, int>::value)
      {
        attribList.push_back(PbPoco::PbXML::tPair("type", "DeltaFault<int>"));
      }

      attribList.push_back(PbPoco::PbXML::tPair("conditions", string("v1 - v2 > W:") +
                                                              Poco::NumberFormatter::format(maxwarn, display_precision) + " > T: " +
                                                              Poco::NumberFormatter::format(maxtrip, display_precision)));

      attribList.push_back(PbPoco::PbXML::tPair("currVal", Poco::NumberFormatter::format(value1, display_precision) + ", " +
                                                           Poco::NumberFormatter::format(value2, display_precision)));
    }

protected:
    rtype &maxwarn, &maxtrip;

    rtype &value1, &value2;

};


}

