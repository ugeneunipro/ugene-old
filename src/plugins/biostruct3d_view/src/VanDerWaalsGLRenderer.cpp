#include <QtOpenGL>
#include <time.h>

#include <U2Core/BioStruct3D.h>
#include <U2Core/Log.h>
#include <U2Algorithm/MolecularSurface.h>

#include "GraphicUtils.h"
#include "BioStruct3DGLWidget.h"
#include "BioStruct3DColorScheme.h"
#include "VanDerWaalsGLRenderer.h"


namespace U2 { 

const QString VanDerWaalsGLRenderer::ID(QObject::tr("Space Fill"));

//unsigned int DisplayLists::atomDL = -1;


void VanDerWaalsGLRenderer::drawBioStruct3D( )
{
    drawAtoms(colorScheme);
}


void VanDerWaalsGLRenderer::drawAtoms( const BioStruct3DColorScheme* colorScheme )
{
    static float tolerance = 0.45f;
    int numSlices = 10 * glWidget->getRenderDetailLevel();
    GLUquadricObj *pObj = gluNewQuadric();  
    gluQuadricNormals(pObj, GLU_SMOOTH);


    // Draw atoms as spheres

    foreach (const SharedMolecule mol, bioStruct.moleculeMap) {
        foreach (int index, shownModels) {
            const Molecule3DModel& model = mol->models.at(index);
            foreach(const SharedAtom atom, model.atoms) {
                float radius = AtomConstants::getAtomCovalentRadius(atom->atomicNumber);
                Vector3D pos = atom->coord3d;
                Color4f atomColor = colorScheme->getAtomColor(atom);
                glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, atomColor.getConstData());
                glPushMatrix();
                glTranslatef(pos.x, pos.y, pos.z);
                gluSphere(pObj, radius + tolerance, numSlices, numSlices);
                glPopMatrix();
            }
        }
    }
    gluDeleteQuadric(pObj);

}
  
VanDerWaalsGLRenderer::VanDerWaalsGLRenderer( const BioStruct3D& struc, const BioStruct3DColorScheme* s, const QList<int> &shownModels, const BioStruct3DGLWidget *widget)
    : BioStruct3DGLRenderer(struc,s,shownModels,widget), displayListsExist(false)
{
}
} //namespace
