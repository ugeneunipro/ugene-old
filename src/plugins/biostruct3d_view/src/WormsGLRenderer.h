#ifndef _U2_BIOSTRUCT3D_WORMS_RENDERER_H_
#define _U2_BIOSTRUCT3D_WORMS_RENDERER_H_

#include <QtCore/QSharedDataPointer>
#include <QtCore/QVector>
#include <QtCore/QMap>
#include <QtCore/QPair>

#include <U2Core/Vector3D.h>
#include "BioStruct3DGLRender.h"
#include "GraphicUtils.h"

class Object3D;
class AtomData;

namespace U2 { 

    class WormsGLRenderer : public BioStruct3DGLRenderer {

        
        // BioPolymer molecule
        
        struct Monomer 
        {
            Monomer() : alphaCarbon(NULL), carbonylOxygen(NULL) {}
            QSharedDataPointer<AtomData> alphaCarbon;
            QSharedDataPointer<AtomData> carbonylOxygen;
        };

        struct BioPolymerModel 
        {
            QMap<int,Monomer> monomerMap;
        };
        
        struct BioPolymer 
        {
            // multiple models
            QVector<BioPolymerModel> bpModels;
        };
        
        QMap<int, BioPolymer > bioPolymerMap;


        // Worm Model
        
        typedef QVector<SharedAtom> AtomsVector;
        
        struct WormModel {
            //! These coords required to draw worms endings correctly
            Vector3D openingAtom, closingAtom;
            // Worm building atom coords
            AtomsVector atoms;
            // Objects representing secondary structure
            QVector<Object3D*>  objects;
        };
        
        struct Worm {
            QVector<WormModel> models;
        };

        QMap<int, Worm> wormMap;
        Color4f atomColor;
        

        void createObjects3D();
        Object3D* createStrand3D( int startId, int endId, const BioPolymerModel &bpModel );
        Object3D* createHelix3D( int startId, int endId, const BioPolymerModel &bpModel );
        void createWorms();
        void drawWorms(); 
        void drawSecondaryStructure();
        const float* getAtomColor(const SharedAtom& atom);

    public:
        
        WormsGLRenderer(const BioStruct3D& struc, const BioStruct3DColorScheme* s);
        
        ~WormsGLRenderer();
        
        virtual void drawBioStruct3D();
        
        virtual void updateColorScheme();
        
        RENDERER_FACTORY(WormsGLRenderer)

    };

} //namespace

#endif // _U2_BIOSTRUCT3D_WORMS_RENDERER_H_
