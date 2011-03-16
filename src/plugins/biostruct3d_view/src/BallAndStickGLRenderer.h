#ifndef _U2_BIOSTRUCT3D_BALLANDSTICK_RENDERER_H_
#define _U2_BIOSTRUCT3D_BALLANDSTICK_RENDERER_H_

#include "BioStruct3DGLRender.h"
#include <QtOpenGL>

namespace U2 { 

class BallAndStickGLRenderer : public BioStruct3DGLRenderer {

        
    void drawAtomsAndBonds();
    
protected:
    BallAndStickGLRenderer(const BioStruct3D& struc, const BioStruct3DColorScheme* s, const QList<int> &shownModels, const BioStruct3DGLWidget *widget);

public:
    void drawBioStruct3D();
    
    virtual void updateColorScheme();
    virtual void updateShownModels();

    RENDERER_FACTORY(BallAndStickGLRenderer)   
};

} //namespace

#endif // _U2_BIOSTRUCT3D_BALLANDSTICK_RENDERER_H_
