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

#include "GenomeAlignerSearchQuery.h"
#include <U2Core/Log.h>
#include <limits.h>

namespace U2 {

SearchQuery::SearchQuery(const DNASequence *shortRead, SearchQuery *revCompl) {
    dna = true;
    wroteResult = false;
    this->revCompl = revCompl;
    seqLength = shortRead->length();
    nameLength = shortRead->getName().length();
    seq = new char[seqLength+1];
    name = new char[nameLength+1];
    qstrcpy(seq, shortRead->constData());
    qstrcpy(name, shortRead->getName().toAscii().constData());
    if (shortRead->hasQualityScores()) {
        quality = new DNAQuality(shortRead->quality);
    } else {
        quality = NULL;
    }
    //assRead = NULL;
}

SearchQuery::SearchQuery(const U2AssemblyRead &shortRead, SearchQuery *revCompl) {
    dna = false;
    wroteResult = false;
    this->revCompl = revCompl;
    seq = NULL;
    name = NULL;
    quality = NULL;
    //assRead = shortRead;
}

SearchQuery::~SearchQuery() {
    delete seq;
    delete name;
    delete quality;
    revCompl = NULL;
}

QString SearchQuery::getName() const {
    if (dna) {
        return QString(name);
    } else {
        return NULL;
    }
}

int SearchQuery::length() const {
    if (dna) {
        return seqLength;
    } else {
        return NULL;//assRead->readSequence.length();
    }
}

int SearchQuery::getNameLength() const {
    if (dna) {
        return nameLength;
    } else {
        return NULL;//assRead->readSequence.length();
    }
}

char *SearchQuery::data() {
    if (dna) {
        return seq;
    } else {
        return NULL;//assRead->readSequence.data();
    }
}

const char *SearchQuery::constData() const {
    if (dna) {
        return seq;
    } else {
        return NULL;//assRead->readSequence.constData();
    }
}
const QByteArray SearchQuery::constSequence() const {
    if (dna) {
        return QByteArray(seq);
    } else {
        return NULL;//assRead->readSequence;
    }
}

bool SearchQuery::hasQuality() const {
    if (dna) {
        return NULL != quality;
    } else {
        return false;
    }
}

const DNAQuality &SearchQuery::getQuality() const {
    assert(dna);
    return *quality;
}

bool SearchQuery::haveResult() const {
    return !results.isEmpty();
}

bool SearchQuery::haveMCount() const {
    return !mismatchCounts.isEmpty();
}

void SearchQuery::addResult(SAType result, quint32 mCount) {
    results.append(result);
    mismatchCounts.append(mCount);
}

void SearchQuery::addOveplapResult(SAType result) {
    overlapResults.append(result);
}

void SearchQuery::onPartChanged() {
    clear();
    results += overlapResults;
    overlapResults.clear();
}

void SearchQuery::clear() {
    results.clear();
    mismatchCounts.clear();
}

SAType SearchQuery::firstResult() const {
    return results.first();
}

quint32 SearchQuery::firstMCount() const {
    if (mismatchCounts.isEmpty()) {
        return INT_MAX;
    }
    return mismatchCounts.first();
}

bool SearchQuery::contains(SAType result) const {
    return (results.contains(result) || overlapResults.contains(result));
}

QVector<SAType> &SearchQuery::getResults() {
    return results;
}

const quint64 SearchQueryContainer::reallocSize = 100;
SearchQueryContainer::SearchQueryContainer() {
    queries = NULL;
    allocated = 0;
    length = 0;
}

SearchQueryContainer::~SearchQueryContainer() {
    delete[] queries;
    allocated = 0;
    length = 0;
}

void SearchQueryContainer::append(SearchQuery *qu) {
    if (length == allocated) {
        queries = (SearchQuery**)qRealloc(queries, (allocated + reallocSize)*sizeof(SearchQuery*));
        allocated += reallocSize;
    }

    queries[length] = qu;
    length++;
}

void SearchQueryContainer::clear() {

}

quint64 SearchQueryContainer::size() {
    return length;
}

SearchQuery *SearchQueryContainer::at(quint64 pos) {
    assert(pos <= size());
    return queries[pos];
}

} //U2
