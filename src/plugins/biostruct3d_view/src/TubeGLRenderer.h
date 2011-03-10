#ifndef _U2_BIOSTRUCT3D_TUBE_RENDERER_H_
#define _U2_BIOSTRUCT3D_TUBE_RENDERER_H_

#include <QtCore/QSharedDataPointer>
#include <QtCore/QVector>
#include <U2Core/BioStruct3D.h>

#include "BioStruct3DGLRender.h"


namespace U2 {


class TubeGLRenderer : public BioStruct3DGLRenderer {
        
    typedef QVector< SharedAtom > AtomsVector;
    
    struct Tube {
        QMap<int, AtomsVector> modelsMap;
    }; 
    

    QMap<int, Tube> tubeMap;

    void drawTubes(const BioStruct3DColorScheme* colorScheme); 
public:
        
    TubeGLRenderer(const BioStruct3D& struc, const BioStruct3DColorScheme* s);
        
    void drawBioStruct3D();

    RENDERER_FACTORY(TubeGLRenderer)

};

} //namespace

#endif // _U2_BIOSTRUCT3D_TUBE_RENDERER_H_
