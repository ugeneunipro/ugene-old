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
