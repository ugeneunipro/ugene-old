/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#ifndef __CUDA_SUPPORT_PLUGIN_H__
#define __CUDA_SUPPORT_PLUGIN_H__

#include <cuda.h>
#include <U2Core/PluginModel.h>
#include <U2Algorithm/CudaGpuRegistry.h>

namespace U2 {

#define ULOG_CAT_PLUGIN_CUDA_SUPPORT "Plugin: CudaSupport"

class CudaSupportPlugin : public Plugin {
    Q_OBJECT
public:
    enum Error {
        Error_NoError,
        Error_NoDriverLib,
        Error_BadDriverLib,
        Error_CudaError
    };

    CudaSupportPlugin();
private:
    Error obtainGpusInfo( QString & err );
    void loadGpusSettings();
    void registerAvailableGpus();

    static QString getCudaErrorString( CUresult code );
    static QString getSettingsErrorString( Error err );

    QList<CudaGpuModel *> gpus;
};

}

#endif //__CUDA_SUPPORT_PLUGIN_H__
