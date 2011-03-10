#ifndef MOLECULARSURFACEFACTORYREGISTRY_H
#define MOLECULARSURFACEFACTORYREGISTRY_H

#include "MolecularSurface.h"

namespace U2 {

class U2ALGORITHM_EXPORT MolecularSurfaceFactoryRegistry : public QObject
{
public:
    MolecularSurfaceFactoryRegistry(QObject* pOwn = 0);
    ~MolecularSurfaceFactoryRegistry();
    bool registerSurfaceFactory(MolecularSurfaceFactory* surf, const QString& surfId);
    bool hadRegistered(const QString& surfId);
    MolecularSurfaceFactory* getSurfaceFactory(const QString& surfId);
    QStringList getSurfNameList();
private:
    QMap<QString, MolecularSurfaceFactory*> surfMap;
    // Copy prohibition
    MolecularSurfaceFactoryRegistry(const MolecularSurfaceFactoryRegistry& m);
    MolecularSurfaceFactoryRegistry& operator=(const MolecularSurfaceFactoryRegistry& m);
};

} // namespace U2

#endif // MOLECULARSURFACEFACTORYREGISTRY_H
