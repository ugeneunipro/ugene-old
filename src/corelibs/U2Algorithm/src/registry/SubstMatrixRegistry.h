#ifndef _U2_SUBST_MATRIX_REGISTRY_H_
#define _U2_SUBST_MATRIX_REGISTRY_H_

#include <U2Core/global.h>
#include <U2Core/SMatrix.h>

#include <QtCore/QMutex>
#include <QtCore/QMap>
#include <QtCore/QStringList>

namespace U2 {

class U2ALGORITHM_EXPORT SubstMatrixRegistry : public QObject {
    Q_OBJECT
public:
    SubstMatrixRegistry(QObject* pOwn = 0);

    SMatrix getMatrix(const QString& name);

    QList<SMatrix> getMatrices() const;

    QStringList getMatrixNames() const;

    QList<SMatrix> selectMatricesByAlphabet(DNAAlphabet* al) const;
    
    QStringList selectMatrixNamesByAlphabet(DNAAlphabet* al) const;

    void registerMatrix(const SMatrix& m);

    static SMatrix readMatrixFromFile(const QString& fileName, QString& error);
    
private:
    void readMatrices();
    static SMatrix parseMatrix(const QString& name, const QByteArray& text, QString& error);

    mutable QMutex mutex;
    QMap<QString, SMatrix> matrixByName;
};

} // namespace

#endif
