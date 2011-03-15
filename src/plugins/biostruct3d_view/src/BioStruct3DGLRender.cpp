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

#include "BioStruct3DColorScheme.h"
#include "BioStruct3DGLRender.h"
#include "BallAndStickGLRenderer.h"
#include "TubeGLRenderer.h"
#include "VanDerWaalsGLRenderer.h"
#include "WormsGLRenderer.h"

namespace U2 { 

#define REGISTER_FACTORY(c) \
    map.insert(c::ID, new c::Factory)

QMap<QString,BioStruct3DGLRendererFactory*> BioStruct3DGLRendererFactory::createFactories()
{
    QMap<QString,BioStruct3DGLRendererFactory*> map;
    REGISTER_FACTORY(BallAndStickGLRenderer);
    REGISTER_FACTORY(TubeGLRenderer);
    REGISTER_FACTORY(VanDerWaalsGLRenderer);
    REGISTER_FACTORY(WormsGLRenderer);
    return map;
}

const QString BioStruct3DGLRendererFactory::defaultFactoryName()
{
    return WormsGLRenderer::ID;
}



void BioStruct3DGLRenderer::updateColorScheme()
{
    
}

BioStruct3DGLRenderer::BioStruct3DGLRenderer( const BioStruct3D& _bioStruct, const BioStruct3DColorScheme* _s )  :  
    bioStruct(_bioStruct), colorScheme(_s), glWidget(_s->getBioStruct3DGlWidget())
{
    bigDL = -1;
}

void BioStruct3DGLRenderer::setColorScheme( const BioStruct3DColorScheme* s )
{
     colorScheme = s;
     updateColorScheme();
}

} //namespace
