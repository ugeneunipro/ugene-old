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

#ifndef _U2_EMBL_GENBANK_ABSTRACT_DOCUMENT_H_
#define _U2_EMBL_GENBANK_ABSTRACT_DOCUMENT_H_

#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentModel.h>

#include <QtCore/QStringList>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/U2SequenceUtils.h>


namespace U2 {

class IOAdapter;
class ParserState;
class EMBLGenbankDataEntry;


class U2FORMATS_EXPORT EMBLGenbankAbstractDocument : public DocumentFormat {
    Q_OBJECT
public:
    EMBLGenbankAbstractDocument(const DocumentFormatId& id, const QString& formatName, 
                                int maxLineSize, DocumentFormatFlags flags, QObject* p);

    virtual DocumentFormatId getFormatId() const {return id;}

    virtual const QString& getFormatName() const {return formatName;}

    static const QString UGENE_MARK;
    static const QString DEFAULT_OBJ_NAME;
    static const QString LOCUS_TAG_CIRCULAR;

    // move to utils??
    static QString	genObjectName(QSet<QString>& usedNames, const QString& name, const QVariantMap& info, int n, const GObjectType& t);

protected:
    virtual Document* loadDocument(IOAdapter* io, const U2DbiRef& dbiRef, const QVariantMap& fs, U2OpStatus& os);

    virtual void load(const U2DbiRef& dbiRef, IOAdapter* io, QList<GObject*>& objects, QVariantMap& fs, U2OpStatus& si, QString& writeLockReason);
    
    virtual int     readMultilineQualifier(IOAdapter* io, char* cbuff, int maxSize, bool prevLineHasMaxSize, int lenFirstQualLine);
    virtual SharedAnnotationData readAnnotation(IOAdapter* io, char* cbuff, int contentLen, int bufSize, U2OpStatus& si, int offset, int seqLen = -1);
    virtual bool    readSequence(ParserState*, U2SequenceImporter& , int&, int&, U2OpStatus&);

    virtual bool readEntry(ParserState*, U2SequenceImporter& ,int& seqSize,int& fullSeqSize,bool merge, int gapSize,U2OpStatus&) = 0;	
    virtual void readAnnotations(ParserState*, int offset);
    virtual void readHeaderAttributes(QVariantMap& tags, DbiConnection& con, U2SequenceObject* so) {
        Q_UNUSED(tags); Q_UNUSED(con); Q_UNUSED(so);
    } // does nothing if not overloaded
    
    DocumentFormatId id;
    QString     formatName;
    QByteArray  fPrefix;
    QByteArray  sequenceStartPrefix;
    int         maxAnnotationLineLen;
};

//////////////////////////////////////////////////////////////////////////
// header model

class EMBLGenbankDataEntry {
public:
    EMBLGenbankDataEntry() : seqLen(0), hasAnnotationObjectFlag(false), circular(false) {}
     /** locus name */
    QString name;

    /** sequence len*/
    int	seqLen;

    QVariantMap tags;
    QList<SharedAnnotationData> features;

    // hasAnnotationObjectFlag parameter is used to indicate that 
    // annotation table object must present even if result list is empty
    bool hasAnnotationObjectFlag;
    bool circular;
};

class ParserState {
public:
    ParserState(int off, IOAdapter* io, EMBLGenbankDataEntry* e, U2OpStatus& si)
        : valOffset(off), entry(e), io(io), buff(NULL), len(0), si(si) {}
    const int valOffset;
    EMBLGenbankDataEntry* entry;
    IOAdapter* io;
    static const int READ_BUFF_SIZE = 8192;
    char* buff;
    int len;
    U2OpStatus& si;
    QString value() const;
    QString key () const;
    bool hasKey(const char*, int slen) const;
    bool hasKey(const char* s) const {return hasKey(s, strlen(s));}
    bool hasContinuation() const { return len > valOffset && hasKey(" ");}
    bool hasValue() const {return len > valOffset;}
    bool readNextLine(bool emptyOK = false);
    bool isNull() const {return entry->name.isNull();}
};

}//namespace

#endif
