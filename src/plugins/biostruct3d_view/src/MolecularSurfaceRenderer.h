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

#ifndef _U2_MOLECULAR_SURFACE_RENDERER_H_
#define _U2_MOLECULAR_SURFACE_RENDERER_H_

#include <QtCore/QString>
#include <QtCore/QMap>

namespace U2 {

class MolecularSurface;
class MolecularSurfaceRenderer;

class MolecularSurfaceRendererFactory {
public:
    MolecularSurfaceRendererFactory() { }
    virtual ~MolecularSurfaceRendererFactory() { }
    virtual MolecularSurfaceRenderer* createInstance() = 0;
    static QMap<QString,MolecularSurfaceRendererFactory*> createFactories();
};

#define SURF_RENDERER_FACTORY(c) \
public: \
    static const QString ID; \
class Factory : public MolecularSurfaceRendererFactory { \
public: \
    Factory() { } \
    MolecularSurfaceRenderer* createInstance() { return new c; } \
};

class MolecularSurfaceRenderer {
public:
    MolecularSurfaceRenderer();
    virtual void drawSurface(MolecularSurface& surface) = 0;
};

class DotsRenderer : public MolecularSurfaceRenderer{
public:
    DotsRenderer() {}
    virtual void drawSurface(MolecularSurface& surface);
    SURF_RENDERER_FACTORY(DotsRenderer)
};

class ConvexMapRenderer : public MolecularSurfaceRenderer {
public:
    ConvexMapRenderer() {}
    virtual void drawSurface(MolecularSurface& surface);
    SURF_RENDERER_FACTORY(ConvexMapRenderer)
};



} //namespace

#endif //_U2_MOLECULAR_SURFACE_RENDERER_H_

