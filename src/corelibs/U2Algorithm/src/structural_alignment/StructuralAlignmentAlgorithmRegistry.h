#ifndef _U2_STRUCTURAL_ALIGNMENT_ALGORITHM_REGISTRY_H_
#define _U2_STRUCTURAL_ALIGNMENT_ALGORITHM_REGISTRY_H_

#include <QObject>
#include <U2Core/global.h>

namespace U2 {

class StructuralAlignmentAlgorithmFactory;

class U2ALGORITHM_EXPORT StructuralAlignmentAlgorithmRegistry : public QObject {
    Q_OBJECT

public:
    StructuralAlignmentAlgorithmRegistry(QObject *parent = 0);
    ~StructuralAlignmentAlgorithmRegistry();

    void registerAlgorithmFactory(StructuralAlignmentAlgorithmFactory *factory, const QString &id);
    StructuralAlignmentAlgorithmFactory* getAlgorithmFactory(const QString &id);

    QList<QString> getFactoriesIds() const;

private:
    StructuralAlignmentAlgorithmRegistry(const StructuralAlignmentAlgorithmRegistry&);
    StructuralAlignmentAlgorithmRegistry& operator= (const StructuralAlignmentAlgorithmRegistry&);

private:
    QMap<QString, StructuralAlignmentAlgorithmFactory*> factories;
};  // class StructuralAlignmentAlgorithmRegistry

}   // namespace U2

#endif  // #ifndef _U2_STRUCTURAL_ALIGNMENT_ALGORITHM_REGISTRY_H_
