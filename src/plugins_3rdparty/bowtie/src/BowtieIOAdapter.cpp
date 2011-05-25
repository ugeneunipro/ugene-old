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

#include "stdint.h"
#include <U2Core/MAlignment.h>
#include <U2Core/DNASequence.h>
#include <U2Core/AppContext.h>
#include <U2Core/TextUtils.h>
#include <U2Core/DNATranslation.h>

#include "BowtieIOAdapter.h"
#include "BowtieContext.h"

#include <QtCore/QMutexLocker>

/************************************************************************/
/* BowtieUrlReadsReader                                                 */
/************************************************************************/

BowtieUrlReadsReader::BowtieUrlReadsReader(const QList<U2::GUrl>& dnaList) {
	initOk = this->reader.init(dnaList);
}

bool BowtieUrlReadsReader::isEnd() {
    if (!initOk) {
        return true;
    }
	return !reader.hasNext();
}

U2::DNASequence *BowtieUrlReadsReader::read() {
    BowtieContext::getContext()->ti.progress = reader.getProgress();
	return reader.getNextSequenceObject();
}

/************************************************************************/
/* BowtieUrlReadsWriter                                                 */
/************************************************************************/

BowtieUrlReadsWriter::BowtieUrlReadsWriter(const U2::GUrl& resultFile, const QString& refName, int refLength)
	:seqWriter(resultFile, refName, refLength)
{
	//do nothing	
}

void BowtieUrlReadsWriter::write(const U2::DNASequence& seq, int offset) {
	seqWriter.writeNextAlignedRead(offset, seq);
}

void BowtieUrlReadsWriter::close() {
	seqWriter.close();
}

/************************************************************************/
/* BowtieDbiReadsWriter                                                 */
/************************************************************************/
const qint64 BowtieDbiReadsWriter::readBunchSize = 10000;

inline void checkOperationStatus(const U2::U2OpStatus &status) {
    if (status.hasError()) {
        throw status.getError();
    }
}

BowtieDbiReadsWriter::BowtieDbiReadsWriter(const U2::GUrl& dbiFilePath, const QString& refName) {
    //TODO: support several assemblies.
    dbiHandle = QSharedPointer<U2::DbiHandle>(new U2::DbiHandle("SQLiteDbi", dbiFilePath.getURLString(), true, status));
    checkOperationStatus(status);
    sqliteDbi = dbiHandle->dbi;
    wDbi = sqliteDbi->getAssemblyDbi();

    sqliteDbi->getObjectDbi()->createFolder("/", status);
    checkOperationStatus(status);
    assembly.visualName = refName;
    U2::U2AssemblyReadsImportInfo importInfo;
    wDbi->createAssemblyObject(assembly, "/", NULL, importInfo, status);
    checkOperationStatus(status);
}

void BowtieDbiReadsWriter::write(const U2::DNASequence& seq, int offset) {
    U2::U2AssemblyRead read(new U2::U2AssemblyReadData());

    read->name = seq.getName().toAscii();
    read->leftmostPos = offset;
    read->effectiveLen = seq.length();
    read->readSequence = seq.constSequence();
    read->quality = seq.hasQualityScores() ? seq.quality.qualCodes : "";
    read->flags = U2::None;
    read->cigar.append(U2::U2CigarToken(U2::U2CigarOp_M, seq.length()));

    reads.append(read);
    if (reads.size() >= readBunchSize) {
        U2::BufferedDbiIterator<U2::U2AssemblyRead> readsIterator(reads);
        wDbi->addReads(assembly.id, &readsIterator, status);
        checkOperationStatus(status);
        reads.clear();
    }
}

void BowtieDbiReadsWriter::close() {
    if (reads.size() > 0) {
        U2::BufferedDbiIterator<U2::U2AssemblyRead> readsIterator(reads);
        wDbi->addReads(assembly.id, &readsIterator, status);
        checkOperationStatus(status);
        reads.clear();
    }

    U2::U2AssemblyPackStat packStatus;
    wDbi->pack(assembly.id, packStatus, status);
    checkOperationStatus(status);
}

/************************************************************************/
/* DNASequencesPatternSource                                            */
/************************************************************************/
DNASequencesPatternSource::DNASequencesPatternSource( uint32_t seed, U2::BowtieReadsReader* reader )
 : PatternSource(seed)
{
    this->reader = reader;
}

