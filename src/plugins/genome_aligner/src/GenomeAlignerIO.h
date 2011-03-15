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

#ifndef _GENOME_ALIGNER_IO_H_
#define _GENOME_ALIGNER_IO_H_

#include <QString>

#include <U2Core/DNASequenceObject.h>
#include <U2Core/GUrl.h>
#include <U2Core/MAlignment.h>
#include <U2Formats/StreamSequenceReader.h>
#include <U2Formats/StreamSequenceWriter.h>
#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>

namespace U2 {

/************************************************************************/
/* Abstract short reads reader and writer                               */
/************************************************************************/

class GenomeAlignerReader {
public:
    virtual ~GenomeAlignerReader() {}
    virtual const DNASequenceObject *read() = 0;
    virtual bool isEnd() = 0;
};

class GenomeAlignerWriter {
public:
    virtual void write(const DNASequence &seq, int offset) = 0;
    virtual void close() = 0;
    virtual void setReferenceName(const QString &refName) = 0;
    quint32 getWrittenReadsCount() {return writtenReadsCount;}
protected:
    quint32 writtenReadsCount;
    QString refName;
};

/************************************************************************/
/* Abstract container for storing in QVariant                            */
/************************************************************************/
class GenomeAlignerReaderContainer {
public:
    GenomeAlignerReaderContainer() : reader(NULL) { }
    GenomeAlignerReaderContainer(GenomeAlignerReader* reader) {
        this->reader = reader;
    }
    GenomeAlignerReader* reader;
};

class GenomeAlignerWriterContainer {
public:
    GenomeAlignerWriterContainer() : writer(NULL) {}
    GenomeAlignerWriterContainer(GenomeAlignerWriter* writer) {
        this->writer = writer;
    }
    GenomeAlignerWriter* writer;
};

/************************************************************************/
/* URL short reads reader and writer                                    */
/************************************************************************/

class GenomeAlignerUrlReader : public GenomeAlignerReader {
public:
    GenomeAlignerUrlReader(const QList<GUrl> &dnaList);
    inline const DNASequenceObject *read();
    inline bool isEnd();
private:
    bool initOk;
    StreamSequenceReader reader;
};

class GenomeAlignerUrlWriter : public GenomeAlignerWriter {
public:
    GenomeAlignerUrlWriter(const GUrl &resultFile, const QString &refName);
    inline void write(const DNASequence &seq, int offset);
    void close();
    void setReferenceName(const QString &refName);
private:
    StreamContigWriter seqWriter;
};


/************************************************************************/
/* Workflow short reads reader and writer                                    */
/************************************************************************/
namespace LocalWorkflow {

class GenomeAlignerCommunicationChanelReader : public GenomeAlignerReader {
public:
    GenomeAlignerCommunicationChanelReader(CommunicationChannel* reads);
    ~GenomeAlignerCommunicationChanelReader();
    inline const DNASequenceObject *read();
    inline bool isEnd();
private:
    CommunicationChannel* reads;
    DNASequenceObject *obj;
};

class GenomeAlignerMAlignmentWriter : public GenomeAlignerWriter {
public:
    GenomeAlignerMAlignmentWriter();
    inline void write(const DNASequence &seq, int offset);
    void close();
    void setReferenceName(const QString &refName);
    MAlignment &getResult();
private:
    MAlignment result;
};

} //LocalWorkflow
} //U2
Q_DECLARE_METATYPE(U2::GenomeAlignerReaderContainer);
Q_DECLARE_METATYPE(U2::GenomeAlignerWriterContainer);
#endif //_GENOME_ALIGNER_IO_H_
