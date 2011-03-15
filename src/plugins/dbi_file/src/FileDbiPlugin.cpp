/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#include "FileDbiPlugin.h"
#include "FileDbi.h"

#include <U2Core/Log.h>
#include <U2Core/AppContext.h>
#include <U2Core/U2OpStatusUtils.h>

namespace U2 {

extern "C" Q_DECL_EXPORT Plugin* U2_PLUGIN_INIT_FUNC() {
    FileDbiPlugin* plug = new FileDbiPlugin();
    return plug;
}

FileDbiPlugin::FileDbiPlugin() : Plugin(tr("Model 2.0 provider"), tr("Transitional interface from old datamodel to new interfaces.")) {
    AppContext::getDbiRegistry()->registerDbiFactory(new FileDbiFactory());
}

void FileDbiPlugin::test()
{
    FileDbi dbi;
    QHash<QString, QString> props;
    props["url"] = "D:/ugene/gb2/trunk/aaa.fa";
    props["url"] = "D:/ugene/gb2/trunk/data/samples/Genbank/murine.gb";
    props["url"] = "D:/ugene/gb2/trunk/data/samples/CLUSTALW/COI.aln";
    U2OpStatusImpl rc;
    dbi.init(props, QVariantMap(), rc);
    rc.setCanceled(false);
    rc.setError("");
    qint64 num = dbi.getObjectRDbi()->countObjects(rc);
    coreLog.info(QString("DBI obj count: %1 %2").arg(num).arg(rc.getError()));
    
    foreach(U2DataId id, dbi.getObjectRDbi()->getObjects(U2Type::Sequence, 0, -1, rc)) {
        U2DataType type = dbi.getEntityTypeById(id);
        coreLog.info(QString("DBI obj : %1 %2 %3").arg(id).arg(type).arg(rc.getError()));
        U2Sequence seq = dbi.getSequenceRDbi()->getSequenceObject(id, rc);
        coreLog.info(QString("Seq obj : %1 %2 %3").arg(seq.alphabet.id).arg(seq.length).arg(rc.getError()));
        rc.setError("");
        QByteArray data = dbi.getSequenceRDbi()->getSequenceData(id, U2Region(0, seq.length), rc);
        coreLog.info(QString("Seq data : %1 %2 %3").arg(QString(data)).arg(seq.dbiId).arg(rc.getError()));
        rc.setError("");
    }

    foreach(U2DataId id, dbi.getObjectRDbi()->getObjects(U2Type::Msa, 0, -1, rc)) {
        U2DataType type = dbi.getEntityTypeById(id);
        coreLog.info(QString("DBI obj : %1 %2 %3").arg(id).arg(type).arg(rc.getError()));
        U2Msa msa = dbi.getMsaRDbi()->getMsaObject(id, rc);
        coreLog.info(QString("MSA obj : %1 %2").arg(msa.alphabet.id).arg(rc.getError()));
        rc.setError("");
        foreach(U2MsaRow row, dbi.getMsaRDbi()->getRows(msa.id, 0, U2_DBI_NO_LIMIT, rc)) {
            U2Sequence seq = dbi.getSequenceRDbi()->getSequenceObject(row.sequenceId, rc);
            coreLog.info(QString("Seq obj : %1 %2 %3").arg(seq.alphabet.id).arg(seq.length).arg(rc.getError()));
            rc.setError("");
            QByteArray data = dbi.getSequenceRDbi()->getSequenceData(seq.id, U2Region(0, seq.length), rc);
            coreLog.info(QString("Seq data : %1 %2 %3").arg(QString(data)).arg(seq.dbiId).arg(rc.getError()));
            rc.setError("");
        }
        coreLog.info(QString("rc : %1 ").arg(rc.getError()));
        rc.setError("");

        /* TODO U2Assembly asmbl = dbi.getAssemblyRDbi()->getAssemblyObject(id, rc);
        coreLog.info(QString("Asm obj : %1 %2 %3").arg(asmbl.alphabet.id).arg(asmbl.id).arg(asmbl.readsCount).arg(rc._error));
        rc._error.clear();
        foreach(U2AssemblyRead row, dbi.getAssemblyRDbi()->getRows(msa.id, 0, msa.sequencesCount, rc)) {
            U2Sequence seq = dbi.getSequenceRDbi()->getSequenceObject(row.sequenceId, rc);
            coreLog.info(QString("Seq obj : %1 %2 %3").arg(seq.alphabet.id).arg(seq.length).arg(rc._error));
            rc._error.clear();
            QByteArray data = dbi.getSequenceRDbi()->getSequenceData(seq.id, U2Region(0, seq.length), rc);
            coreLog.info(QString("Seq data : %1 %2 %3").arg(QString(data)).arg(seq.dbiId).arg(rc._error));
            rc._error.clear();
        } 
        coreLog.info(QString("rc : %1 ").arg(rc._error)); */
    }

    dbi.shutdown(rc);
}
}//namespace
