//
// Created by gmorehead on 11/9/16.
//

#pragma once

#include "FaultBase.h"


namespace PBFault {


////////////////////////////////////////////////////////////////////////////////////////////////////
class VectorDeltaFault
        : public FaultBase {

public:

    VectorDeltaFault(ITick &ticker, const char *fcode, EventLogger &evh,
                     vector<double *> val_vector, bool &en_bit, double &maxw, double &maxt, fault_init_struct init, iHandleFaultResponse *responseHandler)
              : PBFault::FaultBase(ticker, fcode, evh, en_bit, init, responseHandler)
              , maxwarn(maxw), maxtrip(maxt)
    {
      values = val_vector;
    }

    ~VectorDeltaFault()
    {
      values.clear();
    }

    virtual bool Detect(fault_op_modes mode)
    {
      bool retval = false;

      if (!enable)
      {
        ResetTriggers();
        return retval;
      }

      _minval = *values[0];
      _maxval = *values[0];
      for (unsigned int idx = 1; idx < values.size(); ++idx)
      {
        _minval = std::min(_minval, (double) *values[idx]);
        _maxval = std::max(_maxval, (double) *values[idx]);
      }

      newflt = false;  // flag only set for one detect cycle, then goes away

      bool triptest = fabs(_maxval - _minval) >= maxtrip;
      retval = DetectLevel(triptest, PBFault::trip, mode);

      bool warntest = fabs(_maxval - _minval) >= maxwarn;
      if (!retval)
        retval = DetectLevel(warntest, PBFault::warning, mode);

      return (retval);
    }

    virtual string createEventText() override
    {
      string etxt = "range: " + NumberFormatter::format(fabs(_maxval - _minval), display_precision);

      if(level == PBFault::warning)
      {
        etxt += " >= " + Poco::NumberFormatter::format(maxwarn, display_precision);
      }
      else if(level >= PBFault::trip)
      {
        etxt += " >= " + Poco::NumberFormatter::format(maxtrip, display_precision);
      }

      return etxt;
    }


    void sprintinfo(char *buf)
    {
      FaultBase::sprintinfo(buf, "double");
      snprintf(buf, MAX_INFO, "%s, , %.3f, %.3f", buf, maxwarn, maxtrip);
    }

    void GenCode(char *buf)
    {
      if (values.size() >= 2)
      {
        char codebuf[MAX_CODE];
        FaultBase::GenCode(codebuf);
        snprintf(buf, MAX_CODE, "%s{%.1f, %0.1f}", codebuf, *values[0], *values[1]); //add the value to the end of the code
      }
    }

    virtual void writeSpecializedXmlAtribs(PbPoco::PbXML::tAttribs &attribList)
    {
      attribList.push_back(PbPoco::PbXML::tPair("type", "VectorDelta"));
      attribList.push_back(PbPoco::PbXML::tPair("conditions", string("delta > warn:") +
                                                              Poco::NumberFormatter::format(maxwarn, display_precision) + " > trip: " +
                                                              Poco::NumberFormatter::format(maxtrip, display_precision)));
      attribList.push_back(PbPoco::PbXML::tPair("currVal", Poco::NumberFormatter::format(_maxval - _minval, display_precision)));
    }

protected:
    double &maxwarn, &maxtrip;

    vector<double *> values;

    double _minval;
    double _maxval;
};


}

