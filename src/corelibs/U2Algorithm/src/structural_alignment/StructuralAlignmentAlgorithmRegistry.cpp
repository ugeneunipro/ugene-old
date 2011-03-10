#include "StructuralAlignmentAlgorithmRegistry.h"
#include "StructuralAlignmentAlgorithmFactory.h"

namespace U2 {

/* class U2ALGORITHM_EXPORT StructuralAlignmentAlgorithmRegistry : public QObject */

StructuralAlignmentAlgorithmRegistry::StructuralAlignmentAlgorithmRegistry(QObject *parent)
    : QObject(parent), factories()
{
}

StructuralAlignmentAlgorithmRegistry::~StructuralAlignmentAlgorithmRegistry() {
    foreach (StructuralAlignmentAlgorithmFactory *f, factories) {
        delete f;
    }
}

void StructuralAlignmentAlgorithmRegistry::registerAlgorithmFactory(StructuralAlignmentAlgorithmFactory *factory, const QString &id) {
    assert(!factories.contains(id));
    factories.insert(id, factory);
}

StructuralAlignmentAlgorithmFactory* StructuralAlignmentAlgorithmRegistry::getAlgorithmFactory(const QString &id) {
    return factories.value(id, 0);
}

QList<QString> StructuralAlignmentAlgorithmRegistry::getFactoriesIds() const {
    return factories.keys();
}

}   // namespace U2
