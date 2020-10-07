/*   Faultmanager.cc
 * FaultBase.cc
 *
 * */
//#define USE_CODES //define to generate codes instead of named faults

#include "FaultBase.h"

#include <Poco/LocalDateTime.h>
#include <Poco/DateTimeFormatter.h>
#include "iHandleFaultResponse.h"

namespace PBFault {

/*!
 *
 * @param ticker
 * @param fcode This is a unique code string
 * @param evh
 * @param en_bit This should be a reference to an external enable bit.  Mostly intended to be user controlled enable/disable parameter
 * @param init The Initialization structure.
 */
FaultBase::FaultBase(ITick &ticker, const char *fcode, IEventLogger &evh, bool &en_bit, fault_init_struct init, iHandleFaultResponse *responseHandler)
        : IFault()
          , _ticker(ticker)
          , evhandler(evh)
          , enable(en_bit)
          , _responseHandler(responseHandler)
{
  newflt = false;

  primary_location = init.location_primary;
  sub_location = init.location_secondary;

  for (uint i = 0; i < num_fault_shutdown_modes; ++i) fault_responses[i] = init.fault_responses[i];

  timestamp = Poco::Timestamp().epochMicroseconds() / 1000;

  strncpy(Name, init.name.c_str(), MAX_NAME);
  Name[MAX_NAME - 1] = '\0'; //terminate the string if too long
  strncpy(FLT_CODE, fcode, MAX_CODE);
  FLT_CODE[MAX_CODE - 1] = '\0'; //terminate the string if too long
  level = none;  // set to none, only if reinitializing

  memset(Event_details, 0, sizeof(Event_details));

  trigwarn = new FaultTrigger(ticker, init.warndelay_ms);
  trigtrip = new FaultTrigger(ticker, init.tripdelay_ms);

  strncpy(Description, init.description.c_str(), sizeof(Description));

  display_precision = init.precision;

  _faultContainer = init.fault_container;
}

FaultBase::~FaultBase()
{
  //if this is the last fault to be removed, then destroy the fault manager

  delete trigtrip;
  delete trigwarn;

}

bool FaultBase::RaiseFault(faultlevel type)
{
  if (false == enable)
  {
    return false;
  }

  //This function should set the bits 'type' in the data structure, element number 'number'.  It should only make the fault number bigger, not reduce it.
  FAULTMAN_REQUEST_LOCK.lock();

  if (level < type)
  {
    level = type;
    timestamp = Poco::Timestamp().epochMicroseconds() / 1000;
    newflt = true;

    strncpy(Event_details, createEventText().c_str(), sizeof(Event_details));
  }

  FAULTMAN_REQUEST_LOCK.unlock();
  return (newflt);
}


void FaultBase::LowerFault(faultlevel type)
{
  //This function should set the bits 'type' in the data structure, element number 'number'.  The function can be  used to clear faults.
  FAULTMAN_REQUEST_LOCK.lock();

  if (level > type) level = type;

  FAULTMAN_REQUEST_LOCK.unlock();
}


void FaultBase::writeAsXml(Poco::XML::XMLWriter &xmlWriter)
{

  PbPoco::PbXML::tAttribs valueList;

  Poco::Timestamp msg_ts(timestamp * 1000);
  Poco::LocalDateTime msg_ldt(msg_ts);

  valueList.push_back(PbPoco::PbXML::tPair("idx", Poco::NumberFormatter::format(faultnumber)));
  valueList.push_back(PbPoco::PbXML::tPair("fault_code", FLT_CODE));
  valueList.push_back(PbPoco::PbXML::tPair("timestamp", Poco::DateTimeFormatter::format(msg_ldt, "%H:%M:%S")));
  valueList.push_back(PbPoco::PbXML::tPair("name", Name));
  valueList.push_back(PbPoco::PbXML::tPair("desc", Description));
  valueList.push_back(PbPoco::PbXML::tPair("ev_details", Event_details));
  if (trigwarn->get_trigticks() != trigtrip->get_trigticks())
  {
    valueList.push_back(PbPoco::PbXML::tPair("warn_ms", std::to_string(trigwarn->get_trigticks())));    // Don't display it if it's the same.
  }
  valueList.push_back(PbPoco::PbXML::tPair("trip_ms", std::to_string(trigtrip->get_trigticks())));

  if (_faultContainer != NULL)
  {
    valueList.push_back(PbPoco::PbXML::tPair("primLoc", _faultContainer->getLocationText(primary_location)));
    valueList.push_back(PbPoco::PbXML::tPair("subLoc", _faultContainer->getSublocationText(primary_location, sub_location)));
  }
  else
  {
    valueList.push_back(PbPoco::PbXML::tPair("primLoc", Poco::NumberFormatter::format(primary_location)));
    valueList.push_back(PbPoco::PbXML::tPair("subLoc", Poco::NumberFormatter::format(sub_location)));
  }

  for (int idx = 0; idx < num_fault_shutdown_modes; ++idx)
  {
    valueList.push_back(PbPoco::PbXML::tPair(string("mode_") + getFaultOpModeText((fault_op_modes) idx), getFaultReactionsText(fault_responses[idx])));
  }

  writeSpecializedXmlAtribs(valueList);

  PbPoco::PbXML::GeneralAttributes genAttribs(valueList);

  xmlWriter.startElement("", "", "Fault", genAttribs);
  if (enable || level > none)
  {
    xmlWriter.characters(FaultnumToString(level));
  }
  else
  {
    xmlWriter.characters("disabled");
  }
  xmlWriter.endElement("", "", "Fault");
}

void FaultBase::writeAsXml_Simple(Poco::XML::XMLWriter &xmlWriter, Poco::UInt16 &idCounter)
{
  PbPoco::PbXML::tAttribs valueList;

  Poco::Timestamp msg_ts(timestamp * 1000);
  Poco::LocalDateTime msg_ldt(msg_ts);

  valueList.push_back(PbPoco::PbXML::tPair("fault_code", FLT_CODE));
  valueList.push_back(PbPoco::PbXML::tPair("timestamp", Poco::DateTimeFormatter::format(msg_ldt, "%H:%M:%S")));
  valueList.push_back(PbPoco::PbXML::tPair("name", Name));
  //valueList.push_back(PbPoco::PbXML::tPair("desc", Description));
  valueList.push_back(PbPoco::PbXML::tPair("ev_details", Event_details));
  valueList.push_back(PbPoco::PbXML::tPair("idx", Poco::NumberFormatter::format(faultnumber)));

  valueList.push_back(PbPoco::PbXML::tPair("u_id", Poco::NumberFormatter::format(idCounter++)));

  PbPoco::PbXML::GeneralAttributes genAttribs(valueList);

  xmlWriter.startElement("", "", "Fault", genAttribs);
  xmlWriter.characters(FaultnumToString(level));
  xmlWriter.endElement("", "", "Fault");
}


void FaultBase::ClearFault(faultlevel type)
{
  //This function should set the bits 'type' in the data structure, element number 'number'.  The function can be  used to clear faults.
  FAULTMAN_REQUEST_LOCK.lock();

  if ((type >= level) && (level > none))
  {
    level = none;
    memset(Event_details, 0, sizeof(Event_details));

    trigwarn->Reset();

    if (type >= trip) trigtrip->Reset();
  }

  FAULTMAN_REQUEST_LOCK.unlock();
}

void FaultBase::ResetTriggers(void)
{
  trigwarn->Reset();
  trigtrip->Reset();
}


time_t FaultBase::get_timestamp(void)
{
  return timestamp;
}

bool FaultBase::handleResponse(fault_op_modes mode)
{
  if(_responseHandler != nullptr)
    return _responseHandler->handleResponse(*this, mode);
  else
    return false;
}

bool FaultBase::DetectLevel(bool val, faultlevel fl, fault_op_modes mode)
{
  bool fltflag = false;

  if (-1 == faultnumber)    //return a fault if uninitialized
  {
    //ppsyslog(LOG_ERR, "Fault::DetectLevel Uninitialized!!!! (0x%lx)", this);
    return (fatal);
  }

  if (disabled != level)   //if fault is disabled, don't run the triggers
  {
    if (warning == fl)
    {
      if (trigwarn->Step(val))
      {
        fltflag |= RaiseFault(fl);
      }
    }
    else
    {
      if (trigtrip->Step(val))
      {
        fltflag |= RaiseFault(fl);
      }
    }
  }

  if (fltflag)
  {
    logFault(mode);
  }

  return (fltflag);
}


void FaultBase::logFault(fault_op_modes mode)
{
  //ppsyslog(LOG_INFO, "logFault::code");
  char code[MAX_CODE];
  GenCode(code);  // virtual, will call derived function

  if (mode <= invalid_fault_op_mode || mode >= num_fault_shutdown_modes)   // Invalid operation mode.
  {
    if ((level >= trip))  // Should never get here...
    {
      evhandler.logEvent(URGENT_SERVICE_REQUEST, string(" invalid opmode ") + Name + " : " + FaultnumToString(level));
    }
    else if ((level == warning))
    {
      evhandler.logEvent(SERVICE_REQUEST, string(" invalid opmode ") + Name + " : " + FaultnumToString(level));
    }
  }
  else
  {
    int evtype = EVENT_NOTICE;

    if ((level >= trip))
    {
      evtype = SERVICE_REQUEST;                                                 // It's now at least a service_request.
      if (getReaction(mode) > react_none) evtype = URGENT_SERVICE_REQUEST;      // Reaction occurring so it's urgent.
    }
    else if ((level == warning))
    {
      if (getReaction(mode) > react_none) evtype = SERVICE_REQUEST;         // It will react if it turn's into a trip, so make it a service_request.
    }

    if(Event_details == "")
    {
      evhandler.logEvent(evtype, string(Name) + " : " + FaultnumToString(level));
    }
    else
    {
      evhandler.logEvent(evtype, string(Name) + " : " + FaultnumToString(level) + ", " + Event_details);
    }
  }
}

void FaultBase::sprintinfo(char *buf)
{
  snprintf(buf, MAX_INFO, "%d, %s, %s, %s", faultnumber, "Unknown", Name, FLT_CODE);
}

void FaultBase::sprintinfo(char *buf, const char *type)
{
  snprintf(buf, MAX_INFO, "%d, %s, %s, %s", faultnumber, type, Name, FLT_CODE);
}


void FaultBase::GenCode(char *buf)
{
#if USE_CODES
  snprintf(buf, MAX_CODE, "%s", FLT_CODE); //add the value to the end of the code
#else
  snprintf(buf, MAX_CODE, "%s", Name); //add the value to the end of the code
#endif
}


}
