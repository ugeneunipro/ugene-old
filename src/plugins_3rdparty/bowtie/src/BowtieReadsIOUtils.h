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

#ifndef _BOWTIE_IO_READS_READER_H_
#define _BOWTIE_IO_READS_READER_H_

#include <U2Core/DNASequence.h>
#include <QtCore/QMetaType>

namespace U2 {

/************************************************************************/
/* Abstract short reads reader and writer                               */
/************************************************************************/

class BowtieReadsReader {
public:
	virtual DNASequence read()=0;
	virtual bool isEnd()=0;
};

class BowtieReadsWriter {
public:
	virtual void write(const DNASequence& seq, int offset)=0;
	virtual void close()=0;
};

/************************************************************************/
/* Abstract container for store in QVartiant                            */
/************************************************************************/
class BowtieReadsReaderContainer {
public:
	BowtieReadsReaderContainer() : reader(NULL) { }
	BowtieReadsReaderContainer(BowtieReadsReader* reader) {
		this->reader = reader;
	}
	BowtieReadsReader* reader;
};

class BowtieReadsWriterContainer {
public:
	BowtieReadsWriterContainer() : writer(NULL) {}
	BowtieReadsWriterContainer(BowtieReadsWriter* writer) {
		this->writer = writer;
	}
	BowtieReadsWriter* writer;
};

} //namespace

Q_DECLARE_METATYPE( U2::BowtieReadsReaderContainer )
Q_DECLARE_METATYPE( U2::BowtieReadsWriterContainer )

#endif //_BOWTIE_IO_READS_READER_H_