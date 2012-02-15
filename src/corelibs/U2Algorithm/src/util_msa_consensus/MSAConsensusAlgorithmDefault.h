/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#ifndef _U2_MSA_CONSENSUS_ALGORITHM_DEFAULT_H_
#define _U2_MSA_CONSENSUS_ALGORITHM_DEFAULT_H_

#include "MSAConsensusAlgorithm.h"
#include "BuiltInConsensusAlgorithms.h"

namespace U2 {

// Default algorithm is based on JalView experience. It also makes letters with low threshold lowercase
class U2ALGORITHM_EXPORT MSAConsensusAlgorithmFactoryDefault: public MSAConsensusAlgorithmFactory {
    Q_OBJECT
public:
    MSAConsensusAlgorithmFactoryDefault(QObject* p = NULL);
            
    virtual MSAConsensusAlgorithm* createAlgorithm(const MAlignment& ma, QObject* parent);

    virtual QString getDescription() const;

    virtual QString getName() const;

    virtual int getMinThreshold() const {return 1;}
    
    virtual int getMaxThreshold() const {return 100;}
    
    virtual int getDefaultThreshold() const {return 100;}

    virtual QString getThresholdSuffix() const {return QString("%");}

};


class U2ALGORITHM_EXPORT MSAConsensusAlgorithmDefault : public MSAConsensusAlgorithm {
    Q_OBJECT
public:
    MSAConsensusAlgorithmDefault(MSAConsensusAlgorithmFactoryDefault* f, QObject* p = NULL)
        : MSAConsensusAlgorithm(f, p){}

    virtual char getConsensusChar(const MAlignment& ma, int column) const {
        int countStub = 0;
        return getConsensusCharAndScore(ma, column, countStub);
    }

    virtual char getConsensusCharAndScore(const MAlignment& ma, int column, int& score) const;
};

}//namespace

#endif
