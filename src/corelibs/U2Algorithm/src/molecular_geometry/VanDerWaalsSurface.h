#ifndef VANDERWAALSSURFACE_H
#define VANDERWAALSSURFACE_H

#include "MolecularSurface.h"

namespace U2 {

class U2ALGORITHM_EXPORT VanDerWaalsSurface : public MolecularSurface
{
public:
    VanDerWaalsSurface();
    qint64 estimateMemoryUsage(int numberOfAtoms);
    virtual void calculate(const QList<SharedAtom>& atoms, int& progress);
};

class U2ALGORITHM_EXPORT VanDerWaalsSurfaceFactory : public MolecularSurfaceFactory {
public:
    virtual MolecularSurface *createInstance()const;
};

} // namespace U2

#endif // VANDERWAALSSURFACE_H
