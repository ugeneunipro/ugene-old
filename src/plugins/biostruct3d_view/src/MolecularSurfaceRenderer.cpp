/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#include "MolecularSurfaceRenderer.h"
#include <GraphicUtils.h>
#include <U2Algorithm/MolecularSurface.h>

namespace U2 {

/* class MolecularSurfaceRendererRegistry */
const QString MolecularSurfaceRendererRegistry::defaultFactoryName() {
    return ConvexMapRenderer::ID;
}

const QList<QString> MolecularSurfaceRendererRegistry::factoriesNames() {
    return getInstance()->factories.keys();
}

const MolecularSurfaceRendererFactory* MolecularSurfaceRendererRegistry::getFactory(const QString &name) {
    return getInstance()->factories.value(name, 0);
}

MolecularSurfaceRenderer* MolecularSurfaceRendererRegistry::createMSRenderer(const QString &name) {
    const MolecularSurfaceRendererFactory *fact = getFactory(name);

    if (fact) {
        return fact->createInstance();
    }

    return 0;
}

MolecularSurfaceRendererRegistry::MolecularSurfaceRendererRegistry() {
    registerFactories();
}

MolecularSurfaceRendererRegistry* MolecularSurfaceRendererRegistry::getInstance() {
    static MolecularSurfaceRendererRegistry *reg = new MolecularSurfaceRendererRegistry();
    return reg;
}

#define REGISTER_FACTORY(c) factories.insert(c::ID, new c::Factory)
void MolecularSurfaceRendererRegistry::registerFactories() {
    REGISTER_FACTORY(DotsRenderer);
    REGISTER_FACTORY(ConvexMapRenderer);
}


const QString DotsRenderer::ID(QObject::tr("Dots"));
const QString ConvexMapRenderer::ID(QObject::tr("Convex Map"));

/* class DotsRenderer : public MolecularSurfaceRenderer */
void DotsRenderer::drawSurface( MolecularSurface& surface )
{
    QOpenGLFunctions_2_0 *f = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_2_0>();
    GLboolean ligting = glIsEnabled(GL_LIGHTING);
    f->glDisable(GL_LIGHTING);

    f->glPointSize(1.0f);
    f->glColor3f(1.0f, 1.0f, 1.0f);

    f->glBegin(GL_POINTS);
    foreach(const Face& face, surface.getFaces()) {
        float vct[3][3] = {
            {
                static_cast<float>(face.v[0].x),
                static_cast<float>(face.v[0].y),
                static_cast<float>(face.v[0].z),
            },
            {
                static_cast<float>(face.v[1].x),
                static_cast<float>(face.v[1].y),
                static_cast<float>(face.v[1].z),
            },
            {
                static_cast<float>(face.v[2].x),
                static_cast<float>(face.v[2].y),
                static_cast<float>(face.v[2].z),
            },
        };
        f->glVertex3fv(vct[0]);
        f->glVertex3fv(vct[1]);
        f->glVertex3fv(vct[2]);

        /*glVertex3f(face.v[0].x, face.v[0].y, face.v[0].z);
        glVertex3f(face.v[1].x, face.v[1].y, face.v[1].z);
        glVertex3f(face.v[2].x, face.v[2].y, face.v[2].z);*/
    }
    f->glEnd();

    if (ligting) glEnable(GL_LIGHTING);
    CHECK_GL_ERROR;
}


/* class ConvexMapRenderer : public MolecularSurfaceRenderer */
void ConvexMapRenderer::drawSurface( MolecularSurface& surface )
{
    QOpenGLFunctions_2_0 *f = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_2_0>();
    static GLfloat wall_mat[] = {1.f, 1.f, 1.f, 0.3f};
    f->glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, wall_mat);

    f->glBegin(GL_TRIANGLES);
    foreach(const Face& face, surface.getFaces()) {
        f->glNormal3f(face.n[0].x, face.n[0].y, face.n[0].z);
        f->glVertex3f(face.v[0].x, face.v[0].y, face.v[0].z);
        f->glNormal3f(face.n[1].x, face.n[1].y, face.n[1].z);
        f->glVertex3f(face.v[1].x, face.v[1].y, face.v[1].z);
        f->glNormal3f(face.n[2].x, face.n[2].y, face.n[2].z);
        f->glVertex3f(face.v[2].x, face.v[2].y, face.v[2].z);
    }
    f->glEnd( );
    CHECK_GL_ERROR;
}

} // namespace
