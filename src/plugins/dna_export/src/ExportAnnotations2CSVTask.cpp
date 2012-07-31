/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#include "ExportAnnotations2CSVTask.h"

#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/AppContext.h>
#include <U2Core/Counter.h>
#include <U2Core/L10n.h>
#include <U2Core/TextUtils.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/U2SafePoints.h>

#include <memory>

namespace U2 {

ExportAnnotations2CSVTask::ExportAnnotations2CSVTask(const QList<Annotation*>& annotations, 
                                                     const QByteArray& sequence,
                                                     const QString& _seqName,
                                                     DNATranslation *complementTranslation,
                                                     bool exportSequence,
                                                     bool _exportSeqName,
                                                     const QString& url, 
                                                     bool apnd,
                                                     QString sep)
: Task(tr("Export2CSV"), TaskFlag_None),
    annotations(annotations),
    sequence(sequence),
    seqName(_seqName),
    complementTranslation(complementTranslation),
    exportSequence(exportSequence),
    exportSequenceName(_exportSeqName),
    url(url),
    append(apnd),
    separator(sep)

{
    GCOUNTER( cvar, tvar, "ExportAnnotattions2CSVTask" );
}

static void writeCSVLine(const QStringList& container, IOAdapter *ioAdapter, const QString& separator, U2OpStatus& os) {
    bool first = true;
    foreach(QString value, container) {
        if (!first) {
            if (0 == ioAdapter->writeBlock(separator.toAscii())) {
                os.setError(L10N::errorWritingFile(ioAdapter->getURL()));
                return;
            }
        }
        QString preparedStr = "\"" + value.replace("\"","\"\"") + "\"";
        if (0 == ioAdapter->writeBlock(preparedStr.toLocal8Bit())) {
            os.setError(L10N::errorWritingFile(ioAdapter->getURL()));
            return;
        }
        first = false;
    }
    if (0 == ioAdapter->writeBlock("\n")) {
        os.setError(L10N::errorWritingFile(ioAdapter->getURL()));
        return;
    }
}

void ExportAnnotations2CSVTask::run() {
    std::auto_ptr<IOAdapter> ioAdapter;

    IOAdapterId ioAdapterId = IOAdapterUtils::url2io(url);
    IOAdapterFactory *ioAdapterFactory = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(ioAdapterId);
    CHECK_EXT (ioAdapterFactory != NULL, stateInfo.setError(tr("No IO adapter found for URL: %1").arg(url)), );
    ioAdapter.reset(ioAdapterFactory->createIOAdapter());

    if (!ioAdapter->open(url, append ? IOAdapterMode_Append : IOAdapterMode_Write)) {
        stateInfo.setError(L10N::errorOpeningFileWrite(url));
        return;
    }

    QHash<QString, int> columnIndices;
    QStringList columnNames;
    columnNames << tr("Group") << tr("Name") << tr("Start") <<  tr("End")  << tr("Length") << tr("Complementary");
    if (exportSequenceName) {
        columnNames << tr("Sequence name");
    }
    if (exportSequence) {
        columnNames << tr("Sequence");
    }
    foreach(const Annotation* annotation, annotations) {
        foreach(const U2Qualifier& qualifier, annotation->getQualifiers()) {
            const QString& qName = qualifier.name;
            if (!columnIndices.contains(qName)) {
                columnIndices.insert(qName, columnNames.size());
                columnNames.append(qName);
            }
        }
    }
    writeCSVLine(columnNames, ioAdapter.get(), separator, stateInfo);
    CHECK_OP(stateInfo, );

    foreach(const Annotation* annotation, annotations) {
        foreach(const U2Region& region, annotation->getRegions()) {
            QStringList values;
            values << annotation->getGroups().last()->getGroupPath();
            values << annotation->getAnnotationName();
            values << QString::number(region.startPos + 1);
            values << QString::number(region.startPos + region.length);
            values << QString::number(region.length);
            if (annotation->getStrand().isCompementary()) {
                values << tr("yes");
            } else {
                values << tr("no");
            }
            if(exportSequenceName){
                values << seqName.toAscii();
            }
            if (exportSequence) {
                QByteArray sequencePart = sequence.mid(region.startPos, region.length);
                if (annotation->getStrand().isCompementary()) {
                    complementTranslation->translate(sequencePart.data(), sequencePart.size());
                    TextUtils::reverse(sequencePart.data(), sequencePart.size());
                }
                values << sequencePart;
            }
            
            //add empty strings as default qualifier values
            while (values.size() < columnNames.size()) {
                values << QString();
            }

            foreach(const U2Qualifier& qualifier, annotation->getQualifiers()) {
                int qualifiedIndex = columnIndices[qualifier.name];
                SAFE_POINT(qualifiedIndex > 0 && qualifiedIndex < values.length(), "Invalid qualifier index", );
                values[qualifiedIndex] = qualifier.value;
            }
            writeCSVLine(values, ioAdapter.get(), separator, stateInfo);
            CHECK_OP(stateInfo, );
        }
    }
}

} // namespace U2
