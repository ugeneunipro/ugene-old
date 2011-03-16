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

#include <QtOpenGL>
#include <time.h>

#include <U2Core/BioStruct3D.h>
#include <U2Formats/PDBFormat.h>

#include "GraphicUtils.h"
#include "BioStruct3DGLWidget.h"
#include "BioStruct3DColorScheme.h"
#include "BallAndStickGLRenderer.h"

#include <U2Core/Log.h>

namespace U2 { 

const QString BallAndStickGLRenderer::ID(QObject::tr("Ball-and-stick"));

BallAndStickGLRenderer::BallAndStickGLRenderer(const BioStruct3D& struc, const BioStruct3DColorScheme* s, const QList<int> &shownModels, const BioStruct3DGLWidget *widget )
    : BioStruct3DGLRenderer(struc,s,shownModels,widget), dl(0)
{
    dl = glGenLists(1);
    createDisplayList();
}

BallAndStickGLRenderer::~BallAndStickGLRenderer() {
    glDeleteLists(dl, 1);
}

void BallAndStickGLRenderer::drawBioStruct3D() {
    glCallList(dl);
}

void BallAndStickGLRenderer::updateColorScheme() {
    createDisplayList();
}

void BallAndStickGLRenderer::updateShownModels() {
    createDisplayList();
}

static void drawAtomsBonds(const Color4f &viewAtomColor, float renderDetailLevel, const Molecule3DModel &model, const BioStruct3DColorScheme* colorScheme)
{
    GLUquadricObj *pObj = gluNewQuadric();
    gluQuadricNormals(pObj, GLU_SMOOTH);

    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, viewAtomColor.getConstData());

    static float bondThickness = 0.15f;
    float radius = 0.35f;
    int numSlices = 8 * renderDetailLevel;

    foreach(const SharedAtom atom, model.atoms) {
        Color4f atomColor = colorScheme->getAtomColor(atom);
        if (viewAtomColor==atomColor)
        {
            Vector3D pos = atom->coord3d;
            //glPushMatrix();
            glTranslatef(pos.x, pos.y, pos.z);
            gluSphere(pObj, radius, numSlices, numSlices);
            glTranslatef(-pos.x, -pos.y, -pos.z);
            //glPopMatrix();
        }
    }

    foreach (Bond bond, model.bonds) {
        const SharedAtom a1 = bond.getAtom1();
        const SharedAtom a2 = bond.getAtom2();

        const Color4f &a1Color = colorScheme->getAtomColor(a1);
        const Color4f &a2Color = colorScheme->getAtomColor(a2);

        Vector3D middle = (a1->coord3d + a2->coord3d) / 2;

        if (a1Color==viewAtomColor)
        {
            if (a1Color==a2Color)
            {
                //glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, a1Color);
                glDrawCylinder(pObj, a1->coord3d, a2->coord3d, bondThickness, renderDetailLevel);
            }
            else
            {
                //glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, a1Color);
                glDrawCylinder(pObj, a1->coord3d, middle, bondThickness, renderDetailLevel);
            }
        }
        if (a2Color == viewAtomColor)
        {
            //glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, a2Color);
            glDrawCylinder(pObj, middle, a2->coord3d, bondThickness, renderDetailLevel);
        }
    }

    gluDeleteQuadric(pObj);
}

void BallAndStickGLRenderer::createDisplayList()
{
    float renderDetailLevel = glWidget->getRenderDetailLevel();

    QList<Color4f> colors;

    glNewList(dl, GL_COMPILE);

    foreach (const SharedMolecule mol, bioStruct.moleculeMap) {
        foreach (int index, shownModels) {
            const Molecule3DModel& model = mol->models.at(index);

            colors.clear();

            foreach(const SharedAtom atom, model.atoms) {
                Color4f atomColor = colorScheme->getAtomColor(atom);

                if (colors.contains(atomColor))
                {
                    continue; // Atom and bonds with this color has been already viewed
                }
                else
                {
                    drawAtomsBonds(atomColor, renderDetailLevel, model, colorScheme);
                    colors.push_back(atomColor);
                }
            }
        }
    }

    glEndList();
}

} //namespace
