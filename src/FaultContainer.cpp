//
// Created by gmorehead on 11/10/16.
//

#include "FaultContainer.h"

namespace PBFault {

vector<FaultContainer *> FaultContainer::_container_list;

Poco::Mutex FaultContainer::_container_list_mutex;

int FaultContainer::_total_number_of_faults = 0;

FaultContainer::FaultContainer(const std::string name)
{
  _list_name = name;

  _log = new ppcLogger(name.c_str());

  _container_list_mutex.lock();

  _container_list.push_back(this);

  _container_list_mutex.unlock();
}

FaultContainer::~FaultContainer()
{
  for (unsigned int i = 0; i < _fault_list.size(); i++)
  {
    delete _fault_list[i];
  }
  _fault_list.clear();

  _container_list_mutex.lock();

  for (vector<FaultContainer *>::iterator fitr = _container_list.begin();
       fitr != _container_list.end(); ++fitr)
  {
    if (this == *fitr)
    {
      _container_list.erase(fitr);
      break;
    }
  }

  _container_list_mutex.unlock();

  delete _log;
}


void FaultContainer::addFault(IFault *fault)
{
  _fault_list.push_back(fault);

  if (fault->getPrimLocId() >= 0 && fault->getSubLocId() >= 0)
  {
    std::string locAddr = std::to_string(fault->getPrimLocId()) + ":" + std::to_string(fault->getSubLocId());
    _fault_list_by_loc[locAddr].push_back(fault);
  }

  fault->setFaultnumber(++FaultContainer::_total_number_of_faults);
}

bool FaultContainer::DetectFaults()
{
  bool new_faults = false;

  for (unsigned int i = 0; i < _fault_list.size(); i++)
  {
    new_faults |= _fault_list[i]->Detect(fault_tolerant);
  }

  return new_faults;
}

bool FaultContainer::getFaultListExists(int primaryLocation, int subLocation)
{
  std::string locAddr = std::to_string(primaryLocation) + ":" + std::to_string(subLocation);

  auto fault_list = _fault_list_by_loc.find(locAddr);

  return (fault_list != _fault_list_by_loc.end());
}

faultlevel FaultContainer::CheckFaultsAtLocation(int primaryLocation, int subLocation)
{
  faultlevel lev = none;

  for (auto a_fault : getFaultList(primaryLocation, subLocation))
  {
    lev = std::max(lev, a_fault->CheckFault());
  }

  return lev;
}

faultlevel FaultContainer::CheckFaultsAtPrimaryLocation(int primaryLocation)
{
  //returns highest level fault found
  faultlevel MaxLevel = none;

  //Check all faults, and return the highest level found.
  for (auto a_fault : _fault_list)
  {
    if (a_fault->getPrimLocId() == primaryLocation)
    {
      MaxLevel = std::max(MaxLevel, a_fault->CheckFault());
    }
  }

  return (MaxLevel);

}

IFault *FaultContainer::getFault(std::string fltName)
{
  IFault *theFault = NULL;

  for (auto a_fault : _fault_list)
  {
    if (a_fault->getFaultName() == fltName)
    {
      return a_fault;
    }
  }

  std::string errstr = "getFault(" + fltName + ") called with invalid fault name.";
  _log->log(LG_ERR, errstr.c_str());
  throw Poco::InvalidArgumentException(errstr);
}

vector<IFault *> &FaultContainer::getFaultList(int primaryLocation, int subLocation)
{
  std::string locAddr = std::to_string(primaryLocation) + ":" + std::to_string(subLocation);

  auto fault_list = _fault_list_by_loc.find(locAddr);

  if (fault_list == _fault_list_by_loc.end())
  {
    throw Poco::InvalidArgumentException("getFaultList(" + std::to_string(primaryLocation) + ", " + std::to_string(subLocation) + ") called with invalid location id!");
  }

  return _fault_list_by_loc[locAddr];
}


bool FaultContainer::DetectFaults(int primaryLocation, int subLocation)
{
  bool new_faults = false;

  for (auto a_fault : getFaultList(primaryLocation, subLocation)) new_faults |= a_fault->Detect(fault_tolerant);

  return new_faults;
}

void FaultContainer::ClearLocationFaults(int primaryLocation, int subLocation, faultlevel fl)
{
  if (getFaultListExists(primaryLocation, subLocation))
  {
    for (auto a_fault : getFaultList(primaryLocation, subLocation)) a_fault->ClearFault(fl);
  }
}

void FaultContainer::ClearLocationFaultsAllAt(int primaryLocation, faultlevel fl)
{
  int subLocation = 0;

  while (getFaultListExists(primaryLocation, subLocation))
  {
    for (auto a_fault : getFaultList(primaryLocation, subLocation)) a_fault->ClearFault(fl);

    ++subLocation;
  }
}


fault_reactions FaultContainer::checkFaultReactState(int primaryLocation, int subLocation, fault_op_modes mode)
{
  fault_reactions max_react = react_none;

  if (getFaultListExists(primaryLocation, subLocation))
  {
    for (auto a_fault : getFaultList(primaryLocation, subLocation))
    {
      if (a_fault->CheckFault() >= trip)
      {
        max_react = std::max(max_react, a_fault->getReaction(mode));
      }
    }
  }

  return max_react;
}

IFault *FaultContainer::GetFault(unsigned int fault_index)
{
  if (fault_index < _fault_list.size())
  {
    return _fault_list[fault_index];
  }
  else
  {
    return _fault_list[0];
  }
}

faultlevel FaultContainer::CheckAllFaults()
{
  //returns highest level fault found
  faultlevel MaxLevel = none;

  //Check all faults, and return the highest level found.
  for (auto a_fault : _fault_list)
  {
    MaxLevel = std::max(MaxLevel, a_fault->CheckFault());
  }

  return (MaxLevel);
}

faultlevel FaultContainer::CheckAllFaults(std::vector<IFault *> ignore_list)
{
  //returns highest level fault found
  faultlevel MaxLevel = none;

  //Check all faults, and return the highest level found.
  for (unsigned int i = 0; i < _fault_list.size(); i++)
  {
    bool ignore = false;

    for (unsigned int x = 0; !ignore && x < ignore_list.size(); ++x)
      if (_fault_list[i] == ignore_list[x])
        ignore = true;

    if (!ignore && MaxLevel < _fault_list[i]->CheckFault())
    {
      MaxLevel = _fault_list[i]->CheckFault();
    }
  }

  return (MaxLevel);
}

faultlevel FaultContainer::CheckFaults(std::vector<IFault *> fault_list)
{
  //returns highest level fault found
  faultlevel MaxLevel = none;

  for (unsigned int i = 0; i < fault_list.size(); ++i)
  {
    MaxLevel = std::max(MaxLevel, fault_list[i]->CheckFault());
  }

  return (MaxLevel);
}


void FaultContainer::ClearAllFaults(faultlevel fl)
{
  // clears all faults this level or less
  for (unsigned int i = 0; i < _fault_list.size(); i++)
  {
    if (_fault_list[i]->CheckFault() >= fl)
    {
      _log->log(LG_INFO, "Clearing %s, fault: %s", _fault_list[i]->FaultnumToString(_fault_list[i]->CheckFault()), _fault_list[i]->getFaultName());
    }

    _fault_list[i]->ClearFault(fl);
  }
}

void FaultContainer::writeActiveFaultXmlList(Poco::XML::XMLWriter &xmlWriter, bool complete_fault_list)
{
  for (unsigned int f = 0; f < _fault_list.size(); f++)
  {
    if (_fault_list[f]->CheckFault() >= warning || complete_fault_list)
    {
      _fault_list[f]->writeAsXml(xmlWriter);
    }
  }
}

void FaultContainer::writeActiveFaultXmlList_Simple(Poco::XML::XMLWriter &xmlWriter, Poco::UInt16 &idCounter, bool complete_fault_list)
{
  for (unsigned int f = 0; f < _fault_list.size(); f++)
  {
    if (_fault_list[f]->CheckFault() >= warning || complete_fault_list)
    {
      _fault_list[f]->writeAsXml_Simple(xmlWriter, idCounter);
    }
  }
}


void FaultContainer::PrintFaultList(FILE *fp)
{
  char str[256];
  fprintf(fp, "Fault List\n\n");
  fprintf(fp, "DataNum, Type, DigName, Code, Trip State, MinTrip, Min Warn, Max Warn, Max Trip\n");

  for (unsigned int f = 0; f < _fault_list.size(); f++)
  {
    _fault_list[f]->sprintinfo(str);
    fprintf(fp, "%s\n", str);
  }
}


faultlevel FaultContainer::global_CheckAllFaults()
{
  faultlevel MaxLevel = none;

  _container_list_mutex.lock();

  for (auto flt_obj : _container_list)
  {
    MaxLevel = std::max(MaxLevel, flt_obj->CheckAllFaults());
  }

  _container_list_mutex.unlock();

  return MaxLevel;
}

void FaultContainer::global_writeActiveFaultXmlList(Poco::XML::XMLWriter &xmlWriter, bool complete_fault_list)
{
  for (auto flt_obj : _container_list)
  {
    flt_obj->writeActiveFaultXmlList(xmlWriter, complete_fault_list);
  }
}

void FaultContainer::global_writeActiveFaultXmlList_Simple(Poco::XML::XMLWriter &xmlWriter, Poco::UInt16 &idCounter, bool complete_fault_list)
{
  for (auto flt_obj : _container_list)
  {
    flt_obj->writeActiveFaultXmlList_Simple(xmlWriter, idCounter, complete_fault_list);
  }
}

void FaultContainer::global_PrintFaultList(FILE *fp)
{

}

}
