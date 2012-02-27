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

#ifndef _U2_ASSEMBLY_CONSENSUS_ALGORITHM_REGISTRY_H_
#define _U2_ASSEMBLY_CONSENSUS_ALGORITHM_REGISTRY_H_

#include "AssemblyConsensusAlgorithm.h"
#include <QtCore/QStringList>

namespace U2 {

class AssemblyConsensusAlgorithm;

class U2ALGORITHM_EXPORT AssemblyConsensusAlgorithmRegistry {
public:
    AssemblyConsensusAlgorithmRegistry();

    ~AssemblyConsensusAlgorithmRegistry();

    AssemblyConsensusAlgorithmFactory* getAlgorithmFactory(const QString& algoId);
    void addAlgorithmFactory(AssemblyConsensusAlgorithmFactory* algo);

    QList<QString> getAlgorithmIds() const {return algorithms.keys(); }
    QList<AssemblyConsensusAlgorithmFactory*> getAlgorithmFactories() const {return algorithms.values();}

private:
    QMap<QString , AssemblyConsensusAlgorithmFactory*> algorithms;
};

}//namespace

#endif
