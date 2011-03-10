#ifndef _U2_GRAPHIC_UTILS_H_
#define _U2_GRAPHIC_UTILS_H_

#include <U2Core/Vector3D.h>

class GLUquadric;

namespace U2 { 

class BioStruct3D;
class BioStruct3DColorScheme;
class Molecule3DModel;

//! Color
/*!
    This class is convenient for OpenGL rendering purposes, connected with QColor
*/
class Color4f {
private:
    float color[4];

public:
    Color4f();
    Color4f(float r, float g, float b, float a = 1.0);
    Color4f(const Color4f &c);
    Color4f(const QColor &qc);

    float operator[] (unsigned int i) const;
    float& operator[] (unsigned int i);

    bool operator == (const Color4f &a) const;
    const float* getConstData() const { return color; }
};

//! OpenGL drawable object
class Object3D {
protected:
    Color4f color;

public:
    Object3D(const Color4f& color_) : color(color_) {};
    virtual void draw(float renderDetailLevel) = 0;
    virtual ~Object3D() {};

    const Color4f& getColor() const { return color; }
    void setColor(const Color4f &c) { color = Color4f(c); }

};  // class Strand3D

//! Alpha Helix 3D representation
class Helix3D : public Object3D {
    GLUquadric* pObj;
    Vector3D cterm;
    Vector3D nterm;
    Vector3D rotAxis;
    float radius,length,rotAngle;

public:
    Helix3D(const Color4f& color, const Vector3D& c, const Vector3D& n, float r);
    virtual ~Helix3D();

    virtual void draw(float renderDetailLevel);
};  // class Helix3D

//! Beta Strand 3D representation
class Strand3D : public Object3D {
    GLUquadric* pObj;
    Vector3D cterm;
    Vector3D nterm;
    Vector3D upVector;
    Vector3D rotAxis;
    float length, rotAngle;
public:
    Strand3D(const Color4f& color, const Vector3D& c, const Vector3D& n, const Vector3D& up = Vector3D() );
    virtual ~Strand3D();

    virtual void draw(float renderDetailLevel);
};  // class Strand3D


//! Draw arrow
void glCreateArrowPrimitive(float width, float height, float length);
//! Draw atom as sphere with center = pos and radius = r
void glDrawAtom(GLUquadric* pObj, const Vector3D& pos, double r, float renderDetailLevel);

//! Draw cylinder from one point to another
void glDrawCylinder(GLUquadric* pObj, const Vector3D& p1, const Vector3D& p2, double thickness, float renderDetailLevel);

//! Draw a spline (worm) 
/*! Function straight from CN3D Viewer, uses Catmull-Rom interpolation
add a thick splined curve from point 1 *halfway* to point 2 */
void glDrawHalfWorm(const Vector3D& p0, const Vector3D& p1, const Vector3D& p2, const Vector3D& p3,
                  double radius, bool cap1, bool cap2, double tension, float renderDetailLevel);

void glDrawHalfBond(GLUquadric *pObj, const Vector3D& p1, const Vector3D&p2, double thickness, float renderDetailLevel);

//! Project point on axis
Vector3D projectPointOnAxis(const Vector3D& point, const Vector3D& axisUnitVector, const Vector3D& axisPoint);

//! Calculates average point
Vector3D calcMiddlePoint(const QVector<Vector3D>& points);

class DisplayLists {
    static unsigned int bigDL;
    static unsigned int vanDerWaalsDL;
    static const int BIG_DL_INDEX = 1;
    static const int VANDERWAALS_DL_INDEX = 2;

    static void drawAtomsBonds(const Color4f &viewAtomColor, float renderDetailLevel, const Molecule3DModel &model, const BioStruct3DColorScheme* colorScheme);

public:
    static void createBigDisplayList(float renderDetailLevel, const BioStruct3D& bioStruct, const QList<int> &modelIndexList, const BioStruct3DColorScheme* colorScheme);
    static int getBigDisplayList() { return bigDL; }
};


void accPerspective(double fovy, double aspect,
                    double near, double far, double pixdx, double pixdy,
                    double eyedx, double eyedy, double focus);
void accFrustum(double left, double right, double bottom,
                double top, double near, double far, double pixdx, 
                double pixdy, double eyedx, double eyedy, 
                double focus);


//! Calculates axis through points using least linear square method
QPair<Vector3D,Vector3D> calcBestAxisThroughPoints(const QVector<Vector3D>& points);




} //namespace

#endif // _U2_GRAPHIC_UTILS_H_