void DNASequencesPatternSource::nextReadImpl( ReadBuf& r, uint32_t& patid )
{
  U2::DNASequence *dna;
   
  {
    QMutexLocker lock(&mutex);
    patid = readCnt_;
    if (reader->isEnd()) {
       return;
    }
    ++readCnt_;
    dna = new U2::DNASequence(*reader->read());
  }
       
  
  r.clearAll();


  BowtieContext::Search* ctx = BowtieContext::getSearchContext();
  bool doquals = false; //TODO: quals

  int nameLen = dna->getName().length();
  strcpy(r.nameBuf, dna->getName().toAscii().constData());
  _setBegin(r.name, r.nameBuf);
  _setLength(r.name, nameLen);

  if(nameLen == 0) {
	  itoa10(readCnt_, r.nameBuf);
	  _setBegin(r.name, r.nameBuf);
	  nameLen = strlen(r.nameBuf);
	  _setLength(r.name, nameLen);
  }

  const char* row = dna->seq.constData();
  const int seqLen = dna->length();
  if(seqLen+1 > 1024) {
      tooManySeqChars(r.name);
  }
  
  //TODO: trim;
  r.trimmed3 = 0;
  r.trimmed5 = 0;
  r.color = ctx->color;
  //TODO: if(color) ...
  for(int i=0;i<seqLen;i++) {
    char c =  row[i];
    if(c == '.') c = 'N';
    r.patBufFw[i] = charToDna5[(int)c];
  }
  if(!doquals) {
	  if(!dna->quality.isEmpty()) {
		  switch(dna->quality.type) {
			  case U2::DnaQualityType_Solexa: 
				  for(int i=0;i<seqLen;i++) r.qualBuf[i] = solexaToPhred(dna->quality.getValue(i)+33); break;
			  case U2::DNAQualityType_Sanger: 
				  for(int i=0;i<seqLen;i++) r.qualBuf[i] = dna->quality.getValue(i)+33; break;
			  default: for(int i=0;i<seqLen;i++) r.qualBuf[i] = 'I';
		  }
	  } else {
		  for(int i=0;i<seqLen;i++) r.qualBuf[i] = 'I';
	  }
  }
  _setBegin (r.patFw, (Dna5*)r.patBufFw);
  _setLength(r.patFw, seqLen);
  _setBegin (r.qual,  r.qualBuf);
  _setLength(r.qual,  seqLen);

  //r.readOrigBufLen = fb_.copyLastN(r.readOrigBuf);
}

void DNASequencesPatternSource::nextReadPairImpl( ReadBuf& ra, ReadBuf& rb, uint32_t& patid )
{
  Q_UNUSED(ra);
  Q_UNUSED(rb);
  Q_UNUSED(patid);
  throw 1;
}

/************************************************************************/
/* MAlignmentHitSink                                                    */
/************************************************************************/

MAlignmentHitSink::MAlignmentHitSink( U2::BowtieReadsWriter* _writer, OutFileBuf* out, DECL_HIT_DUMPS, bool onePairFile, bool sampleMax, RecalTable *table, vector<string>* refnames /*= NULL*/ )
:HitSink(out, PASS_HIT_DUMPS, onePairFile, sampleMax, table, refnames), writer(_writer)
{
  //do nothing
}

void MAlignmentHitSink::append( ostream& o, const Hit& h )
{
  Q_UNUSED(o);
  QMutexLocker lock(&mutex);
  QByteArray name(h.patName.data_begin, length(h.patName));
  QByteArray sequence;
  QByteArray quality;
  const int seqLen = length(h.patSeq);
  for(int i=0; i < seqLen; i++) {
	  char qryChar = (h.fw ? h.patSeq[i] : h.patSeq[seqLen-i-1]);
	  char qualChar = (h.fw ? h.quals[i] : h.quals[seqLen-i-1]);
	  sequence.append(qryChar);
	  quality.append(qualChar);
  }

  if(!h.fw) {
      U2::DNAAlphabet *al = U2::AppContext::getDNAAlphabetRegistry()->findAlphabet(sequence);
      if(al == NULL) {
          throw new BowtieException("Can't find alphabet for sequence");
      }
      U2::DNATranslation* tr = U2::AppContext::getDNATranslationRegistry()->lookupComplementTranslation(al);
      if(tr == NULL) {
          throw new BowtieException("Can't translation for alphabet");
      }

      U2::TextUtils::translate(tr->getOne2OneMapper(), sequence.data(), sequence.length());
  }

  U2::DNASequence dna(name, sequence);
  int offset = h.h.second;
  dna.quality.qualCodes = quality;
  writer->write(dna, offset);
  (*BowtieContext::getContext()->numHitsOverall)++;
  
//   if(BowtieContext::getSearchContext()->sortAlignment) {
// 	  int count = rowList.count();
// 	  bool indexFound = false;
// 	  for(int i=0;i<count;i++) {
// 		  if(rowList.at(i).getCoreStart() > row.getCoreStart()) {
// 			  rowList.insert(i, row);
// 			  indexFound = true;
// 			  break;
// 		  }
// 	  }
// 	  if(!indexFound) rowList.append(row);
//   } else {
// 	  rowList.append(row);
//   }
}

void MAlignmentHitSink::commitResultMA()
{
    writer->close();
    // 	for(int i=0;i<rowList.count();i++)
// 		resultMA.addRow(rowList.at(i));
}

