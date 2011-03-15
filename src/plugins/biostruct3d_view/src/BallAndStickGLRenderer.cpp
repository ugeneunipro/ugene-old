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
unsigned int DisplayLists::bigDL = -1;

void BallAndStickGLRenderer::drawBioStruct3D(  )
{
    modelIndexList = glWidget->getActiveModelIndexList();
    drawAtomsAndBonds();
}

void BallAndStickGLRenderer::drawAtomsAndBonds()
{
    glCallList(bigDL);
}

BallAndStickGLRenderer::BallAndStickGLRenderer( const BioStruct3D& struc, const BioStruct3DColorScheme* s )
    : BioStruct3DGLRenderer(struc,s)
{
    bigDL = DisplayLists::getBigDisplayList();
}

void BallAndStickGLRenderer::updateColorScheme()
{   
    float detailLevel = glWidget->getRenderDetailLevel();
    DisplayLists::createBigDisplayList(detailLevel, bioStruct, glWidget->getActiveModelIndexList(),colorScheme);

}



} //namespace
