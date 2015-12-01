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

#ifndef _U2_DOCUMENT_FORMAT_UTILS_H_
#define _U2_DOCUMENT_FORMAT_UTILS_H_

#include <U2Core/DocumentModel.h>
#include <U2Core/U2Region.h>

#include <QtCore/QStringList>

namespace U2 {

class AnnotationSettings;
class AnnotationTableObject;
class DNAAlphabet;
class DNASequence;
class Document;
class GObject;
class GObjectReference;
class GUrl;
class MAlignment;
class U2DbiRef;
class U2OpStatus;
class U2Sequence;
class U2SequenceObject;

class U2FORMATS_EXPORT DocumentFormatUtils : public QObject {
    Q_OBJECT
public:
    static QList<DocumentFormatId> toIds(const QList<DocumentFormat*>& formats);

    static QList<AnnotationSettings*> predefinedSettings();

    /** Extracts sequences either from Sequence or MAlignment object */
    static QList<DNASequence> toSequences(const GObject* obj);

    static int getMergeGap(const QVariantMap& hints);

    static int getMergedSize(const QVariantMap& hints, int defaultVal);

    static void updateFormatHints(QList<GObject*>& objects, QVariantMap& fs);

    static U2SequenceObject* addSequenceObject(const U2DbiRef& dbiRef, const QString& name, const QByteArray& seq, bool circular, const QVariantMap& hints, U2OpStatus& os);

    /** Doc URL here is used to set up sequence<->annotation relations */
    static AnnotationTableObject * addAnnotationsForMergedU2Sequence(const GObjectReference& mergedSequenceRef,
                                                                     const U2DbiRef& dbiRef,
                                                                     const QStringList& contigs,
                                                                     const QVector<U2Region>& mergedMapping,
                                                                     const QVariantMap &hints);

    static U2SequenceObject* addSequenceObjectDeprecated(const U2DbiRef& dbiRef,
                                                         const QString &folder,
                                                         const QString& seqObjName,
                                                         QList<GObject*>& objects,
                                                         DNASequence& seq,
                                                         U2OpStatus& os);

    /** if no docURL provided -> relations are not set*/
    static U2SequenceObject* addMergedSequenceObjectDeprecated(const U2DbiRef& dbiRef,
                                                               const QString &folder,
                                                               QList<GObject*>& objects,
                                                               const GUrl& docUrl,
                                                               const QStringList& contigs,
                                                               QByteArray& mergedSequence,
                                                               const QVector<U2Region>& mergedMapping,
                                                               U2OpStatus& os);
};

}//namespace

#endif
