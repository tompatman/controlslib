//
// Created by gmorehead on 12/21/18.
//


#pragma once

#include "IFault.h"

namespace PBFault {

class iHandleFaultResponse {

public:
    virtual bool handleResponse(PBFault::IFault &fault, PBFault::fault_op_modes mode) = 0;

};

}

