#ifndef _U2_BIOSTRUCT3D_VANDERWAALS_RENDERER_H_
#define _U2_BIOSTRUCT3D_VANDERWAALS_RENDERER_H_

#include "BioStruct3DGLRender.h"

namespace U2 { 

class VanDerWaalsGLRenderer : public BioStruct3DGLRenderer {


    void drawAtoms(const BioStruct3DColorScheme* s);

    bool displayListsExist;

protected:
    VanDerWaalsGLRenderer(const BioStruct3D& struc, const BioStruct3DColorScheme* s, const QList<int> &shownModels, const BioStruct3DGLWidget *widget);

public:
    void drawBioStruct3D();

    virtual void updateColorScheme() {};
    virtual void updateShownModels() {};

    RENDERER_FACTORY(VanDerWaalsGLRenderer)
};

} //namespace

#endif // _U2_BIOSTRUCT3D_VANDERWAALS_RENDERER_H_
