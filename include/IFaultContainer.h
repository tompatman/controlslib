//
// Created by gmorehead on 5/29/18.
//


#pragma once


#include <string>

namespace PBFault {


class IFaultContainer {

public:
    virtual std::string getLocationText(int primary_location) = 0;

    virtual std::string getSublocationText(int primary_location, int sub_location) = 0;

    virtual void addFault(IFault *fault) = 0;

};

}
