/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#include <U2Algorithm/NWAligner.h>

#include "PairwiseAligner.h"

namespace U2 {

const QString PairwiseAlignerFactory::NEEDLEMAN_WUNSCH("Needleman-Wunsch");

PairwiseAligner::PairwiseAligner(const QByteArray &_seq1, const QByteArray &_seq2)
: seq1(_seq1), seq2(_seq2)
{

}

PairwiseAligner::~PairwiseAligner() {

}

void PairwiseAligner::setSeq1(const QByteArray &value) {
    seq1 = value;
}

void PairwiseAligner::setSeq2(const QByteArray &value) {
    seq2 = value;
}

void PairwiseAligner::setSeqs(const QByteArray &value1, const QByteArray &value2) {
    seq1 = value1;
    seq2 = value2;
}

PairwiseAligner * PairwiseAlignerFactory::createAligner(const QString &alignerId,
    const QByteArray &seq1, const QByteArray &seq2) {
    if (NEEDLEMAN_WUNSCH == alignerId) {
        return new NWAligner(seq1, seq2);
    }
    return NULL;
}

} // U2
