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

#include <U2Core/BioStruct3D.h>
#include <QtOpenGL>

#include "GraphicUtils.h"
#include "BioStruct3DColorScheme.h"
#include "BioStruct3DGLWidget.h"
#include "WormsGLRenderer.h"


namespace U2 { 

const QString WormsGLRenderer::ID(QObject::tr("Worms"));


WormsGLRenderer::WormsGLRenderer( const BioStruct3D& struc, const BioStruct3DColorScheme* s, const QList<int> &shownModels, const BioStruct3DGLWidget *widget )
        : BioStruct3DGLRenderer(struc,s,shownModels,widget)
{
    const char* alphaCarbonTag = "CA";
    const char* phosophorTag = "P";
    const char* carbonylOxygenTag = "O";
    
    int numModels = struc.modelMap.count();


    QMapIterator<int, SharedMolecule> i(struc.moleculeMap);
    while (i.hasNext()) {
        i.next();
        const SharedMolecule mol = i.value();
        BioPolymer bioPolymer;
        bioPolymer.bpModels.resize(numModels);
        int modelId = 0;
        foreach (const Molecule3DModel& model, mol->models) {
            BioPolymerModel& bpModel = bioPolymer.bpModels[modelId];
            foreach (const SharedAtom atom, model.atoms) {
                if ((atom->name.trimmed() == alphaCarbonTag) || (atom->name.trimmed() == phosophorTag)) {
                    bpModel.monomerMap[atom->residueIndex].alphaCarbon = atom;
                }
                if ( (atom->name.trimmed() == carbonylOxygenTag) && (bpModel.monomerMap.contains(atom->residueIndex))) {
                    bpModel.monomerMap[atom->residueIndex].carbonylOxygen = atom;
                }
            }
            ++modelId;
            
        }
        bioPolymerMap.insert(i.key(), bioPolymer);
    }

    createWorms();    

}


void WormsGLRenderer::drawWorms(  )
{
    GLUquadricObj *pObj;    // Quadric Object

    // Draw atoms as spheres
    pObj = gluNewQuadric();
    gluQuadricNormals(pObj, GLU_SMOOTH);
    static float ribbonThickness = 0.3f;
    static float tension = 0.01f;

    foreach (int chainId, wormMap.keys()) {
        const Worm worm = wormMap.value(chainId);
        foreach (int index, shownModels) {
            const WormModel& model = worm.models.at(index);
            //Draw worm bodies (let the bodies set the scene!!!)
            const AtomsVector wormCoords = model.atoms; 
            int size = wormCoords.size();
            Color4f atomColor;
            for (int i = 0; i + 3 < size; ++i) {
                const SharedAtom a0 = wormCoords.at(i);
                const SharedAtom a1 = wormCoords.at(i+1);
                const SharedAtom a2 = wormCoords.at(i+2);
                const SharedAtom a3 = wormCoords.at(i+3);
                // draw worms only between sequential residues
                if (a2->residueIndex - a1->residueIndex != 1) {
                    continue;
                }

                glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, getAtomColor(a1));
                glDrawHalfWorm(a0->coord3d, a1->coord3d, a2->coord3d, a3->coord3d, ribbonThickness, false, false, tension, glWidget->getRenderDetailLevel());
                glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, getAtomColor(a2));
                glDrawHalfWorm(a3->coord3d, a2->coord3d, a1->coord3d, a0->coord3d, ribbonThickness, false, false, tension, glWidget->getRenderDetailLevel());
            }

            if (wormCoords.size() >= 3) {
                // Draw worm opening
                const SharedAtom a0 = wormCoords.at(0);
                const SharedAtom a1 = wormCoords.at(1);
                const SharedAtom a2 = wormCoords.at(2);
                Vector3D atomCoordFirst = model.openingAtom;
                glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, getAtomColor(a0));
                glDrawHalfWorm(atomCoordFirst, a0->coord3d, a1->coord3d, a2->coord3d, ribbonThickness, true, false, tension, glWidget->getRenderDetailLevel());
                glDrawHalfWorm(a2->coord3d, a1->coord3d, a0->coord3d, atomCoordFirst, ribbonThickness, false, false, tension, glWidget->getRenderDetailLevel());
                // Draw worm ending
                const SharedAtom aN1 = wormCoords.at(size - 1);
                const SharedAtom aN2 = wormCoords.at(size - 2);
                const SharedAtom aN3 = wormCoords.at(size - 3);
                Vector3D atomCoordLast = model.closingAtom;
                glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, getAtomColor(aN1));
                glDrawHalfWorm(atomCoordLast, aN1->coord3d, aN2->coord3d, aN3->coord3d, ribbonThickness, true, false, tension, glWidget->getRenderDetailLevel());
                glDrawHalfWorm(aN3->coord3d, aN2->coord3d, aN1->coord3d, atomCoordLast, ribbonThickness, false, false, tension, glWidget->getRenderDetailLevel());
            }
            
            //Draw 3d objects
            if (shownModels.count() == 1) {
                foreach( Object3D* obj, model.objects) {
                    obj->draw(glWidget->getRenderDetailLevel());
                }
            }

        }
    }

    gluDeleteQuadric(pObj);
}


void WormsGLRenderer::drawBioStruct3D()
{
    drawWorms();

}

