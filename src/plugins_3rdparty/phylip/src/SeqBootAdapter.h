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

#include <U2Core/MAlignment.h>
#include <U2Core/PhyTree.h>
#include <U2Algorithm/CreatePhyTreeSettings.h>
#include <QVector>
#include <iostream>

#include "seqboot.h"

namespace U2{

typedef QVector<float> matrixrow;
typedef QVector<matrixrow> matrix;

class SeqBoot{
private:
    const MAlignment* malignment;
    QVector<sequence> generatedSeq;

public:
//    QVector<sequence>& getGeneratedSequences() const {return generatedSeq;}
    void clearGenratedSequences(); //to free memory
    void generateSequencesFromAlignment( const MAlignment& ma, const CreatePhyTreeSettings& settings );

    
};


}
