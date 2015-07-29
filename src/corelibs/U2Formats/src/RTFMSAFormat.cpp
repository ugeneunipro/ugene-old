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

#include <U2Algorithm/MSAColorScheme.h>

#include <U2Core/AppContext.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/L10n.h>
#include <U2Core/MAlignmentImporter.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/MAlignmentWalker.h>
#include <U2Core/MSAUtils.h>
#include <U2Core/Settings.h>
#include <U2Core/TextUtils.h>
#include <U2Core/U2AlphabetUtils.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2ObjectDbi.h>
#include <U2Core/U2OpStatus.h>
#include <U2Core/U2OpStatusUtils.h>

#include <U2Formats/DocumentFormatUtils.h>


#include "RTFMSAFormat.h"

namespace U2 {


RTFMSAFormat::RTFMSAFormat(QObject* p) : DocumentFormat(p, DocumentFormatFlags(DocumentFormatFlag_SupportWriting) | DocumentFormatFlag_OnlyOneObject | DocumentFormatFlag_Hidden| DocumentFormatFlag_RTFOutput, QStringList("rtf")) {
    formatName = tr("RTF");
    formatDescription = tr("Colored multiple alignments in RTF format.");
    supportedObjectTypes+=GObjectTypes::MULTIPLE_ALIGNMENT;
}

Document* RTFMSAFormat::loadDocument(IOAdapter* io, const U2DbiRef& dbiRef, const QVariantMap& fs, U2OpStatus& os){
    // the format is used for writing only
    QList<GObject*> objs;
    return new Document(this, io->getFactory(), io->getURL(), dbiRef, objs, fs);
}

void RTFMSAFormat::storeDocument(Document* d, IOAdapter* io, U2OpStatus& os) {
    CHECK_EXT(d!=NULL, os.setError(L10N::badArgument("doc")), );
    CHECK_EXT(io != NULL && io->isOpen(), os.setError(L10N::badArgument("IO adapter")), );

    MAlignmentObject* obj = NULL;
    if( (d->getObjects().size() != 1)
        || ((obj = qobject_cast<MAlignmentObject*>(d->getObjects().first())) == NULL)) {
            os.setError("No data to write;");
            return;
    }

    QList<GObject*> als; als << obj;
    QMap< GObjectType, QList<GObject*> > objectsMap;
    objectsMap[GObjectTypes::MULTIPLE_ALIGNMENT] = als;
    storeEntry(io, objectsMap, os);
    CHECK_EXT(!os.isCoR(), os.setError(L10N::errorWritingFile(d->getURL())), );
}

FormatCheckResult RTFMSAFormat::checkRawData(const QByteArray& rawData, const GUrl&) const {
    // the format is used for writing only
    return FormatDetection_NotMatched;
}

void RTFMSAFormat::storeEntry(IOAdapter *io, const QMap< GObjectType, QList<GObject*> > &objectsMap, U2OpStatus &ti) {
    SAFE_POINT(objectsMap.contains(GObjectTypes::MULTIPLE_ALIGNMENT), "RTFMSA entry storing: no alignment", );
    const QList<GObject*> &als = objectsMap[GObjectTypes::MULTIPLE_ALIGNMENT];
    SAFE_POINT(1 == als.size(), "RTFMSA entry storing: alignment objects count error", );

    MAlignmentObject* obj = dynamic_cast<MAlignmentObject*>(als.first());
    SAFE_POINT(NULL != obj, "RTFMSA entry storing: NULL alignment object", );

    const DNAAlphabet* al = obj->getAlphabet();
    if (!al){
        return;
    }
    Settings* s = AppContext::getSettings();
    SAFE_POINT(NULL != s, "RTFMSA entry storing: NULL settings object", );

    DNAAlphabetType atype = al->getType();
    MSAColorSchemeRegistry* csr = AppContext::getMSAColorSchemeRegistry();
        QString csid = atype == DNAAlphabet_AMINO ?
            s->getValue(MOBJECT_SETTINGS_ROOT + MOBJECT_SETTINGS_COLOR_AMINO, MSAColorScheme::UGENE_AMINO).toString()
          : s->getValue(MOBJECT_SETTINGS_ROOT + MOBJECT_SETTINGS_COLOR_NUCL, MSAColorScheme::UGENE_NUCL).toString();

    MSAColorSchemeFactory* csf = csr->getMSAColorSchemeFactoryById(csid);
    if (csf == NULL) {
        csf = csr->getMSAColorSchemeFactoryById(atype == DNAAlphabet_AMINO ? MSAColorScheme::UGENE_AMINO : MSAColorScheme::UGENE_NUCL);
    }
    SAFE_POINT(NULL != (csf!=NULL), "RTFMSA entry storing: NULL MSAColorSchemeFactory object", );
    MSAColorScheme* colorScheme = csf->create(this, obj);

    QString fontFamily = s->getValue(MOBJECT_SETTINGS_ROOT + MOBJECT_SETTINGS_FONT_FAMILY, MOBJECT_DEFAULT_FONT_FAMILY).toString();
    int pointSize = s->getValue(MOBJECT_SETTINGS_ROOT + MOBJECT_SETTINGS_FONT_SIZE, MOBJECT_DEFAULT_FONT_SIZE).toInt();

    io->writeBlock(QString("<span style=\"font-size:%1pt; font-family:%2;\">\n").arg(pointSize).arg(fontFamily).toLatin1());
        const MAlignment& ma = obj->getMAlignment();
        int numRows = ma.getNumRows();
        for (int i = 0; i < numRows; i++){
            io->writeBlock("<p>");
                QString res;
                const MAlignmentRow& row = ma.getRow(i);
                QByteArray rowData = row.getData();
                int dataLen = rowData.length();
                for (int j = 0; j < dataLen; j++){
                    QColor color = colorScheme->getColor(i, j, rowData[j]);
                    //QColor color = QColor("red");
                    if (color.isValid()){
                        res.append(QString("<span style=\"background-color:%1;\">%2</span>").arg(color.name()).arg(rowData[j]));
                    }else{
                        res.append(QString("%1").arg(rowData[j]));
                    }
                }
                io->writeBlock(res.toLatin1());
            io->writeBlock("</p>\n");
        }
    io->writeBlock("</span>");

    delete colorScheme;
}

} //namespace
