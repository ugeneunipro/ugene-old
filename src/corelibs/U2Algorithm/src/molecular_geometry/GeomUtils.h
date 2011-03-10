#ifndef _U2_GEOM_UTILS_H_
#define _U2_GEOM_UTILS_H_

#include <QtCore/QVector>
#include <U2Core/Vector3D.h>
#include <memory>

namespace U2 { 

const float PI = 3.14159265f;
const float Rad2Deg = 57.2957795f;
const float Deg2Rad = 0.017453293f;


struct U2ALGORITHM_EXPORT Face {
    Face() {}
    Vector3D v[3];
    Vector3D n[3];
};

//! Builds sphere
class U2ALGORITHM_EXPORT GeodesicSphere {
    QVector<Vector3D> vertices;
    QVector<Face> faces;
    static std::auto_ptr< QVector<Vector3D> > elementarySphere;
    static int currentDetailLevel;
    static void interpolate(const Vector3D& v1, const Vector3D& v2, const Vector3D& v3, QVector<Vector3D>* v, int detailLevel);
public:
    GeodesicSphere(const Vector3D& center, float radius, int detaillevel);
    static QVector<Vector3D>* createGeodesicSphere(int detailLevel);
    QVector<Vector3D> getVertices()  { return vertices; }
    QVector<Face> getFaces() { return faces; }
};



} //namespace

#endif // _U2_GEOM_UTILS_H_

