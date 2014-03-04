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

#ifndef _U2_PROT_STABILITY_3D_WORKER_
#define _U2_PROT_STABILITY_3D_WORKER_

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>

#include "../BaseRequestForSnpWorker.h"

namespace U2 {

namespace LocalWorkflow {

/************************************************************************/
/* Worker */
/************************************************************************/

class ProtStability3DWorker : public BaseRequestForSnpWorker
{
    Q_OBJECT
public:
                              ProtStability3DWorker( Actor *p );

protected:
    QList< QVariantMap >      getInputDataForRequest( const U2Variant& variant,
                                const U2VariantTrack& track, U2Dbi* dataBase );
    QString                   getRequestingScriptName( ) const;
    QList<SnpResponseKey>     getResultKeys( ) const;
};

/************************************************************************/
/* Factory */
/************************************************************************/

class ProtStability3DWorkerFactory : public DomainFactory
{
public:
                                     ProtStability3DWorkerFactory( );

    static void                      init( );
    Worker *                         createWorker( Actor *a );

private:
    static const QString             ACTOR_ID;
};

/************************************************************************/
/* Prompter */
/************************************************************************/

class ProtStability3DPrompter : public PrompterBase<ProtStability3DPrompter>
{
    Q_OBJECT
public:
                                ProtStability3DPrompter( Actor *p = NULL );

protected:
    QString                     composeRichDoc( );
};

} // namespace LocalWorkflow

} // namespace U2

#endif // _U2_PROT_STABILITY_3D_WORKER_
