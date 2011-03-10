#ifndef _DNA_ALPHABET_REGISTRY_IMPL_H_
#define _DNA_ALPHABET_REGISTRY_IMPL_H_

#include <U2Core/DNAAlphabet.h>

namespace U2 {

class DNATranslationRegistry;

class U2CORE_EXPORT DNAAlphabetRegistryImpl : public DNAAlphabetRegistry {
    Q_OBJECT
public:
    DNAAlphabetRegistryImpl(DNATranslationRegistry* tr);
    ~DNAAlphabetRegistryImpl();

    virtual bool registerAlphabet(DNAAlphabet* a);

    virtual void unregisterAlphabet(DNAAlphabet* a);

    virtual DNAAlphabet* findById(const QString id) const;

    virtual QList<DNAAlphabet*> getRegisteredAlphabets() const {return alphabets;}

    virtual DNAAlphabet* findAlphabet(const QByteArray& seq) const;

    virtual QList<DNAAlphabet*> findAlphabets(const QByteArray& seq) const;

    virtual QList<DNAAlphabet*> findAlphabets(const QByteArray& seq, const QVector<U2Region>& regionsToProcess, bool onlyOne) const;

private:
    void initBaseAlphabets();
    void initBaseTranslations();
    void reg4tables(const char* amino, const char* role, const char* n1, const char* n2, const char* n3,
        const QString& id, const QString& name);

    void regPtables(const char* amino, const int* prob, const char* n1, const char* n2, const char* n3,
        const QString& id, const QString& name);

    QList<DNAAlphabet*> alphabets;
    DNATranslationRegistry* treg;
};


} //namespace

#endif
