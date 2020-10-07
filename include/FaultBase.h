/*
 * FaultBase.h
 *
 *
 * */

#pragma once

#include "Poco/BasicEvent.h"
#include "Poco/Delegate.h"
#include "Poco/XML/XMLWriter.h"
#include "eventDb/EventLogger.h"
#include "core/ppcLogger.h"
#include <time.h>
#include <xml/GeneralAttributes.h>
#include "FaultTrigger.h"
#include "IFault.h"
#include "IFaultContainer.h"
#include "iHandleFaultResponse.h"

namespace PBFault {

#define FAULTMAN_SHM_STRUCT_VERSION 1
#define SHAREDFAULTSTATUSPATH "/www/shared"
#define SHAREDFAULTSTATUS_NAME "fault-status"
#define MAX_NAME 83 //52
#define MAX_FAULT 1023
#define MAX_INFO 256
#define MAX_CODE 52 // 32


struct fault_init_struct
{
    fault_init_struct(int primary_loc, int secondary_loc, string name, string desc, Poco::Int64 &warn_ms, Poco::Int64 &trip_ms,
                      fault_reactions critical, fault_reactions tolerant, fault_reactions averse, IFaultContainer *fc, int display_precision=2)
            : fault_container(fc)
              , location_primary(primary_loc)
              , location_secondary(secondary_loc)
              , name(name)
              , description(desc)
              , warndelay_ms(warn_ms)
              , tripdelay_ms(trip_ms)
              , precision(display_precision)
    {
      fault_responses[critical_only] = critical;
      fault_responses[fault_tolerant] = tolerant;
      fault_responses[fault_averse] = averse;
    };

    IFaultContainer *fault_container;

    int location_primary;
    int location_secondary;
    string name;
    string description;
    Poco::Int64 &warndelay_ms;
    Poco::Int64 &tripdelay_ms;
    int precision;
    fault_reactions fault_responses[num_fault_shutdown_modes];
};

/////////////////////////////////////////////////////////////////////////////////////////////////////
class FaultBase
        : public IFault {

public:

    FaultBase(ITick &ticker, const char *fcode, IEventLogger &evh, bool &en_bit, fault_init_struct init, iHandleFaultResponse *responseHandler);

    virtual ~FaultBase();

    virtual bool Detect(fault_op_modes mode) = 0;

    virtual void writeSpecializedXmlAtribs(PbPoco::PbXML::tAttribs &attribList) = 0;

    virtual string createEventText()
    { return ""; };    // It's up to derived classes to create event details.

    void ClearFault(faultlevel);

    virtual void writeAsXml(Poco::XML::XMLWriter &xmlWriter);

    virtual void writeAsXml_Simple(Poco::XML::XMLWriter &xmlWriter, Poco::UInt16 &idCounter);

    virtual void sprintinfo(char *buf);

    void sprintinfo(char *buf, const char *type);

    bool RaiseFault(faultlevel);

    void LowerFault(faultlevel);

    void ResetTriggers(void);

    time_t get_timestamp(void);

    virtual fault_reactions getReaction(fault_op_modes mode)
    { return fault_responses[mode]; };

    virtual bool validResponseHandler() final
    { return (_responseHandler != nullptr); };

    virtual bool handleResponse(fault_op_modes mode);

    const char *getFaultName()
    { return Name; };

    virtual int getPrimLocId()
    { return primary_location; };

    virtual int getSubLocId()
    { return sub_location; };

    int primary_location;

    int sub_location;

    char Name[MAX_NAME];

    char Event_details[MAX_INFO];

    char Description[MAX_INFO];

    virtual void GenCode(char *buf);

    bool newflt;

    fault_reactions fault_responses[num_fault_shutdown_modes];

    int display_precision = 2;

protected:
    bool DetectLevel(bool val, faultlevel fl, fault_op_modes mode);

    char FLT_CODE[MAX_CODE];

    IEventLogger &evhandler;

    void logFault(fault_op_modes mode);

    bool &enable;

private:

    ITick &_ticker;

    IFaultContainer *_faultContainer = nullptr;

    iHandleFaultResponse *_responseHandler = nullptr;

    Poco::Int64 timestamp = 0;
    FaultTrigger *trigtrip = nullptr;
    FaultTrigger *trigwarn = nullptr;

    Poco::Mutex FAULTMAN_REQUEST_LOCK;
};


}