void WormsGLRenderer::updateColorScheme()
{
    foreach  (int id, wormMap.keys() ) {
         Worm& worm = wormMap[id];
         int numModels = worm.models.count();
         for (int i= 0; i < numModels; ++i) {
             WormModel& model = worm.models[i];
             qDeleteAll(model.objects);
             model.objects.clear();
         }
     }
    
    //TODO: optimization -> don't have to create everything again
    createObjects3D();
}

void WormsGLRenderer::updateShownModels() {
    updateColorScheme();
}

void WormsGLRenderer::createObjects3D()
{
    foreach (const SharedSecondaryStructure ss, bioStruct.secondaryStructures) {
        int startId = ss->startSequenceNumber;
        int endId = ss->endSequenceNumber;
        int chainId = ss->chainIndex;
        Q_ASSERT( chainId != 0 );
        if (bioPolymerMap.contains(chainId)) {
            const BioPolymer& bpolymer = bioPolymerMap.value(chainId);
            int modelId = 0;
            foreach (const BioPolymerModel& bpModel, bpolymer.bpModels ) {
                if (bpModel.monomerMap.contains(startId) && bpModel.monomerMap.contains(endId)) {
                    Object3D* obj = NULL;
                    if (ss->type == SecondaryStructure::Type_AlphaHelix ) {
                        obj = createHelix3D(startId, endId, bpModel);
                    } else if (ss->type == SecondaryStructure::Type_BetaStrand) {
                        obj = createStrand3D(startId, endId, bpModel);
                    } 
                    if (obj != NULL) {
                        wormMap[chainId].models[modelId].objects.append(obj);
                    }
                }
                ++modelId;
            }
        }
    }
}

void WormsGLRenderer::createWorms()
{
    // Create worms coordinates
    QMapIterator<int, BioPolymer> i(bioPolymerMap);
    while (i.hasNext()) {
        i.next();
        int chainID = i.key();
        const BioPolymer& bioPolymer = i.value();
        Vector3D r1, r2,a,b;
        Vector3D openingVector, closingVector;
        Worm worm;
        foreach (const BioPolymerModel& bpModel, bioPolymer.bpModels) {
            WormModel wormModel;
            const QMap<int,Monomer> monomers = bpModel.monomerMap;
            QMap<int,Monomer>::const_iterator iter(monomers.constBegin());
            // Calculate opening atom coords
            Monomer m = iter.value();
            r1 = iter.value().alphaCarbon.constData()->coord3d;
            r2 = (++iter).value().alphaCarbon.constData()->coord3d;
            
            a = r1;
            b = (r2 - r1) / 100.f;
            wormModel.openingAtom = a + b*(-10.f);
            // Calculate closing atom coords
            iter = bpModel.monomerMap.constEnd();
            r1 = (--iter).value().alphaCarbon.constData()->coord3d;
            r2 = (--iter).value().alphaCarbon.constData()->coord3d;
            a = r1;
            b = (r2 - r1) / 100.f;
            wormModel.closingAtom = a + b*(-10.f);
            // Add worm-building atom coords 
            foreach (const Monomer& monomer, monomers) {
                const SharedAtom& atom = monomer.alphaCarbon;
                wormModel.atoms.append(atom);
            }
            worm.models.append(wormModel);
        }
        wormMap.insert(chainID, worm);
    }
    
    createObjects3D();

}


WormsGLRenderer::~WormsGLRenderer()
{
    foreach (Worm worm, wormMap) {
        foreach (WormModel model, worm.models) {
            qDeleteAll(model.objects);
        }
    }
}

const float* WormsGLRenderer::getAtomColor(const SharedAtom& atom)
{
    atomColor = colorScheme->getAtomColor(atom);
    return atomColor.getConstData();
}

Object3D* WormsGLRenderer::createHelix3D( int startId, int endId, const BioPolymerModel &bpModel )
{
    static float radius = 1.5f;
    QVector<Vector3D> helixPoints;
    Color4f color(0,0,0,0);

    for (int i = startId; i <= endId; ++i) {
        if (!bpModel.monomerMap.contains(i))
            continue;
        helixPoints.append(bpModel.monomerMap.value(i).alphaCarbon->coord3d);

        // getting "average" color
        const Color4f c = colorScheme->getAtomColor(bpModel.monomerMap.value(i).alphaCarbon);
        for (int i = 0; i < 4; ++i) {
            color[i] += c[i];
        }
    }
    QPair<Vector3D, Vector3D> axis = calcBestAxisThroughPoints(helixPoints);

    // getting "average" color
    for (int i = 0; i < 4; ++i) {
        color[i] /= (endId - startId + 1);
    }

    return new Helix3D(color, axis.first, axis.second, radius);
}

Object3D* WormsGLRenderer::createStrand3D( int startId, int endId, const BioPolymerModel &bpModel )
{
    QVector<Vector3D> arrowPoints;
    Color4f color(0,0,0,0);

    for (int i = startId; i <= endId; ++i) {
        if (!bpModel.monomerMap.contains(i))
            continue;
        arrowPoints.append(bpModel.monomerMap.value(i).alphaCarbon->coord3d);

        // getting "average" color
        const Color4f c = colorScheme->getAtomColor(bpModel.monomerMap.value(i).alphaCarbon);
        for (int i = 0; i < 4; ++i) {
            color[i] += c[i];
        }
    }
    QPair<Vector3D, Vector3D> axis = calcBestAxisThroughPoints(arrowPoints);

    // getting "average" color
    for (int i = 0; i < 4; ++i) {
        color[i] /= (endId - startId + 1);
    }

    return new Strand3D(color, axis.first, axis.second); 
}

} //namespace
