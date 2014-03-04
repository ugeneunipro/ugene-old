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

#include "SNPToolboxTask.h"
#include "DamageEffectEvaluator.h"

#include <U2Formats/SNPDatabaseUtils.h>
#include <U2Core/U2SafePoints.h>

#include <QtCore/QScopedPointer>

namespace U2 {

SNPToolboxTask::SNPToolboxTask(const SNPToolboxSettings& _settings, const U2VariantTrack& _track, const U2DbiRef& _dbiRef)
: Task("SNPToolbox variations annotation", TaskFlag_None)
, settings(_settings)
, track(_track)
, dbiRef(_dbiRef)
{

}

void SNPToolboxTask::run() {
    if (isCanceled() || hasError() || !track.hasValidId() || !dbiRef.isValid()){
        return;
    }

    //database
    QScopedPointer<Database> database (SNPDatabaseUtils::openDatabase(settings.dbPath));
    if (database.isNull()){
        setError(tr("Cannot open database file: %1").arg(settings.dbPath));
        return;
    }
    U2Dbi* databaseDbi = database->getDbi().dbi;

    //session
    DbiConnection* sessionHandle = new DbiConnection(dbiRef, stateInfo);
    CHECK_OP(stateInfo, );
    QScopedPointer<DbiConnection> session(sessionHandle);
    U2Dbi* sessionDbi = session->dbi;
    if(sessionDbi == NULL){
        setError(tr("Dbi Session is null"));
        return;
    }    

    stateInfo.setProgress(0);

    DbiOperationsBlock opBlock (databaseDbi->getDbiRef(), stateInfo);
    DbiOperationsBlock opBlock2 (sessionDbi->getDbiRef(), stateInfo);

    DamageEffectEvaluator deEval(sessionDbi, databaseDbi);

    deEval.calcDamageEffectForTracks(QList<U2VariantTrack>() << track, DamageEffectFull, stateInfo);

    stateInfo.setProgress(100);
}

} // U2
