/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#ifndef _U2_DNA_ALPHABET_H_
#define _U2_DNA_ALPHABET_H_

#include <U2Core/global.h>
#include <U2Core/U2Region.h>

#include <QtCore/QBitArray>
namespace U2 {

class U2CORE_EXPORT BaseDNAAlphabetIds : public QObject {
public:
    // functions, because are used in static variable initialization: Text2SequenceWorker.cpp
    static const QString RAW();
    static const QString NUCL_DNA_DEFAULT();
    static const QString NUCL_DNA_EXTENDED();
    static const QString NUCL_RNA_DEFAULT();
    static const QString NUCL_RNA_EXTENDED();
    static const QString AMINO_DEFAULT();
};



class U2CORE_EXPORT DNAAlphabet {

public:

    DNAAlphabet(const QString& id, const QString& name, DNAAlphabetType t, const QBitArray& map, Qt::CaseSensitivity caseMode, char defSym);

    const QBitArray& getMap() const {return map;}

    const QString& getName() const {return name;}

    const QString& getId() const {return id;}

    bool contains(char c) const {return map.at(c);}

    bool containsAll(const char* str, int len) const;

    DNAAlphabetType getType() const {return type;}

    bool isCaseSensitive() const {return caseMode == Qt::CaseSensitive;}

    char getDefaultSymbol() const {return defSym;}

    bool isRaw() const {return type == DNAAlphabet_RAW;}
    
    bool isNucleic() const {return type == DNAAlphabet_NUCL;}

    bool isAmino() const {return type == DNAAlphabet_AMINO;}

    bool isRNA() const {return id.contains("RNA");}
    
    // returns sorted array of all chars used in alphabet
    // forceBothCases == true : even case-insensitive alphabet will return both cases
    // forceBothCases == false: case-sensitivity in result depends on alphabets case-sensitivity
    QByteArray getAlphabetChars(bool forceBothCases = false) const;

    int getNumAlphabetChars() const {return numChars;}
    
private:
    QString             id;
    QString             name;
    DNAAlphabetType     type;
    QBitArray           map;
    Qt::CaseSensitivity caseMode;
    char                defSym;
    int                 numChars;
};

class U2CORE_EXPORT DNAAlphabetRegistry : public QObject {
public:
    virtual bool registerAlphabet(DNAAlphabet* a) = 0;

    virtual void unregisterAlphabet(DNAAlphabet* a) = 0;

    virtual DNAAlphabet* findById(const QString& id) const = 0;

    virtual QList<DNAAlphabet*> getRegisteredAlphabets() const =  0;
};

}//namespace

#endif

