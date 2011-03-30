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

#include "DNATranslation.h"
#include "DNAAlphabet.h"

namespace U2 {

DNATranslation::DNATranslation(const QString& _id, const QString& _name, DNAAlphabet* src, DNAAlphabet* dst) {
    name = _name;
    id = _id;
    srcAlphabet = src;
    dstAlphabet = dst;

    assert(srcAlphabet!=NULL && dstAlphabet!=NULL);

    DNAAlphabetType srcType = srcAlphabet->getType();
    DNAAlphabetType dstType = dstAlphabet->getType();
    type = DNATranslationType_UNKNOWN;

    if (srcType == DNAAlphabet_NUCL) {
        switch(dstType) {
            case DNAAlphabet_NUCL :
                type = dstAlphabet == srcAlphabet ? DNATranslationType_NUCL_2_COMPLNUCL : DNATranslationType_NUCL_2_NUCL; break;
            case DNAAlphabet_AMINO:
                type = DNATranslationType_NUCL_2_AMINO; break;
                        default: assert(0); break;
        }
    } else if (srcType == DNAAlphabet_AMINO) {
        switch(dstType) {
            case DNAAlphabet_AMINO :
                type = DNATranslationType_AMINO_2_AMINO; break;
            case DNAAlphabet_NUCL :
                type = DNATranslationType_AMINO_2_NUCL; break;
            default: assert(0); break;
        }
    } else if (srcType == DNAAlphabet_RAW) {
        switch(dstType) {
            case DNAAlphabet_NUCL :
                type = DNATranslationType_RAW_2_NUCL; break;
            case DNAAlphabet_AMINO:
                type = DNATranslationType_RAW_2_AMINO; break;
                        default: assert(0); break;
        }
    }
    assert(type!=DNATranslationType_UNKNOWN);
}


//////////////////////////////////////////////////////////////////////////
/// Registry

QStringList DNATranslationRegistry::getDNATranlations() const {
    QStringList l;
    foreach(DNATranslation* t, translations) {
        l<<t->getTranslationName();
    }
    return l;
}


QStringList DNATranslationRegistry::getDNATranslationIds() const
{
    QStringList l;
    foreach(DNATranslation* t, translations) {
        l<<t->getTranslationId();
    }
    return l;

}


void DNATranslationRegistry::registerDNATranslation(DNATranslation* t) {
    translations.push_back(t);
}

DNATranslationRegistry::~DNATranslationRegistry() {
    foreach(DNATranslation* t, translations) {
        delete t;
    }
    translations.clear();
}

QList<DNATranslation*> DNATranslationRegistry::lookupTranslation(DNAAlphabet* srcAlphabet, DNATranslationType type) {
    QList<DNATranslation*> res;
    foreach(DNATranslation* t, translations) {
        if (t->getSrcAlphabet() == srcAlphabet && t->getDNATranslationType() == type) {
            res.append(t);
        }
    }
    return res;
}

DNATranslation* DNATranslationRegistry::lookupTranslation(DNAAlphabet* srcAlphabet, 
                                                          DNATranslationType type,
                                                          const QString& id) 
{
    foreach(DNATranslation* t, translations) {
        if (t->getTranslationId() == id && t->getSrcAlphabet() == srcAlphabet && t->getDNATranslationType() == type) {
            return t;
        }
    }
    return NULL;
}

DNATranslation* DNATranslationRegistry::lookupTranslation(const QString& id) {
    foreach(DNATranslation* t, translations) {
        if (t->getTranslationId() == id) {
            return t;
        }
    }
    return NULL;
}

DNATranslation* DNATranslationRegistry::lookupComplementTranslation(DNAAlphabet* srcAlphabet) {
    assert(srcAlphabet->isNucleic());
    QList<DNATranslation*> complTs = lookupTranslation(srcAlphabet, DNATranslationType_NUCL_2_COMPLNUCL);
    if (complTs.isEmpty()) {
        return NULL;
    }
    return complTs.first();

}

}//namespace
