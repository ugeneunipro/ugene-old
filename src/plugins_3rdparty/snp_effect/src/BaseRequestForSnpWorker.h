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

#ifndef _U2_BASE_REQUEST_FOR_SNP_WORKER_
#define _U2_BASE_REQUEST_FOR_SNP_WORKER_

#include <U2Lang/LocalDomain.h>

#include "SnpRequestKeys.h"

namespace U2 {

class RequestForSnpTask;

namespace LocalWorkflow {

class BaseRequestForSnpWorker :     public BaseWorker
{
    Q_OBJECT
public:
                                    BaseRequestForSnpWorker( Actor *p );

    void                            init( );
    Task *                          tick( );

private slots:
    void                            sl_taskFinished( );

protected:
    virtual QVariantMap             getInputDataForRequest( ) = 0;
    virtual SnpRequestingScript     getRequestAddress( ) = 0;

    IntegralBus *                   inChannel;
    IntegralBus *                   outChannel;
};

} // namespace LocalWorkflow

} // namespace U2

#endif // _U2_BASE_REQUEST_FOR_SNP_WORKER_