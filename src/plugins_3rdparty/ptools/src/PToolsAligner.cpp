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

static PTools::Rigidbody* createRigidBody(const BioStruct3D &biostruct, int /*chainId = 0*/, int /*modelId = 0*/)
{
    // chainId and modelId ignored for now
    // all chains and first model taken

    PTools::Rigidbody *body = new PTools::Rigidbody();

    // all chains taken
    foreach (const SharedMolecule &mol, biostruct.moleculeMap.values()) {
        // this function uses modelId - key from BioStruct3D::modelMap
        //int idx = biostruct.modelMap.keys().indexOf(modelId);
        //const Molecule3DModel &model = mol->models[idx];
        // first model taken
        const Molecule3DModel &model = mol->models.first();

        // Built in assumtion that order of atoms match order of residues
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
StructuralAlignment PToolsAligner::align(const BioStruct3D &ref, const BioStruct3D &alt, int refModel /*= 0*/, int altModel /*= 0*/)
{
    StructuralAlignment result = {0, Matrix44()};
    try {
        auto_ptr<PTools::Rigidbody> prefBody(createRigidBody(ref, 0, refModel));
        auto_ptr<PTools::Rigidbody> paltBody(createRigidBody(alt, 0, altModel));

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
