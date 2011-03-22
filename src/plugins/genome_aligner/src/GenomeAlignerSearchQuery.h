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

#ifndef _U2_GENOME_ALIGNER_SEARCH_QUERY_H_
#define _U2_GENOME_ALIGNER_SEARCH_QUERY_H_

#include <U2Core/DNASequence.h>
#include <U2Core/U2AssemblyUtils.h>

#define ResType qint64

namespace U2 {

class GenomeAlignerIndex;
class FindInPartSubTask;
class PrepareVectorsSubTask;

class CacheResult {
public:
    int posAtShortRead;
    int numberOfPart;
    quint64 bitValue;
};

class SearchQuery {
public:
    SearchQuery(const DNASequence &shortRead);
    SearchQuery(const U2AssemblyRead &shortRead);
    ~SearchQuery();

    DNASequence &getSequence() {return *dnaRead;} //tmp method
    QString getName();
    int length();
    char *data();
    const char *constData();
    const QByteArray &constSequence();
    bool hasQuality();
    const DNAQuality &getQuality();

    QList<CacheResult> cacheResults; //must be sorted by numberOfPart
    QList<quint32> results;

private:
    bool dna;
    DNASequence *dnaRead;
    U2AssemblyRead assRead;
};

} //U2
#endif //_U2_GENOME_ALIGNER_SEARCH_QUERY_H_
