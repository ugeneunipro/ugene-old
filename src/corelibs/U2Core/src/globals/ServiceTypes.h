#ifndef _U2_SERVICE_TYPE_H_
#define _U2_SERVICE_TYPE_H_

#include "ServiceModel.h"

namespace U2 {


const ServiceType Service_PluginViewer			= 1;
const ServiceType Service_Project				= 2;
const ServiceType Service_ProjectView			= 3;

const ServiceType Service_DNAGraphPack			= 10;
const ServiceType Service_DNAExport             = 11;
const ServiceType Service_TestRunner            = 12;
const ServiceType Service_ScriptRegistry        = 13;
const ServiceType Service_ExternalToolSupport   = 14;

const ServiceType Service_MinCoreServiceId = 500;
const ServiceType Service_MaxCoreServiceId = 1000;

}//namespace

#endif
