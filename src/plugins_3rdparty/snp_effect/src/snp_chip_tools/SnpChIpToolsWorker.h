/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#ifndef _U2_SNP_CHIP_TOOLS_WORKER_
#define _U2_SNP_CHIP_TOOLS_WORKER_

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>

#include "../BaseRequestForSnpWorker.h"

namespace U2 {

namespace LocalWorkflow {

/************************************************************************/
/* Worker */
/************************************************************************/

class SnpChipToolsWorker :   public BaseRequestForSnpWorker
{
    Q_OBJECT
public:
                                SnpChipToolsWorker( Actor *p );

protected:
    QList<QVariantMap>          getInputDataForRequest( const U2Variant &variant,
                                    const U2VariantTrack &track, U2Dbi *dataBase );

    QString                     getRequestingScriptName( ) const;
    QList<SnpResponseKey>       getResultKeys( ) const;
};

/************************************************************************/
/* Factory */
/************************************************************************/

class SnpChipToolsWorkerFactory :    public DomainFactory
{
public:
                                        SnpChipToolsWorkerFactory( );

    static void                         init( );
    virtual Worker *                    createWorker( Actor *a );

private:
    static const QString                ACTOR_ID;
};

/************************************************************************/
/* Prompter */
/************************************************************************/

class SnpChipToolsPrompter : public PrompterBase<SnpChipToolsPrompter>
{
    Q_OBJECT
public:
                                SnpChipToolsPrompter( Actor *p = NULL );

protected:
    QString                     composeRichDoc( );
};

} // namespace LocalWorkflow

} // namespace U2

#endif // _U2_SNP_CHIP_TOOLS_WORKER_
