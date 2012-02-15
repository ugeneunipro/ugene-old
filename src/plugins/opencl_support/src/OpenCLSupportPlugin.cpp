/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#define _CRT_SECURE_NO_WARNINGS

#include <QtCore/QLibrary>

#include <U2Core/AppContext.h>
#include <U2Core/AppResources.h>
#include <U2Core/Settings.h>
#include <U2Core/Log.h>

#include <U2Core/GAutoDeleteList.h>

#include "OpenCLSupportPlugin.h"
#include "OpenCLSupportSettingsController.h"

namespace U2 {

    extern "C" Q_DECL_EXPORT Plugin * U2_PLUGIN_INIT_FUNC() {
    OpenCLSupportPlugin * plug = new OpenCLSupportPlugin();
    return plug;
}

const static char * RESOURCE_OPENCL_GPU_NAME = "OpenCLGpu";

OpenCLSupportPlugin::OpenCLSupportPlugin() : Plugin(tr("OpenCL Support"), 
                                                    tr("Plugin provides support for OpenCL-enabled GPUs.") ) {
    QString err_str;
    OpenCLSupportError err = obtainGpusInfo( err_str );
    if( err_str.isEmpty() && gpus.empty() ) {
        err_str = "No OpenCL-enabled GPUs found.";
    }
    if( Error_NoError == err ) {
        loadGpusSettings();
        registerAvailableGpus();
    } else {
        coreLog.details( err_str );
    }

    //adding settings page
    if (AppContext::getMainWindow()) {
        QString settingsPageMsg = getSettingsErrorString(err);
        AppContext::getAppSettingsGUI()->registerPage( new OpenCLSupportSettingsPageController(settingsPageMsg) );
    }

    //registering gpu resource
    if( !gpus.empty() ) {
        AppResource * gpuResource = new AppResource( RESOURCE_OPENCL_GPU, gpus.size(), RESOURCE_OPENCL_GPU_NAME);
        AppResourcePool::instance()->registerResource( gpuResource );
    }
}

QString OpenCLSupportPlugin::getSettingsErrorString( OpenCLSupportError err ) {
    switch(err) {
        case Error_NoError:
            return QString("");

        case Error_BadDriverLib:
             return tr("Cannot load OpenCL driver dynamic library.<p> \
                       Install the latest video GPU driver." );

        case Error_OpenCLError:
            return tr("An error has occurred while obtaining information \
                      about installed OpenCL GPUs.<br>\
                      See OpenCL Support plugin log for details." );

        default: assert(false); return QString();
    }
}

OpenCLSupportPlugin::OpenCLSupportError OpenCLSupportPlugin::obtainGpusInfo( QString & errStr )
{
    //load driver library
    const OpenCLHelper& openCLHelper = AppContext::getOpenCLGpuRegistry()->getOpenCLHelper();
    if (!openCLHelper.isLoaded()) {
        errStr = openCLHelper.getErrorString();
        return Error_BadDriverLib;
    }

    cl_int errCode = 0;

    coreLog.details( tr("Initializing OpenCL") );

    //numEntries is the number of cl_platform_id entries that can be added to platforms
    cl_uint numPlatformEntries = 15;
    gauto_array<cl_platform_id> platformIDs (new cl_platform_id[numPlatformEntries]);
    cl_uint numPlatforms = 0;

    errCode = openCLHelper.clGetPlatformIDs_p(numPlatformEntries, platformIDs.get(), &numPlatforms);
    if (hasOPENCLError(errCode, errStr)) {
        return Error_OpenCLError;
    }
    coreLog.details(tr("Number of OpenCL platforms: %1").arg(numPlatforms));

    //Get each platform info
    for (unsigned int i = 0; i < numPlatforms; i++) {

        //numEntries is the number of cl_platform_id entries that can be added to platforms
        cl_uint numDeviceEntries = 15;
        gauto_array<cl_device_id> deviceIDs (new cl_device_id[numDeviceEntries]);

        cl_uint numDevices = 0;

        errCode = openCLHelper.clGetDeviceIDs_p(platformIDs.get()[i], CL_DEVICE_TYPE_GPU, numDeviceEntries, deviceIDs.get(), &numDevices);
        if (hasOPENCLError(errCode, errStr)) {
            return Error_OpenCLError;
        }
        coreLog.details(tr("Number of OpenCL devices: %1").arg(numDevices));

        for (unsigned int k = 0; k < numDevices; k++) {

            cl_device_id deviceId = deviceIDs.get()[k];

            int maximumParamLength = 200;
            gauto_array<char> paramValue (new char[maximumParamLength]);
            size_t actualParamSize;
            int actualParamLength = 0;            

            //******************************
            errCode = openCLHelper.clGetDeviceInfo_p(deviceId, CL_DEVICE_VENDOR, sizeof(char)*maximumParamLength, paramValue.get(), &actualParamSize);
            if (hasOPENCLError(errCode, errStr)) {
                return Error_OpenCLError;
            }

            actualParamLength = (actualParamSize) / sizeof(char);
            gauto_array<char> vendorNameValue(new char[actualParamLength + 1]);
            strncpy(vendorNameValue.get(), paramValue.get(), actualParamLength);

            QString vendorName = vendorNameValue.get();            
            //******************************
            errCode = openCLHelper.clGetDeviceInfo_p(deviceId, CL_DEVICE_NAME , sizeof(char)*maximumParamLength, paramValue.get(), &actualParamSize);
            if (hasOPENCLError(errCode, errStr)) {
                return Error_OpenCLError;
            }

            actualParamLength = (actualParamSize) / sizeof(char);
            gauto_array<char> deviceNameValue(new char[actualParamLength + 1]);
            strncpy(deviceNameValue.get(), paramValue.get(), actualParamLength);

            QString deviceName = deviceNameValue.get();                        
            //******************************
            cl_ulong globalMemSize = 0;

            errCode = openCLHelper.clGetDeviceInfo_p(deviceId, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong), &globalMemSize, &actualParamSize);
            if (hasOPENCLError(errCode, errStr)) {
                return Error_OpenCLError;
            }
            //******************************
            cl_ulong localMemSize = 0;

            errCode = openCLHelper.clGetDeviceInfo_p(deviceId, CL_DEVICE_LOCAL_MEM_SIZE , sizeof(cl_ulong), &localMemSize, &actualParamSize);
            if (hasOPENCLError(errCode, errStr)) {
                return Error_OpenCLError;
            }
            //******************************
            cl_uint maxClockFrequency = 0;

            errCode = openCLHelper.clGetDeviceInfo_p(deviceId, CL_DEVICE_MAX_CLOCK_FREQUENCY  , sizeof(cl_uint), &maxClockFrequency, &actualParamSize);
            if (hasOPENCLError(errCode, errStr)) {
                return Error_OpenCLError;
            }
            //******************************
            cl_uint maxComputeUnits = 10;

            errCode = openCLHelper.clGetDeviceInfo_p(deviceId, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &maxComputeUnits, &actualParamSize);
            if (hasOPENCLError(errCode, errStr)) {                
                return Error_OpenCLError;
            }
            //******************************
            cl_uint maxWorkItemDimensions = 0;

            errCode = openCLHelper.clGetDeviceInfo_p(deviceId, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(cl_uint), &maxWorkItemDimensions, &actualParamSize);
            if (hasOPENCLError(errCode, errStr)) {                
                return Error_OpenCLError;
            }
            //******************************
            size_t maxWorkGroupSize = 0;

            errCode = openCLHelper.clGetDeviceInfo_p(deviceId, CL_DEVICE_MAX_WORK_GROUP_SIZE , sizeof(size_t), &maxWorkGroupSize, &actualParamSize);
            if (hasOPENCLError(errCode, errStr)) {                
                return Error_OpenCLError;
            }

            //create OpenCL model
            OpenCLGpuModel * openCLGpuModel = new OpenCLGpuModel( vendorName + " " + deviceName,
                                                                  OpenCLGpuId((long)deviceId),
                                                                  (qint64)platformIDs.get()[i],
                                                                  globalMemSize,
                                                                  localMemSize,
                                                                  maxComputeUnits,
                                                                  maxWorkGroupSize,
                                                                  maxClockFrequency);
            gpus.push_back(openCLGpuModel);
            coreLog.info( tr("Registering OpenCL-enabled GPU: %1, global mem: %2 Mb, \
                             local mem: %3 Kb, max compute units: %4, \
                             max work group size: %5, max frequency: %6 Hz")
                .arg(openCLGpuModel->getName())
                .arg(openCLGpuModel->getGlobalMemorySizeBytes() / (1024 * 1024))
                .arg(openCLGpuModel->getLocalMemorySizeBytes() / 1024)
                .arg(openCLGpuModel->getMaxComputeUnits())
                .arg(openCLGpuModel->getMaxWorkGroupSize())
                .arg(openCLGpuModel->getMaxClockFrequency()));
        }
    }

    return Error_NoError;
}

bool OpenCLSupportPlugin::hasOPENCLError(cl_int errCode, QString& errMessage) {
    //TODO: print details error message
    if(errCode != CL_SUCCESS) {
        errMessage = tr("OpenCL error code (%1)").arg(errCode);
        return true;
    } else {
        return false;
    }
}

void OpenCLSupportPlugin::registerAvailableGpus() {
    foreach( OpenCLGpuModel * m, gpus ) {
        AppContext::getOpenCLGpuRegistry()->registerOpenCLGpu(m);
    }
}

void OpenCLSupportPlugin::loadGpusSettings() {
    Settings * s = AppContext::getSettings();
    foreach( OpenCLGpuModel * m, gpus ) {
        QString key = OPENCL_GPU_REGISTRY_SETTINGS_GPU_SPECIFIC + 
            QString::number(m->getId()) + OPENCL_GPU_SETTINGS_ENABLED;
        QVariant enabled_v = s->getValue( key );
        if( !enabled_v.isNull() ) {
            m->setEnabled( enabled_v.toBool() );
        } else {
            m->setEnabled( true );
        }
    }
}

} //namespace
