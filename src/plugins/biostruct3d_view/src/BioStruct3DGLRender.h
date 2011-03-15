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

#ifndef _U2_BIOSTRUCT3D_RENDERER_H_
#define _U2_BIOSTRUCT3D_RENDERER_H_

#include <QtCore/QString>
#include <QtCore/QMap>

namespace U2 { 

class BioStruct3D;
class BioStruct3DGLRenderer;
class BioStruct3DGlWidget;

//! BioStruct3DGLRenderer abstract factory
class BioStruct3DGLRendererFactory {
public:
    /*!
    * Constructor.
    */
    BioStruct3DGLRendererFactory() { }
    /*!
    * Destructor.
    */
    virtual ~BioStruct3DGLRendererFactory() { }
    /*!
    * @return Constructed glRenderer
    * @param bs Corresponding BioStruct3D.
    */
    virtual BioStruct3DGLRenderer* createInstance(const BioStruct3D& bs, const BioStruct3DColorScheme* s) = 0;
    /*!
    * @return Existing renderer factories associated with their names
    */
    static QMap<QString,BioStruct3DGLRendererFactory*> createFactories();
    /*!
    * @return Default renderer factory name
    */
    static const QString defaultFactoryName();
};

#define RENDERER_FACTORY(c) \
public: \
    static const QString ID; \
class Factory : public BioStruct3DGLRendererFactory { \
public: \
    Factory() { } \
    BioStruct3DGLRenderer* createInstance(const BioStruct3D& bs, const BioStruct3DColorScheme* s) { return new c(bs,s); } \
};

//! Abstract biological 3D structure OpenGL renderer
class BioStruct3DGLRenderer {

protected:
    const BioStruct3D& bioStruct;
    const BioStruct3DColorScheme* colorScheme;
    const BioStruct3DGLWidget* glWidget;

    unsigned int bigDL;

public:
    /*!
    * Constructor.
    */
    BioStruct3DGLRenderer(const BioStruct3D& _bioStruct, const BioStruct3DColorScheme* _s); 
    /*!
    * Destructor.
    */
    virtual ~BioStruct3DGLRenderer() { }
    /*!
    * Visualizes macromolecule.
    */
    virtual void drawBioStruct3D() = 0;
    /*!
    * Used to update current color scheme, whenever it is changed.
    */
    virtual void updateColorScheme();
    /*!
    * Sets new color scheme.
    */
    void setColorScheme(const BioStruct3DColorScheme* s);
};


} //namespace

#endif // _U2_BIOSTRUCT3D_RENDERER_H_
