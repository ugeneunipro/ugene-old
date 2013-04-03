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

#include <U2Core/IOAdapter.h>
#include <U2Core/L10n.h>
#include <U2Core/TextUtils.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2ObjectDbi.h>
#include <U2Core/U2OpStatus.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2Type.h>
#include <U2Core/U2Variant.h>
#include <U2Core/U2VariantDbi.h>
#include <U2Core/VariantTrackObject.h>

#include "AbstractVariationFormat.h"

namespace U2 {

const QString AbstractVariationFormat::COMMENT_START("#");

AbstractVariationFormat::AbstractVariationFormat(QObject *p, const QStringList &fileExts)
: DocumentFormat(p, DocumentFormatFlags_SW, fileExts), sep(QString())
{
    supportedObjectTypes += GObjectTypes::VARIANT_TRACK;
    formatDescription = tr("SNP formats are used to store single-nucleotide polymorphism data");
    indexing = AbstractVariationFormat::ZeroBased;
}

#define READ_BUFF_SIZE 4095
#define CHR_PREFIX "chr"

Document *AbstractVariationFormat::loadDocument(IOAdapter *io, const U2DbiRef &dbiRef, const QVariantMap &fs, U2OpStatus &os) {
    DbiConnection con(dbiRef, os);
    SAFE_POINT_OP(os, NULL);
    U2Dbi *dbi = con.dbi;

    SAFE_POINT(dbi->getVariantDbi() , "Variant DBI is NULL!", NULL);
    SAFE_POINT(io, "IO adapter is NULL!",  NULL);
    SAFE_POINT(io->isOpen(), QString("IO adapter is not open %1").arg(io->getURL().getURLString()), NULL);

    QByteArray readBuff(READ_BUFF_SIZE + 1, 0);
    char* buff = readBuff.data();

    //TODO: load snps with chunks of fixed size to avoid memory consumption
    QMap<QString, QList<U2Variant> > snpsMap;

    do {
        bool lineOk = true;
        os.setProgress(io->getProgress());
        qint64 len = io->readUntil(buff, READ_BUFF_SIZE, TextUtils::LINE_BREAKS, IOAdapter::Term_Include, &lineOk);
        if (len == 0) { //end of stream
            break;
        }
        if (!lineOk) {
            os.setError(L10N::tr("Line is too long"));
            return NULL;
        }

        // skip comments
        if (readBuff.startsWith('#')) {
            continue;
        }

        QString line(readBuff);
        QStringList columns = sep.isEmpty() ? line.split(QRegExp("\\s")) : line.split(sep);

        if (columns.size() < maxColumnNumber) {
            continue;
        }

        U2Variant v;
        QString seqName;

        foreach (int columnNumber, columnRoles.keys()) {
            assert(columns.size() > columnNumber);
            const QString& columnData = columns.at(columnNumber);
            ColumnRole role = columnRoles.value(columnNumber);
            switch (role) {
                case ColumnRole_ChromosomeId:
                    seqName = columnData;
                    break;
                case ColumnRole_StartPos:
                    v.startPos = columnData.toInt();
                    if (indexing == AbstractVariationFormat::OneBased){
                        v.startPos -= 1;
                    }
                    break;
                case ColumnRole_EndPos:
                    v.endPos = columnData.toInt();
                    if (indexing == AbstractVariationFormat::OneBased){
                        v.startPos -= 1;
                    }
                    break;
                case ColumnRole_RefData:
                    v.refData = columnData.toLatin1();
                    break;
                case ColumnRole_ObsData:
                    v.obsData = columnData.toLatin1();
                    break;
                case ColumnRole_PublicId:
                    v.publicId = columnData.toLatin1();
                    break;
                default:
                    coreLog.trace("Warning: unknown column role (%, line %, column %)");
                    break;
            }
        }
        
        if (v.publicId.isEmpty()) {
            QString prefix = seqName.contains(CHR_PREFIX) ? seqName : seqName.prepend(CHR_PREFIX);
            v.publicId = QString("%1v%2").arg(prefix).arg(snpsMap[seqName].count() + 1).toLatin1();
        }

        snpsMap[seqName].append(v);

    } while (!io->isEof());

    QList<GObject*> objects;
    QSet<QString> names;

    foreach (const QString &seqName, snpsMap.keys().toSet()) {
        U2VariantTrack track;
        track.sequenceName = seqName;
        dbi->getVariantDbi()->createVariantTrack(track, "", os);

        const QList<U2Variant>& vars = snpsMap.value(seqName);
        BufferedDbiIterator<U2Variant> bufIter(vars);
        dbi->getVariantDbi()->addVariantsToTrack(track, &bufIter, os);

        U2EntityRef trackRef(dbiRef, track.id);
        QString objName = TextUtils::variate(track.sequenceName, "_", names);
        names.insert(objName);
        VariantTrackObject *trackObj = new VariantTrackObject(objName, trackRef);
        objects << trackObj;
    }

    QString lockReason;
    Document* doc = new Document(this, io->getFactory(), io->getURL(), dbiRef, objects, fs, lockReason);
    return doc;
}

FormatCheckResult AbstractVariationFormat::checkRawData(const QByteArray &dataPrefix, const GUrl &) const {
    QStringList lines = QString(dataPrefix).split("\n");
    int idx = 0;
    foreach (const QString &l, lines) {
        bool lastLine = (idx == lines.size()-1);
        if (lastLine) {
            continue;
        }

        QString line = l.simplified();
        idx++;
        if (line.startsWith(COMMENT_START)) {
            continue;
        }

        QStringList cols = line.split(QRegExp("\\s"), QString::SkipEmptyParts);
        if (!this->checkFormatByColumnCount(cols.size())) {
            return FormatDetection_NotMatched;
        }
    }
    if (0 == idx) {
        return FormatDetection_NotMatched;
    }
    return FormatDetection_AverageSimilarity;
}

void AbstractVariationFormat::storeDocument(Document *doc, IOAdapter *io, U2OpStatus &os) {
    foreach (GObject *obj, doc->getObjects()) {
        if (GObjectTypes::VARIANT_TRACK != obj->getGObjectType()) {
            continue;
        }
        VariantTrackObject *trackObj = dynamic_cast<VariantTrackObject*>(obj);
        storeTrack(io, trackObj, os);
    }
}

void AbstractVariationFormat::storeEntry(IOAdapter *io, const QMap< GObjectType, QList<GObject*> > &objectsMap, U2OpStatus &os) {
    SAFE_POINT(objectsMap.contains(GObjectTypes::VARIANT_TRACK), "Variation entry storing: no variations", );
    const QList<GObject*> &vars = objectsMap[GObjectTypes::VARIANT_TRACK];
    SAFE_POINT(1 == vars.size(), "Variation entry storing: variation objects count error", );

    VariantTrackObject *trackObj = dynamic_cast<VariantTrackObject*>(vars.first());
    SAFE_POINT(NULL != trackObj, "Variation entry storing: NULL variation object", );

    storeTrack(io, trackObj, os);
}

void AbstractVariationFormat::storeTrack(IOAdapter *io, const VariantTrackObject *trackObj, U2OpStatus &os) {
    CHECK(NULL != trackObj, );
    U2VariantTrack track = trackObj->getVariantTrack(os);
    CHECK_OP(os, );
    QScopedPointer< U2DbiIterator<U2Variant> > varsIter(trackObj->getVariants(U2_REGION_MAX, os));
    CHECK_OP(os, );

    QByteArray snpString;
    while (varsIter->hasNext()){
        U2Variant variant = varsIter->next();

        snpString.clear();
        bool first = true;
        foreach (int columnNumber, columnRoles.keys()) {
            if (first) {
                first = false;
            } else {
                snpString += "\t";
            }
            ColumnRole role = columnRoles.value(columnNumber);
            switch (role) {
                    case ColumnRole_ChromosomeId:
                        snpString += track.sequenceName;
                        break;
                    case ColumnRole_StartPos:
                        if (indexing == AbstractVariationFormat::OneBased){
                            snpString += QByteArray::number(variant.startPos + 1);
                        }else if (indexing == AbstractVariationFormat::ZeroBased){
                            snpString += QByteArray::number(variant.startPos);
                        }else{
                            assert(0);
                        }
                        
                        break;
                    case ColumnRole_EndPos:
                        if (indexing == AbstractVariationFormat::OneBased){
                            snpString += QByteArray::number(variant.endPos + 1);
                        }else if (indexing == AbstractVariationFormat::ZeroBased){
                            snpString += QByteArray::number(variant.endPos);
                        }else{
                            assert(0);
                        }
                        break;
                    case ColumnRole_RefData:
                        snpString += variant.refData;
                        break;
                    case ColumnRole_ObsData:
                        snpString += variant.obsData;
                        break;
                    case ColumnRole_PublicId:
                        snpString += variant.publicId;
                        break;
                    default:
                        coreLog.trace("Warning: unknown column role (%, line %, column %)");
                        break;
            }
        }
        snpString += "\n";
        io->writeBlock(snpString);
    }
}

} // U2
