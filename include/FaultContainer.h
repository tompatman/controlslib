//
// Created by gmorehead on 11/10/16.
//

#pragma once

#include "IFault.h"
#include "IFaultContainer.h"
#include <limits>
#include <core/ppcLogger.h>
#include <math.h>


namespace PBFault {

static double _pos_inf = INFINITY;
static double _neg_inf = -INFINITY;
static bool _always_enabled = true;
static bool _always_disabled = false;

static Poco::Int64 __msDelay_0 = 0;     // Use these values as reference for delay time if it will be constant.
static Poco::Int64 __msDelay_250 = 250;
static Poco::Int64 __msDelay_500 = 500;
static Poco::Int64 __msDelay_1000 = 1000;
static Poco::Int64 __msDelay_2000 = 2000;
static Poco::Int64 __msDelay_5000 = 5000;
static Poco::Int64 __msDelay_10000 = 10000;
static Poco::Int64 __msDelay_11000 = 11000;
static Poco::Int64 __msDelay_15000 = 15000;
static Poco::Int64 __msDelay_20000 = 20000;
static Poco::Int64 __msDelay_21000 = 21000;
static Poco::Int64 __msDelay_30000 = 30000;
static Poco::Int64 __msDelay_35000 = 35000;
static Poco::Int64 __msDelay_60000 = 60000;
static Poco::Int64 __msDelay_61000 = 61000;
static Poco::Int64 __msDelay_600000 = 600000;
static Poco::Int64 __msDelay_601000 = 601000;


class FaultContainer : public IFaultContainer{

public:

    FaultContainer(const std::string name);

    virtual ~FaultContainer();

    virtual std::string getLocationText(int primary_location) override
    {
      return std::to_string(primary_location);
    };

    virtual std::string getSublocationText(int primary_location, int sub_location) override
    {
      return std::to_string(sub_location);
    };

    virtual void addFault(IFault *fault);

    virtual bool DetectFaults();

    int get_numfaults(void)
    {
      return _fault_list.size();
    };

    IFault *GetFault(unsigned int fault_index); // get a pointer to the fault object

    faultlevel CheckAllFaults();  //returns highest level fault found

    faultlevel CheckAllFaults(vector<IFault *> ignore_list);

    faultlevel CheckFaults(vector<IFault *> fault_list);

    void ClearAllFaults(faultlevel fl); // clears all faults this level or less

    void writeActiveFaultXmlList(Poco::XML::XMLWriter &xmlWriter, bool complete_fault_list = false);

    void writeActiveFaultXmlList_Simple(Poco::XML::XMLWriter &xmlWriter, Poco::UInt16 &idCounter, bool complete_fault_list = false);

    void PrintFaultList(FILE *fp);

    //  New location based fault handling.
    virtual bool DetectFaults(int primaryLocation, int subLocation);

    void ClearLocationFaults(int primaryLocation, int subLocation, faultlevel fl);

    void ClearLocationFaultsAllAt(int primaryLocation, faultlevel fl);

    fault_reactions checkFaultReactState(int primaryLocation, int subLocation, fault_op_modes mode);

    vector<IFault *> &getFaultList(int primaryLocation, int subLocation);

    bool getFaultListExists(int primaryLocation, int subLocation);

    faultlevel CheckFaultsAtLocation(int primaryLocation, int subLocation);

    faultlevel CheckFaultsAtPrimaryLocation(int primaryLocation); //returns highest level fault found at this primaryLocation, includes any sub-locations within the primary.

    IFault *getFault(std::string fltName);

    // Static functions.
    static int global_numfaults()
    {
      return _total_number_of_faults;
    }

    static faultlevel global_CheckAllFaults();  //returns highest level fault found

    static void global_writeActiveFaultXmlList(Poco::XML::XMLWriter &xmlWriter, bool complete_fault_list = false);

    static void global_writeActiveFaultXmlList_Simple(Poco::XML::XMLWriter &xmlWriter, Poco::UInt16 &idCounter, bool complete_fault_list = false);

    static void global_PrintFaultList(FILE *fp);

protected:
    vector<IFault *> _fault_list;

    std::map<std::string, vector<IFault *>> _fault_list_by_loc;

    ppcLogger *_log;

    std::string _list_name;

    static Poco::Mutex _container_list_mutex;

    static vector<FaultContainer *> _container_list;

    static int _total_number_of_faults;
};

}

