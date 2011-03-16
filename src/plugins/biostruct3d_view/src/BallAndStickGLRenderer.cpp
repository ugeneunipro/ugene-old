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
    drawAtomsAndBonds();
}

void BallAndStickGLRenderer::drawAtomsAndBonds()
{
    glCallList(bigDL);
}

BallAndStickGLRenderer::BallAndStickGLRenderer( const BioStruct3D& struc, const BioStruct3DColorScheme* s, const QList<int> &shownModels, const BioStruct3DGLWidget *widget )
    : BioStruct3DGLRenderer(struc,s,shownModels,widget)
{
    bigDL = DisplayLists::getBigDisplayList();
    updateColorScheme();
}

void BallAndStickGLRenderer::updateColorScheme()
{   
    float detailLevel = glWidget->getRenderDetailLevel();
    DisplayLists::createBigDisplayList(detailLevel, bioStruct, shownModels, colorScheme);

}

void BallAndStickGLRenderer::updateShownModels() {
    float detailLevel = glWidget->getRenderDetailLevel();
    DisplayLists::createBigDisplayList(detailLevel, bioStruct, shownModels, colorScheme);
}



} //namespace
