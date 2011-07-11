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

#ifndef _BOWTIE_IO_ADAPTER_H_
#define _BOWTIE_IO_ADAPTER_H_

#include "bowtie/pat.h"
#include "bowtie/hit.h"
#include "BowtieReadsIOUtils.h"

#include <U2Core/GUrl.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Formats/StreamSequenceReader.h>
#include <U2Formats/StreamSequenceWriter.h>

#include <QtCore/QMutex>
#include <QtCore/QSharedPointer>

namespace U2 {
	class MAlignment;
	class MAlignmentRow;
	class DNASequence;
}

/************************************************************************/
/* DNAReadsReader                                                       */
/************************************************************************/

class BowtieUrlReadsReader : public U2::BowtieReadsReader {
public:
	BowtieUrlReadsReader(const QList<U2::GUrl>& dnaList);
	U2::DNASequence *read();
	bool isEnd();
private:
	bool initOk;
	U2::StreamSequenceReader reader;
};

class BowtieUrlReadsWriter : public U2::BowtieReadsWriter {
public:
	BowtieUrlReadsWriter(const U2::GUrl& resultFile, const QString& refName, int refLength);
	void write(const U2::DNASequence& seq, int offset);
	void close();
private:
	U2::StreamShortReadsWriter seqWriter;
};

/************************************************************************/
/* DbiReadsWriter                                                       */
/************************************************************************/

class BowtieDbiReadsWriter : public U2::BowtieReadsWriter {
public:
    BowtieDbiReadsWriter(const U2::GUrl& dbiFilePath, const QString& refName);
    void write(const U2::DNASequence& seq, int offset);
    void close();
private:
    U2::U2OpStatusImpl status;
    QSharedPointer<U2::DbiHandle> dbiHandle;
    U2::U2Dbi* sqliteDbi;
    U2::U2AssemblyDbi *wDbi;
    U2::U2Assembly assembly;
    QList<U2::U2AssemblyRead> reads;

    static const qint64 readBunchSize;
};

/************************************************************************/
/* DNASequencesPatternSource                                            */
/************************************************************************/

class DNASequencesPatternSource: public PatternSource {
public:
	DNASequencesPatternSource(uint32_t seed, U2::BowtieReadsReader* reader);
	void nextReadImpl(ReadBuf& r, uint32_t& patid);
	void nextReadPairImpl(ReadBuf& ra, ReadBuf& rb, uint32_t& patid);
private:
	U2::BowtieReadsReader* reader;
	QMutex mutex;
};

/************************************************************************/
/* MAlignmentHitSink                                                    */
/************************************************************************/


class MAlignmentHitSink: public HitSink {
public:
	MAlignmentHitSink(U2::BowtieReadsWriter* writer, OutFileBuf* out, DECL_HIT_DUMPS, bool onePairFile, bool sampleMax, RecalTable *table, vector<string>* refnames = NULL);
	void append(ostream& o, const Hit& h);
	void commitResultMA();
private:
	U2::BowtieReadsWriter* writer;
	QMutex mutex;
};

#endif // _BOWTIE_IO_ADAPTER_H_
