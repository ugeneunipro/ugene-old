#ifndef _U2_POSITION_FREQUENCY_MATRIX_H_
#define _U2_POSITION_FREQUENCY_MATRIX_H_

#include <U2Core/global.h>

#include <U2Core/MAlignment.h>
#include <U2Core/DNASequence.h>

#include <QtCore/QVarLengthArray>
#include <QtCore/QList>
#include <QtCore/QMap>

namespace U2 {

//info for JASPAR matrices, contained in matrix_list.txt
class U2CORE_EXPORT JasparInfo {
public:
    //default empty constructor
    JasparInfo();

    //constructor from parsed data
    JasparInfo(const QMap<QString, QString>& properties);

    //constructor from matrix_list.txt string
    JasparInfo(const QString& line);

    QString getProperty (const QString& name) const;

    QMap<QString, QString> getProperties() const;

private:
    QMap<QString, QString> properties;
};

//Type of Position frequency matrix
enum PFMatrixType {
    PFM_MONONUCLEOTIDE,
    PFM_DINUCLEOTIDE
};

//Position frequency matrix
class U2CORE_EXPORT PFMatrix {
public:
    //create empty matrix
    PFMatrix() : data(QVarLengthArray<int>()), length(0), type(PFM_MONONUCLEOTIDE) {};

    //create matrix from pre-counted data
    PFMatrix(const QVarLengthArray<int>& data, const PFMatrixType type);

    //create matrix from alignment (without gaps)
    PFMatrix(const MAlignment& data, const PFMatrixType type);

    //create matrix from set of sequences with equal length
    PFMatrix(const QList<DNASequence*>& data, const PFMatrixType type);

    //get internal index of position in 1-dimensional array
    int index (int row, int column) const;

    //get length of matrix row
    int getLength () const;

    //get type of matrix: mono- or dinucleotide
    PFMatrixType getType() const;

    //get value at specified position
    int getValue (int row, int column) const;

    //set JASPAR info for matrix
    void setInfo (const JasparInfo& info);

    //get specified Jaspar property
    QString getProperty (const QString& propertyName) const;
    
    //get all Jaspar properties
    QMap<QString, QString> getProperties() const;

    //convert dinucleotide matrix to mononucleotide one
    static PFMatrix convertDi2Mono(const PFMatrix& source);

private:
    QVarLengthArray<int> data;
    int length;
    PFMatrixType type;
    JasparInfo info;
};

}//namespace

#endif
