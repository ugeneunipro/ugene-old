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

#ifndef _U2_DOCUMENT_FORMAT_UTILS_H_
#define _U2_DOCUMENT_FORMAT_UTILS_H_

#include <U2Core/U2Region.h>

#include <QtCore/QStringList>

namespace U2  {

class Document;
class DocumentFormat;
class DNAAlphabet;
class DNASequenceObject;
class GObject;
class MAlignment;
class DNASequence;
class GUrl;
class AnnotationSettings;
class U2OpStatus;

class U2FORMATS_EXPORT DocumentFormatUtils : public QObject {
    Q_OBJECT
public:
    static DNASequenceObject* addSequenceObject(QList<GObject*>& objects, const QString& name, DNASequence& seq, 
        const QVariantMap& hints, U2OpStatus& os);

    /** if no docURL provided -> relations are not set*/
    static DNASequenceObject* addMergedSequenceObject(QList<GObject*>& objects, const GUrl& docUrl, const QStringList& contigs, 
                            QByteArray& mergedSequence, const QVector<U2Region>& mergedMapping, const QVariantMap& hints, U2OpStatus& os);

    static DNAAlphabet* findAlphabet(const QByteArray& arr);
    
    static QList<DNAAlphabet*> findAlphabets(const QByteArray& arr);

    static DNAAlphabet* findAlphabet(const QByteArray& arr, const QVector<U2Region>& regionsToProcess);

    static void trySqueeze(QByteArray& a);

    static int getIntSettings(const QVariantMap& fs, const char* sName, int defVal);

    static void updateFormatSettings(QList<GObject*>& objects, QVariantMap& fs);

    static QList<DocumentFormatId> toIds(const QList<DocumentFormat*>& formats);

    static void assignAlphabet(MAlignment& ma);

    static void assignAlphabet(MAlignment& ma, char ignore);

    static QList<AnnotationSettings*> predefinedSettings();

    /** Extracts sequences either from Sequence or MAlignment object */
    static QList<DNASequence> toSequences(const GObject* obj);
};

}//namespace

#endif
