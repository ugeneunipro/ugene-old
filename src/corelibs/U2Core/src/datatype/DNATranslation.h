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

#ifndef _U2_DNA_TRANSLATION_H_
#define _U2_DNA_TRANSLATION_H_

#include <U2Core/global.h>
#include <QtCore/QStringList>

namespace U2 {

class DNAAlphabet;

enum DNATranslationType {
    DNATranslationType_UNKNOWN,
    DNATranslationType_NUCL_2_NUCL,
    DNATranslationType_NUCL_2_COMPLNUCL,
    DNATranslationType_NUCL_2_AMINO,
    DNATranslationType_AMINO_2_AMINO,
    DNATranslationType_AMINO_2_NUCL,
    DNATranslationType_RAW_2_NUCL,
    DNATranslationType_RAW_2_AMINO
};

class U2CORE_EXPORT BaseDNATranslationIds: public QObject {
public:
    static const QString NUCL_DNA_DEFAULT_COMPLEMENT;
    static const QString NUCL_RNA_DEFAULT_COMPLEMENT;

    static const QString NUCL_DNA_EXTENDED_COMPLEMENT;
    static const QString NUCL_RNA_EXTENDED_COMPLEMENT;
};


#define DNATranslationID(n) "NCBI-GenBank #"#n
#define DNABackTranslationID(type, name) "BackTranslation/"#type"/"#name
#define AMINO_TT_GOBJECT_HINT "AminoTT"

class U2CORE_EXPORT DNATranslation {
public:
    DNATranslation(const QString& _id, const QString& _name, DNAAlphabet* src, DNAAlphabet* dst);
    virtual ~DNATranslation(){}

    virtual int translate(const char* src, int src_len, char* dst, int dst_capacity) const = 0;
    virtual int translate(char* src_and_dst, int len) const = 0;

    const QString& getTranslationName() const {return name;}
    const QString& getTranslationId() const {return id;}

    DNATranslationType getDNATranslationType() const {return type;}

    DNAAlphabet* getSrcAlphabet() const {return srcAlphabet;}

    DNAAlphabet* getDstAlphabet() const {return dstAlphabet;}

    virtual bool isOne2One() const {return false;}
    
    virtual bool isThree2One() const {return false;}

       virtual bool isOne2Three() const {return false;}

    virtual QByteArray getOne2OneMapper() const {
        assert(0); return QByteArray(256, 0);
    }

    virtual char translate3to1(char c1, char c2, char c3) const {
        Q_UNUSED(c1); Q_UNUSED(c2); Q_UNUSED(c3);
        assert(0); return 0;
    }

    virtual char* translate1to3(char c) const {
        Q_UNUSED(c);
        assert(0); return NULL;
    }

protected:
    DNATranslationType type;
    QString name;
    QString id;
    DNAAlphabet* srcAlphabet;
    DNAAlphabet* dstAlphabet;
};


class U2CORE_EXPORT DNATranslationRegistry : public QObject {
public:
    DNATranslationRegistry(QObject* p=0) :QObject(p){}
    ~DNATranslationRegistry();

    QStringList getDNATranlations() const;

    QStringList getDNATranslationIds() const;

    void registerDNATranslation(DNATranslation* t);

    QList<DNATranslation*> lookupTranslation(DNAAlphabet* srcAlphabet, DNATranslationType type);

    DNATranslation* lookupTranslation(DNAAlphabet* srcAlphabet, DNATranslationType type, const QString& id);

    DNATranslation* lookupTranslation(const QString& id);

    DNATranslation* lookupComplementTranslation(DNAAlphabet* srcAlphabet);

private:
    QList<DNATranslation*> translations;
};

} //namespace

#endif

