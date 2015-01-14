/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#ifndef _U2_ASSEMBLY_CONSENSUS_ALGORITHM_DEFAULT_H_
#define _U2_ASSEMBLY_CONSENSUS_ALGORITHM_DEFAULT_H_

#include "AssemblyConsensusAlgorithm.h"

namespace U2 {

class U2ALGORITHM_EXPORT AssemblyConsensusAlgorithmFactoryDefault : public AssemblyConsensusAlgorithmFactory {
    Q_OBJECT
public:
    AssemblyConsensusAlgorithmFactoryDefault();
    virtual QString getDescription() const;
    virtual QString getName() const;

    virtual AssemblyConsensusAlgorithm* createAlgorithm();
};

class U2ALGORITHM_EXPORT AssemblyConsensusAlgorithmDefault : public AssemblyConsensusAlgorithm {
public:
    AssemblyConsensusAlgorithmDefault(AssemblyConsensusAlgorithmFactoryDefault* factory) : AssemblyConsensusAlgorithm(factory) {}

    virtual QByteArray getConsensusRegion(const U2Region &region, U2DbiIterator<U2AssemblyRead>* reads, QByteArray referenceFragment, U2OpStatus &os);
};

}//namespace

#endif
