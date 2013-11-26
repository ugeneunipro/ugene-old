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
#include "snp_toolbox/VariationInfo.h"

#include <U2Formats/SNPDatabaseUtils.h>

#include <U2Core/U2SafePoints.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>

#include <QtCore/QScopedPointer>

namespace U2 {

SNPReportWriterTask::SNPReportWriterTask(const SNPReportWriterSettings& _settings, const QList<U2VariantTrack>& _tracks, const U2DbiRef& _dbiRef)
: Task("Generating and writing an SNP report", TaskFlag_None)
, settings(_settings)
, tracks(_tracks)
, dbiRef(_dbiRef)
{
    tpm = Progress_Manual;
}

void SNPReportWriterTask::run() {
    if (isCanceled() || hasError() || tracks.isEmpty() || !dbiRef.isValid()){
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

    QSharedPointer<DamageEffectEvaluator> deEval(new DamageEffectEvaluator(sessionDbi, databaseDbi));

    stateInfo.setProgress(0);
    U2VariantDbi* varDbi = sessionDbi->getVariantDbi();
    if(varDbi == NULL){
        setError(tr("Variant dbi is Null"));
        return;
    }

    U2ObjectDbi* objectDbi = databaseDbi->getObjectDbi();
    if(objectDbi == NULL){
        setError(tr("Object dbi is Null"));
        return;
    }

    U2SequenceDbi* sequenceDbi = databaseDbi->getSequenceDbi();
    if(sequenceDbi == NULL){
        setError(tr("Sequence dbi is Null"));
        return;
    }

    U2AttributeDbi* attrDbi = sessionDbi->getAttributeDbi();
    if(attrDbi == NULL){
        setError(tr("Attribute dbi is Null"));
        return;
    }

    qint64 variationsNumber = 0;
    foreach (const U2VariantTrack& track, tracks){
        variationsNumber+=varDbi->getVariantCount(track.id, stateInfo);
    }

    qint64 curNumber = 0;

    //in-gene
    IOAdapter* io = IOAdapterUtils::open(settings.reportPath, stateInfo, IOAdapterMode_Write);
    bool first = true;

    foreach (const U2VariantTrack& track, tracks){
        QScopedPointer<U2DbiIterator<U2Variant> > snpIter( varDbi->getVariants(track.id, U2_REGION_MAX, stateInfo));
        if (stateInfo.isCoR()){
            return;
        }

        while(snpIter->hasNext()){
            const U2Variant& var = snpIter->next();
            U2DataId seqID = track.sequence.isEmpty() ? SNPDatabaseUtils::getSequenceId(track.sequenceName, objectDbi) : track.sequence;
            //for in-gene only
            if (!deEval->isInGene(var, seqID)){
                continue;
            }
            curNumber++;
            stateInfo.progress = curNumber*100 / (float)variationsNumber;
            VariationInfo varReport(var, seqID, sequenceDbi, attrDbi, track.sequenceName);
            if (first){
                QString header = varReport.getInGeneTableHeader();
                io->writeBlock(header.toLatin1());
                first = false;
                outPaths << settings.reportPath;
            }
            varReport.initInfo(varDbi, deEval, true);
            QStringList raws = varReport.getInGeneTableRaws();
            clearDuplicates(raws);
            foreach(const QString& raw, raws){
                io->writeBlock(raw.toLatin1());
                io->writeBlock("\n");
            }
        }
    }
    io->close();

    stateInfo.setProgress(50);

    //regulatory
    IOAdapter* ioReg = IOAdapterUtils::open(settings.regulatoryReportPath, stateInfo, IOAdapterMode_Write);
    first = true;
    foreach (const U2VariantTrack& track, tracks){
        QScopedPointer<U2DbiIterator<U2Variant> > snpIter( varDbi->getVariants(track.id, U2_REGION_MAX, stateInfo));
        if (stateInfo.isCoR()){
            return;
        }
        while(snpIter->hasNext()){
            const U2Variant& var = snpIter->next();
            U2DataId seqID = track.sequence.isEmpty() ? SNPDatabaseUtils::getSequenceId(track.sequenceName, objectDbi) : track.sequence;
            //for regulatory only
            if (deEval->isInGene(var, seqID)){
                continue;
            }
            curNumber++;
            stateInfo.progress = curNumber*100 / (float)variationsNumber;
            VariationInfo varReport(var, seqID, sequenceDbi, attrDbi, track.sequenceName);
            if (first){
                QString header = varReport.getOutGeneTableHeader();
                ioReg->writeBlock(header.toLatin1());
                first = false;
                outPaths << settings.regulatoryReportPath;
            }
            varReport.initInfo(varDbi, deEval, true);
            QStringList raws = varReport.getOutGeneTableRaws();
            clearDuplicates(raws);
            foreach(const QString& raw, raws){
                ioReg->writeBlock(raw.toLatin1());
                ioReg->writeBlock("\n");
            }
        }
    }
    ioReg->close();

    stateInfo.setProgress(100);
}

void SNPReportWriterTask::clearDuplicates( QStringList& raws ){
    
    for (int i = 0; i < raws.size()-1;) {
        const QString& r0 = raws[i];
        const QString& r1 = raws[i+1];
        if (r0 != r1){
            i++;
            continue;
        }
        raws.takeAt(i+1);
    }
}

} // U2

