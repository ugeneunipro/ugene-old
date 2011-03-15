/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

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
