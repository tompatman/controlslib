//
// Created by gmorehead on 11/9/16.
//

#pragma once

#include <Poco/XML/XMLWriter.h>
#include "vector"


namespace PBFault {

using std::vector;

typedef enum {
    disabled
    , none
    , warning
    , trip
    , fatal
} faultlevel;

static const char *getFaultLevelText(faultlevel lev)
{
  switch (lev)
  {
    case disabled:
      return "disabled";
    case none:
      return "none";
    case warning:
      return "warning";
    case trip:
      return "trip";
    case fatal:
      return "fatal";
    default:
      return("error");
  }

  return("Unknown");
}

typedef enum {
    invalid_fault_op_mode = -1
    , critical_only
    , fault_tolerant
    , fault_averse
    , num_fault_shutdown_modes
} fault_op_modes;


static const char *getFaultOpModeText(fault_op_modes mode)
{
  switch (mode)
  {
    case critical_only:
      return "critical_only";
    case fault_tolerant:
      return "tolerant";
    case fault_averse:
      return "averse";
    case invalid_fault_op_mode:
    case num_fault_shutdown_modes:
      return "invalid";
  }
}

typedef enum {
    react_none
    , react_standby
    , react_offline
    , react_epo
    , react_shutdown
} fault_reactions;

static const char *getFaultReactionsText(fault_reactions reaction)
{
  switch (reaction)
  {
    case react_none:
      return "none";
    case react_standby:
      return "standby";
    case react_offline:
      return "offline";
    case react_epo:
      return "epo";
    case react_shutdown:
      return "shutdown";
  }
}

class IFault {

public:

    virtual bool Detect(fault_op_modes mode = fault_tolerant) = 0;

    virtual void ClearFault(faultlevel) = 0;

    virtual void writeAsXml(Poco::XML::XMLWriter &xmlWriter) = 0;

    virtual void writeAsXml_Simple(Poco::XML::XMLWriter &xmlWriter, Poco::UInt16 &idCounter) = 0;

    virtual void sprintinfo(char *buf) = 0;

    virtual const char *getFaultName() = 0;

    virtual int getPrimLocId() = 0;

    virtual int getSubLocId() = 0;

    virtual fault_reactions getReaction(fault_op_modes mode) = 0;

    virtual bool validResponseHandler() = 0;

    virtual bool handleResponse(fault_op_modes mode) = 0;

    faultlevel CheckFault()
    {
      return (level);
    }

    void setFaultnumber(int faultnumber)
    {
      IFault::faultnumber = faultnumber;
    }

    int get_faultnumber(void)
    {
      return faultnumber;
    }


    const char *FaultnumToString(faultlevel num)
    {
      return getFaultLevelText(num);
    }

protected:

    int faultnumber;

    faultlevel level;
};

}

