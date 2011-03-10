#ifndef _U2_DNA_SEQUENCE_H_
#define _U2_DNA_SEQUENCE_H_

#include <U2Core/global.h>
#include "DNAInfo.h"
#include "DNAQuality.h"

#include <QtCore/QByteArray>

namespace U2 {

class DNAAlphabet;

class U2CORE_EXPORT DNASequence {
public:
    DNASequence(const QString& name, const QByteArray& s = QByteArray(), DNAAlphabet* a = NULL);
    DNASequence(const QByteArray& s = QByteArray(), DNAAlphabet* a = NULL) : seq(s), alphabet(a), circular(false), quality(NULL) {}
 
    QVariantMap     info;
    QByteArray      seq;
    DNAAlphabet*    alphabet;
    bool            circular;
    DNAQuality      quality;
    
    QString getName() const {return DNAInfo::getName(info);}
    void setName(const QString& name);
    bool isNull() const {return !alphabet && seq.length() == 0;}
    int length() const {return seq.length();}
    bool hasQualityScores() const {return (!quality.isEmpty()); }
    const char* constData() const {return seq.constData();}
    const QByteArray &constSequence() const {return seq;}
};

}//namespace

Q_DECLARE_METATYPE(U2::DNASequence)
Q_DECLARE_METATYPE(QList<U2::DNASequence>)

#endif
