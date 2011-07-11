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

#ifndef _SAM_FORMAT_H
#define _SAM_FORMAT_H

#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentModel.h>

namespace U2 {

class IOAdapter;
class DNASequence;

class U2FORMATS_EXPORT  SAMFormat : public DocumentFormat {
    Q_OBJECT
public:
    SAMFormat(QObject* p = NULL);

    virtual DocumentFormatId getFormatId() const {return BaseDocumentFormats::SAM;}

    virtual const QString& getFormatName() const {return formatName;}

    virtual Document* loadDocument( IOAdapter* io, TaskStateInfo& ti, const QVariantMap& fs, DocumentLoadMode mode = DocumentLoadMode_Whole);

    virtual void storeDocument( Document* d, TaskStateInfo& ts, IOAdapter* io );

    virtual RawDataCheckResult checkRawData(const QByteArray& rawData, const GUrl& = GUrl()) const;

    bool storeHeader(IOAdapter* io, const QVector<QByteArray> &names, const QVector<int> &lengths);
    bool storeAlignedRead(int offset, const DNASequence& read, IOAdapter* io, const QByteArray& refName, int refLength, bool first, bool useCigar = false, const QByteArray &cigar = "");

    class Field {
        public:
            Field(QString _name, QString _pattern): name(_name), precompiled(_pattern) {}
            QString name;
            QRegExp getPattern() const { return QRegExp(precompiled); }
        private:
            const QRegExp precompiled;

    };

private:

    static bool getSectionTags(QByteArray &line, const QByteArray &sectionName, QList<QByteArray> &tags);

    static bool validateField(int num, QByteArray &field, TaskStateInfo *ti = NULL);

    static const QByteArray VERSION;

    static const QByteArray SAM_SECTION_START;
 
    static const QByteArray SECTION_HEADER; //Header
    static const QByteArray SECTION_SEQUENCE; //Sequence dictionary
    static const QByteArray SECTION_READ_GROUP; //read group
    static const QByteArray SECTION_PROGRAM; //Program
    static const QByteArray SECTION_COMMENT; //comment

    static const QByteArray TAG_VERSION; //File format version.
    static const QByteArray TAG_SORT_ORDER; //Sort order. Valid values are: unsorted, queryname or coordinate.
    static const QByteArray TAG_GROUP_ORDER; //Group order (full sorting is not imposed in a group). Valid values are: none, query or reference.

    static const QByteArray TAG_SEQUENCE_NAME; //Sequence name. Unique among all sequence records in the file. The value of this field is used in alignment records.
    static const QByteArray TAG_SEQUENCE_LENGTH; //Sequence length.
    static const QByteArray TAG_GENOME_ASSEMBLY_ID; //Genome assembly identifier. Refers to the reference genome assembly in an unambiguous    form. Example: HG18.
    static const QByteArray TAG_SEQUENCE_MD5_SUM; //MD5 checksum of the sequence in the uppercase (gaps and space are removed)
    static const QByteArray TAG_SEQUENCE_URI; //URI of the sequence
    static const QByteArray TAG_SEQUENCE_SPECIES; //Species.

    static const int READ_BUFF_SIZE = 1024;
    static const char SPACE = '\t';

    static const Field samFields[];

    /*
    static const TAG_READ_GROUP_ID = "ID";
    static const TAG_READ_GROUP_SAMPLE = "SM";
    static const TAG_READ_GROUP_LIBRARY = "LB";
    static const TAG_READ_GROUP_DESCRIPTION = "DS";
    static const TAG_READ_GROUP_PLATFORM = "PU";
    */

    QString formatName;

};

} //namespace

#endif //_SAM_FORMAT_H
