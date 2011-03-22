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

namespace U2 {

SearchQuery::SearchQuery(const DNASequence &shortRead) {
    dna = true;
    dnaRead = new DNASequence(shortRead);
    assRead = NULL;
}

SearchQuery::SearchQuery(const U2AssemblyRead &shortRead) {
    dna = false;
    dnaRead = NULL;
    assRead = shortRead;
}

SearchQuery::~SearchQuery() {
    delete dnaRead;
    delete assRead;
}

QString SearchQuery::getName() {
    if (dna) {
        return dnaRead->getName();
    } else {
        return "";
    }
}

int SearchQuery::length() {
    if (dna) {
        return dnaRead->length();
    } else {
        return assRead->readSequence.length();
    }
}

char *SearchQuery::data() {
    if (dna) {
        return dnaRead->seq.data();
    } else {
        return assRead->readSequence.data();
    }
}

const char *SearchQuery::constData() {
    if (dna) {
        return dnaRead->constData();
    } else {
        return assRead->readSequence.constData();
    }
}
const QByteArray &SearchQuery::constSequence() {
    if (dna) {
        return dnaRead->constSequence();
    } else {
        return assRead->readSequence;
    }
}

bool SearchQuery::hasQuality() {
    if (dna) {
        return dnaRead->hasQualityScores();
    } else {
        return false;
    }
}

const DNAQuality &SearchQuery::getQuality() {
    assert(dna);
    return dnaRead->quality;
}

} //U2
