#ifndef PHYTREEGENERATORREGISTRY_H
#define PHYTREEGENERATORREGISTRY_H

#include "PhyTreeGenerator.h"

namespace U2 {

    class U2ALGORITHM_EXPORT PhyTreeGeneratorRegistry : public QObject
    {
    public:
        PhyTreeGeneratorRegistry(QObject* pOwn = 0);
        ~PhyTreeGeneratorRegistry();
        bool registerPhyTreeGenerator(PhyTreeGenerator* generator, const QString& gen_id);
        bool hadRegistered(const QString& surfId);
        PhyTreeGenerator* getGenerator(const QString& surfId);
        QStringList getNameList();
    private:
        QMap<QString, PhyTreeGenerator*> genMap;
        // Copy prohibition
        PhyTreeGeneratorRegistry(const PhyTreeGeneratorRegistry& m);
        PhyTreeGeneratorRegistry& operator=(const PhyTreeGeneratorRegistry& m);
    };

} // namespace U2

#endif
