#include <QtOpenGL>

#include "GraphicUtils.h"
#include "BioStruct3DColorScheme.h"
#include "BioStruct3DGLWidget.h"
#include "TubeGLRenderer.h"


namespace U2 { 

const QString TubeGLRenderer::ID(QObject::tr("Tubes"));

void TubeGLRenderer::drawTubes( const BioStruct3DColorScheme* colorScheme )
{
    GLUquadricObj *pObj;    // Quadric Object

    pObj = gluNewQuadric();
    gluQuadricNormals(pObj, GLU_SMOOTH);
    bool firstPass = true;
    static float ribbonThickness = 0.3f;
    SharedAtom bufAtom;
    
    foreach (Tube tube, tubeMap) {
        QList<int> modelsIndexList = glWidget->getActiveModelIndexList();
        foreach (int index, modelsIndexList) {
            const AtomsVector& tubeAtoms = tube.modelsMap.value(index);
            foreach(const SharedAtom atom, tubeAtoms) {
                Color4f atomColor = colorScheme->getAtomColor(atom);
                Vector3D pos = atom.constData()->coord3d;
                glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, atomColor.getConstData());
                glDrawAtom(pObj, pos, ribbonThickness, glWidget->getRenderDetailLevel());

                if (!firstPass) {
                    // Draw bonds only between atoms of the same molecular chain
                    if (atom.constData()->chainIndex == bufAtom.constData()->chainIndex) {
                        // ... and when they are sequential
                        if (atom.constData()->residueIndex - bufAtom.constData()->residueIndex == 1) { 
                            Vector3D bufPos = bufAtom.constData()->coord3d;
                            Color4f bufAtomColor = colorScheme->getAtomColor(bufAtom);
                            glDrawHalfBond(pObj, bufPos, pos, ribbonThickness, glWidget->getRenderDetailLevel());
                            glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, bufAtomColor.getConstData());
                            glDrawHalfBond(pObj, pos, bufPos, ribbonThickness, glWidget->getRenderDetailLevel());
                        }
                    }
                } else {
                    firstPass = false;
                }

                bufAtom = atom;
            } 
        }
    }

    gluDeleteQuadric(pObj);
}


void TubeGLRenderer::drawBioStruct3D()
{

    drawTubes(colorScheme);
}


TubeGLRenderer::TubeGLRenderer( const BioStruct3D& struc, const BioStruct3DColorScheme* s ) : BioStruct3DGLRenderer(struc,s)
{
    const char* alphaCarbonTag = "CA";
    const char* phosporTag = "P";
    
    foreach (const SharedMolecule mol, struc.moleculeMap) {
        int modelId = 0;
        foreach (const Molecule3DModel& model, mol->models) {
            foreach (const SharedAtom atom, model.atoms) {
                if ( (atom->name.trimmed() == alphaCarbonTag) || (atom->name.trimmed() == phosporTag)) {
                    tubeMap[atom->chainIndex].modelsMap[modelId].append(atom);
                }
            }
            ++modelId;    
        }
    }
    
}


} //namespace
