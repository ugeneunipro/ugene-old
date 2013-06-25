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

#include "SNPReportWriterTask.h"

#include <U2Formats/S3DatabaseUtils.h>
#include <U2Core/U2SafePoints.h>

#include <QtCore/QScopedPointer>

namespace U2 {

SNPReportWriterTask::SNPReportWriterTask(const SNPReportWriterSettings& _settings, const QList<U2VariantTrack>& _tracks, const U2DbiRef& _dbiRef)
: Task("Generating and writing an SNP report", TaskFlag_None)
, settings(_settings)
, dbiRef(_dbiRef)
, tracks(_tracks)
{

}

void SNPReportWriterTask::run() {
    if (isCanceled() || hasError() || !tracks.isEmpty() || !dbiRef.isValid()){
        return;
    }

    DbiConnection* sessionHandle = new DbiConnection(dbiRef, stateInfo);
    CHECK_OP(stateInfo, );
    QScopedPointer<DbiConnection> session(sessionHandle);
    U2Dbi* sessionDbi = session->dbi;
    if(sessionDbi == NULL){
        setError(tr("Dbi Session is null"));
        return;
    }    

    stateInfo.setProgress(0);
    

    stateInfo.setProgress(100);
}

} // U2
