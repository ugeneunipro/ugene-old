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

#include "PToolsAligner.h"

#include <U2Core/Log.h>
#include <U2Core/BioStruct3D.h>

#include "ptools/superpose.h"

#include <memory>
#include <exception>

using std::auto_ptr;

namespace U2 {

static PTools::Rigidbody* createRigidBody(const BioStruct3D &biostruct, QList<int> chainIds, int modelId)
{
    PTools::Rigidbody *body = new PTools::Rigidbody();

    foreach (int chainId, chainIds) {
        const Molecule3DModel &model = biostruct.getModelByName(chainId, modelId);

        // built in assumtion that order of atoms in BioStruct3D matches order of residues
        foreach (const SharedAtom &atom, model.atoms)
        {
            PTools::Atomproperty pproperty;
            pproperty.SetType(atom->name.data());
            pproperty.SetResidId(atom->residueIndex);
            //pproperty.SetAtomId();

            const Vector3D &coord = atom->coord3d;
            PTools::Coord3D pcoord(coord.x, coord.y, coord.z);

            PTools::Atom patom(pproperty, pcoord);
            body->AddAtom(patom);
        }
    }

    return body;
}

/* class PToolsAligner : public StructuralAlignmentAlgorithm */
StructuralAlignment PToolsAligner::align(const BioStruct3D &ref, const BioStruct3D &alt, int refModel /*= -1*/, int altModel /*= -1*/)
{
    StructuralAlignment result;
    try {
        // by default first model used
        refModel = (refModel == -1) ? ref.getModelsNames().first() : refModel;
        altModel = (altModel == -1) ? alt.getModelsNames().first() : altModel;

        // all chains used
        auto_ptr<PTools::Rigidbody> prefBody(createRigidBody(ref, ref.moleculeMap.keys(), refModel));
        auto_ptr<PTools::Rigidbody> paltBody(createRigidBody(alt, alt.moleculeMap.keys(), altModel));

        Superpose_t presult = PTools::superpose(*prefBody, *paltBody);

        result.rmsd = presult.rmsd;
        for (int i = 0; i < 16; ++i)
        {
            result.transform[i] = presult.matrix(i/4, i%4);
        }
    }
    catch (std::exception e) {
        algoLog.error(QString("Internal ptools error: %1").arg(e.what()));
    }
    catch (...) {
        algoLog.error(QString("Internal ptools error"));
    }

    return result;
}

}   // namespace U2
