/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#ifndef _U2_MSA_CONSENSUS_ALGORITHM_LEVITSKY_H_
#define _U2_MSA_CONSENSUS_ALGORITHM_LEVITSKY_H_

#include "MSAConsensusAlgorithm.h"
#include "BuiltInConsensusAlgorithms.h"

#include <QtCore/QVarLengthArray>

namespace U2 {

// DNA/RNA only consensus, that tries to
//    1) switch to extended nucleic alphabet when there is no 100% match
//    2) uses threshold score and selects the most rare (in whole alignment) symbol that has greater score
class U2ALGORITHM_EXPORT MSAConsensusAlgorithmFactoryLevitsky: public MSAConsensusAlgorithmFactory {
    Q_OBJECT
public:
    MSAConsensusAlgorithmFactoryLevitsky(QObject* p = NULL);
    
    virtual MSAConsensusAlgorithm* createAlgorithm(const MAlignment& ma, QObject* parent);

    virtual QString getDescription() const;

    virtual QString getName() const;

    virtual int getMinThreshold() const {return 50;}

    virtual int getMaxThreshold() const {return 100;}

    virtual int getDefaultThreshold() const {return 90;}

    virtual QString getThresholdSuffix() const {return QString("%");}

};

class U2ALGORITHM_EXPORT MSAConsensusAlgorithmLevitsky: public MSAConsensusAlgorithm {
    Q_OBJECT
public:
    MSAConsensusAlgorithmLevitsky(MSAConsensusAlgorithmFactoryLevitsky* f, const MAlignment& ma,  QObject* p = NULL);

    virtual char getConsensusChar(const MAlignment& ma, int column) const;

private:
    QVarLengthArray<int> globalFreqs;
};


}//namespace

#endif
