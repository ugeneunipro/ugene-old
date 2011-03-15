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

#ifndef _U2_MSA_DISTANCE_ALGORITHM_H_
#define _U2_MSA_DISTANCE_ALGORITHM_H_

#include <U2Core/global.h>
#include <U2Core/U2Region.h>
#include <U2Core/Task.h>
#include <U2Core/MAlignment.h>
#include <QtCore/QVarLengthArray>
#include <QtCore/QMutex>

namespace U2 {

class MAlignment;
class MSADistanceAlgorithm;
class DNAAlphabet;

enum DistanceAlgorithmFlag {
    DistanceAlgorithmFlag_Nucleic = 1 << 0,
    DistanceAlgorithmFlag_Amino = 1 << 1,
    DistanceAlgorithmFlag_Raw = 1 << 2,
};

typedef QFlags<DistanceAlgorithmFlag> DistanceAlgorithmFlags;
#define DistanceAlgorithmFlags_AllAlphabets (DistanceAlgorithmFlags(DistanceAlgorithmFlag_Nucleic) | DistanceAlgorithmFlag_Amino | DistanceAlgorithmFlag_Raw)
#define DistanceAlgorithmFlags_NuclAmino    (DistanceAlgorithmFlags(DistanceAlgorithmFlag_Nucleic) | DistanceAlgorithmFlag_Amino)

class U2ALGORITHM_EXPORT MSADistanceAlgorithmFactory : public QObject {
    Q_OBJECT
public:
    MSADistanceAlgorithmFactory(const QString& algoId, DistanceAlgorithmFlags flags, QObject* p = NULL);
    
    virtual MSADistanceAlgorithm* createAlgorithm(const MAlignment& ma, QObject* parent = NULL) = 0;
    
    QString getId() const {return algorithmId;}

    DistanceAlgorithmFlags getFlags() const {return flags;}

    virtual QString getDescription() const = 0;

    virtual QString getName() const = 0;

    // utility method
    static DistanceAlgorithmFlags getAphabetFlags(const DNAAlphabet* al);

private:
    QString                 algorithmId;
    DistanceAlgorithmFlags  flags;

};

class U2ALGORITHM_EXPORT MSADistanceAlgorithm : public Task {
    Q_OBJECT
public:
    MSADistanceAlgorithm(MSADistanceAlgorithmFactory* factory, const MAlignment& ma);

    int getSimilarity(int row1, int row2);
    
    virtual QString getDescription() const {return factory->getDescription();}

    virtual QString getName() const {return factory->getName();}

    QString getId() const {return factory->getId();}

    MSADistanceAlgorithmFactory* getFactory() const {return factory;}

private:
    MSADistanceAlgorithmFactory*    factory;

protected:
    MAlignment                                  ma;
    QVarLengthArray<QVarLengthArray<int> >      distanceTable;
    QMutex                                      lock;
};

}//namespace

#endif
