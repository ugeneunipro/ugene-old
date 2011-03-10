#ifndef SAS_H
#define SAS_H

#include <U2Algorithm/MolecularSurface.h>

namespace U2
{

class SolventAccessibleSurface : public MolecularSurface
{
public:
    SolventAccessibleSurface();
    virtual qint64 estimateMemoryUsage(int numberOfAtoms);
    virtual void calculate(const QList<SharedAtom>& atoms, int& progress);
};

class SolventAccessibleSurfaceFactory : public MolecularSurfaceFactory
{
public:
    virtual MolecularSurface *createInstance()const;
};

} // namespace

#endif // SAS_H
