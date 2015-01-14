/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#ifndef _U2_ANALYZE_TATA_BOXES_WORKER_
#define _U2_ANALYZE_TATA_BOXES_WORKER_

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>

#include "../BaseRequestForSnpWorker.h"

namespace U2 {

namespace LocalWorkflow {

class AnalyzeTataBoxesWorker : public BaseRequestForSnpWorker
{
    Q_OBJECT
public:
                               AnalyzeTataBoxesWorker( Actor *p );

protected:
    QList<Task *>              createVariationProcessingTasks( const U2Variant &var,
                                   const U2VariantTrack &track, U2Dbi *dbi );
    virtual QList<SnpResponseKey>   getResultKeys( ) const;
};

/************************************************************************/
/* Factory */
/************************************************************************/

class AnalyzeTataBoxesWorkerFactory :   public DomainFactory
{
public:
                                        AnalyzeTataBoxesWorkerFactory( );

    static void                         init( );
    Worker *                            createWorker( Actor *a );

private:
    static const QString                ACTOR_ID;
};

/************************************************************************/
/* Prompter */
/************************************************************************/

class AnalyzeTataBoxesPrompter : public PrompterBase<AnalyzeTataBoxesPrompter>
{
    Q_OBJECT
public:
                                 AnalyzeTataBoxesPrompter( Actor *p = NULL );

protected:
    QString                      composeRichDoc( );
};

} // namespace LocalWorkflow

} // namespace U2

#endif // _U2_BASE_REQUEST_FOR_SNP_WORKER_
