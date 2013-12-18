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

#ifndef _U2_SEQUENCE_UTILS_H_
#define _U2_SEQUENCE_UTILS_H_

#include <U2Core/AnnotationData.h>
#include <U2Core/FormatSettings.h>
#include <U2Core/U2Sequence.h>
#include <U2Core/U2DbiUtils.h>

namespace U2 {

class Annotation;
class DNASequence;
class DNAQuality;
class DNATranslation;

/**                                           
    U2Sequence and related structures utility functions
*/
class U2CORE_EXPORT U2SequenceUtils : public QObject {
    Q_OBJECT
private:
    U2SequenceUtils(){}
public:

    /** Extract sequence parts marked by the regions
        Note: the order of complemented regions is also reversed
    */
    static QList<QByteArray> extractRegions(const U2EntityRef& seqRef, const QVector<U2Region>& regions, 
        DNATranslation* complTT, DNATranslation* aminoTT, bool join, U2OpStatus& os);

    
    static DNAAlphabetType alphabetType(const U2EntityRef& ref, U2OpStatus& os);

    static qint64 length(const U2EntityRef& ref, U2OpStatus& os);

    static U2Sequence copySequence(const DNASequence& srcSeq, const U2DbiRef& dstDbi, U2OpStatus& os);
    static U2Sequence copySequence(const U2EntityRef& srcSeq, const U2DbiRef& dstDbi, U2OpStatus& os);

    static U2EntityRef import(const U2DbiRef& dbiRef, const DNASequence& seq, U2OpStatus& os);

    static void setQuality(const U2EntityRef& entityRef, const DNAQuality& quality);
};

/** Class used to efficiently import sequence into DBI */
class U2CORE_EXPORT U2SequenceImporter {
public:
    U2SequenceImporter(const QVariantMap& fs = QVariantMap(), bool lazyMode = false, bool singleThread = true);
    U2SequenceImporter(qint64 _insertBlockSize, const QVariantMap& fs = QVariantMap(),
        bool lazyMode = false, bool singleThread = true);
    virtual ~U2SequenceImporter();
    
    void startSequence(const U2DbiRef& dbiRef, const QString& visualName, bool circular, U2OpStatus& os);
    virtual void addBlock(const char* data, qint64 len, U2OpStatus& os);
    void addSequenceBlock(const U2EntityRef& seqId, const U2Region& r, U2OpStatus& os);
    virtual void addDefaultSymbolsBlock(int n, U2OpStatus& os);
    U2Sequence finalizeSequence(U2OpStatus& os);
    U2AlphabetId getAlphabet() const {return sequence.alphabet;}

    void setCaseAnnotationsMode(CaseAnnotationsMode mode);
    bool isCaseAnnotationsModeOn() const;
    QList<AnnotationData> & getCaseAnnotations( );

    virtual qint64 getCurrentLength() const;

protected:
    void _addBlock2Buffer(const char* data, qint64 len, U2OpStatus& os);
    void _addBlock2Db(const char* data, qint64 len, U2OpStatus& os);
    void _addBuffer2Db(U2OpStatus& os);

    DbiConnection           con;
    qint64                  insertBlockSize;

    U2Sequence              sequence;
    QByteArray              sequenceBuffer;

    // for lower case annotations
    qint64                  currentLength;
    QList<AnnotationData>   annList;
    bool                    isUnfinishedRegion;
    U2Region                unfinishedRegion;
    CaseAnnotationsMode     caseAnnsMode;

    bool                    lazyMode;
    bool                    singleThread;

    bool                    sequenceCreated;
    qint64                  committedLength; // singleThread only

};


/** Class to read sequences when there is already readers which use U2SequenceImporter interface */
class U2CORE_EXPORT U2MemorySequenceImporter : public U2SequenceImporter {
public:
    U2MemorySequenceImporter(QByteArray& data) : sequenceData(data) {}
    virtual ~U2MemorySequenceImporter() {}

    virtual void addBlock(const char* data, qint64 len, U2OpStatus& os);
    virtual void addDefaultSymbolsBlock(int n, U2OpStatus& os);

    virtual qint64 getCurrentLength() const;

private:
    void startSequence(const U2DbiRef& dbiRef, const QString& visualName, bool circular, U2OpStatus& os);
    void addSequenceBlock(const U2EntityRef& seqId, const U2Region& r, U2OpStatus& os);
    U2Sequence finalizeSequence(U2OpStatus& os);
    void setCaseAnnotationsMode(CaseAnnotationsMode mode);
    bool isCaseAnnotationsModeOn() const;
    QList<Annotation*> &getCaseAnnotations();

    QByteArray &sequenceData;
};

class U2CORE_EXPORT U2SequenceDbiHints {
public:
    static const QString UPDATE_SEQUENCE_LENGTH;
    static const QString EMPTY_SEQUENCE;
};

} //namespace


#endif
