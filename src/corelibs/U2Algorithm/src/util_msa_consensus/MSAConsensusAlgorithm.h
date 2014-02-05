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

#ifndef _U2_MSA_CONSENSUS_ALGORITHM_H_
#define _U2_MSA_CONSENSUS_ALGORITHM_H_

#include <U2Core/global.h>
#include <U2Core/U2Region.h>

namespace U2 {

class MAlignment;
class MSAConsensusAlgorithm;
class DNAAlphabet;

enum ConsensusAlgorithmFlag {
    ConsensusAlgorithmFlag_Nucleic = 1 << 0,
    ConsensusAlgorithmFlag_Amino = 1 << 1,
    ConsensusAlgorithmFlag_Raw = 1 << 2,
    ConsensusAlgorithmFlag_SupportThreshold = 1 << 3,
};

typedef QFlags<ConsensusAlgorithmFlag> ConsensusAlgorithmFlags;
#define ConsensusAlgorithmFlags_AllAlphabets (ConsensusAlgorithmFlags(ConsensusAlgorithmFlag_Nucleic) | ConsensusAlgorithmFlag_Amino | ConsensusAlgorithmFlag_Raw)
#define ConsensusAlgorithmFlags_NuclAmino    (ConsensusAlgorithmFlags(ConsensusAlgorithmFlag_Nucleic) | ConsensusAlgorithmFlag_Amino)

class U2ALGORITHM_EXPORT MSAConsensusAlgorithmFactory : public QObject {
    Q_OBJECT
public:
    MSAConsensusAlgorithmFactory(const QString& algoId, ConsensusAlgorithmFlags flags, QObject* p = NULL);
    
    virtual MSAConsensusAlgorithm* createAlgorithm(const MAlignment& ma, QObject* parent = NULL) = 0;
    
    QString getId() const {return algorithmId;}

    ConsensusAlgorithmFlags getFlags() const {return flags;}

    virtual QString getDescription() const = 0;

    virtual QString getName() const = 0;

    virtual bool supportsThreshold() const {return flags.testFlag(ConsensusAlgorithmFlag_SupportThreshold);}

    virtual int getMinThreshold() const = 0;
    
    virtual int getMaxThreshold() const = 0;

    virtual int getDefaultThreshold() const = 0;

    virtual QString getThresholdSuffix() const {return QString();}

    // utility method
    static ConsensusAlgorithmFlags getAphabetFlags(const DNAAlphabet* al);

private:
    QString                 algorithmId;
    ConsensusAlgorithmFlags flags;

};

class U2ALGORITHM_EXPORT MSAConsensusAlgorithm : public QObject {
    Q_OBJECT
public:
    MSAConsensusAlgorithm(MSAConsensusAlgorithmFactory* factory, QObject* p = NULL);

    /**
        Returns consensus char and score for the given row
        Score is a number: [0, num] sequences. Usually is means count of the char in the row
        Note that consensus character may be out of the to MSA alphabet symbols range
    */
    virtual char getConsensusCharAndScore(const MAlignment& ma, int column, int& score) const;

    virtual char getConsensusChar(const MAlignment& ma, int column) const = 0;
    
    virtual QString getDescription() const {return factory->getDescription();}

    virtual QString getName() const {return factory->getName();}

    virtual void setThreshold(int val);
    
    virtual int getThreshold() const {return threshold;}

    bool supportsThreshold() const {return factory->supportsThreshold();}

    virtual int getMinThreshold() const {return factory->getMinThreshold();}
    
    virtual int getMaxThreshold() const {return factory->getMaxThreshold();}

    virtual int getDefaultThreshold() const {return factory->getDefaultThreshold();}

    virtual QString getThresholdSuffix() const {return factory->getThresholdSuffix();}

    QString getId() const {return factory->getId();}

    MSAConsensusAlgorithmFactory* getFactory() const {return factory;}

signals:
    void si_thresholdChanged(int);

private:
    MSAConsensusAlgorithmFactory* factory;
    int threshold;

};

}//namespace

#endif
