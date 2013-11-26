/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#ifndef _U2_MSA_COLOR_SCHEME_H_
#define _U2_MSA_COLOR_SCHEME_H_

#include <U2Core/global.h>

#include <QtCore/QVector>
#include <QtGui/QColor>

namespace U2 {

class MAlignmentObject;
class MSAColorScheme;
class MAlignment;
class MAlignmentModInfo;

//////////////////////////////////////////////////////////////////////////
// factories

class U2VIEW_EXPORT MSAColorSchemeFactory : public QObject {
    Q_OBJECT
public:
    MSAColorSchemeFactory(QObject* p, const QString& id, const QString& name, DNAAlphabetType atype);
    virtual MSAColorScheme* create(QObject* p, MAlignmentObject* obj) = 0;

    const QString& getId() const {return id;}
    const QString& getName() const {return name;}
    DNAAlphabetType getAlphabetType() const {return aType;}

private:
    QString         id;
    QString         name;
    DNAAlphabetType aType;
};

class U2VIEW_EXPORT MSAColorSchemeStaticFactory : public MSAColorSchemeFactory {
    Q_OBJECT
public:
    MSAColorSchemeStaticFactory(QObject* p, const QString& id, const QString& name, DNAAlphabetType atype, const QVector<QColor>& colorsPerChar);
    virtual MSAColorScheme* create(QObject* p, MAlignmentObject* obj);
private:
    QVector<QColor> colorsPerChar;
};

class U2VIEW_EXPORT MSAColorSchemeCustomSettingsFactory : public MSAColorSchemeFactory {
    Q_OBJECT
public:
    MSAColorSchemeCustomSettingsFactory(QObject* p, const QString& id, const QString& name, DNAAlphabetType atype, const QVector<QColor>& colorsPerChar);
    virtual MSAColorScheme* create(QObject* p, MAlignmentObject* obj);
private:
    QVector<QColor> colorsPerChar;
};

class MSAColorSchemePercIdentFactory : public MSAColorSchemeFactory {
    Q_OBJECT
public:
    MSAColorSchemePercIdentFactory(QObject* p, const QString& id, const QString& name, DNAAlphabetType atype);
    virtual MSAColorScheme* create(QObject* p, MAlignmentObject* obj);
};

class MSAColorSchemeClustalXFactory : public MSAColorSchemeFactory {
    Q_OBJECT
public:
    MSAColorSchemeClustalXFactory(QObject* p, const QString& id, const QString& name, DNAAlphabetType atype);
    virtual MSAColorScheme* create(QObject* p, MAlignmentObject* obj);
};


//////////////////////////////////////////////////////////////////////////
// schemes

class U2VIEW_EXPORT MSAColorScheme : public QObject {
    Q_OBJECT
public:
    MSAColorScheme(QObject* p, MSAColorSchemeFactory* f, MAlignmentObject* o);
    virtual QColor getColor(int seq, int pos) = 0;
    MSAColorSchemeFactory* getFactory() const {return factory;}

    static QString EMPTY_NUCL;
    static QString UGENE_NUCL;
    static QString JALVIEW_NUCL;
    static QString IDENTPERC_NUCL;
    static QString CUSTOM_NUCL;

    static QString EMPTY_AMINO;
    static QString UGENE_AMINO;
    static QString ZAPPO_AMINO;
    static QString TAILOR_AMINO;
    static QString HYDRO_AMINO;
    static QString HELIX_AMINO;
    static QString STRAND_AMINO;
    static QString TURN_AMINO;
    static QString BURIED_AMINO;
    static QString IDENTPERC_AMINO;
    static QString CLUSTALX_AMINO;
    static QString CUSTOM_AMINO;

protected:
    MSAColorSchemeFactory*  factory;
    MAlignmentObject*       maObj;
};

class U2VIEW_EXPORT MSAColorSchemeStatic : public MSAColorScheme {
    Q_OBJECT
public:
    MSAColorSchemeStatic(QObject* p, MSAColorSchemeFactory* f, MAlignmentObject* o, const QVector<QColor>& colorsPerChar);
    virtual QColor getColor(int seq, int pos);
    const QColor& getColor(char c) const {return colorsPerChar[(quint8)c];}
private:

    QVector<QColor> colorsPerChar;
};


//PERCENT
class U2VIEW_EXPORT MSAColorSchemePercIdent : public MSAColorScheme {
    Q_OBJECT
public:
    MSAColorSchemePercIdent(QObject* p, MSAColorSchemeFactory* f, MAlignmentObject* o);
    virtual QColor getColor(int seq, int pos);

private slots:
    void sl_alignmentChanged(const MAlignment&, const MAlignmentModInfo&) {objVersion++;}

protected:
    void updateCache();

    QVector<quint32>    indentCache;
    int                 cacheVersion;
    int                 objVersion;
    QColor              colorsByRange[4];
    int                 mask4[4];
    char                tmpChars[4];
    int                 tmpRanges[4];
};

//CLUSTALX
// 0.5 * alisize mem use, slow update
class U2VIEW_EXPORT MSAColorSchemeClustalX: public MSAColorScheme {
    Q_OBJECT
public:
    MSAColorSchemeClustalX(QObject* p, MSAColorSchemeFactory* f, MAlignmentObject* o);
    virtual QColor getColor(int seq, int pos);
private slots:
    void sl_alignmentChanged(const MAlignment&, const MAlignmentModInfo&) {objVersion++;}

protected:
    void updateCache();
    int getCacheIdx(int seq, int pos, bool& low) const {
        assert(objVersion == cacheVersion); 
        int res = seq * aliLen + pos;
        low = !(res & 0x1);
        return res / 2;
    }

    int getColorIdx(int seq, int pos);
    void setColorIdx(int seq, int pos, int cidx);

    enum ClustalColor {
        ClustalColor_NO_COLOR,
        ClustalColor_BLUE,
        ClustalColor_RED,
        ClustalColor_GREEN,
        ClustalColor_PINK,
        ClustalColor_MAGENTA,
        ClustalColor_ORANGE,
        ClustalColor_CYAN,
        ClustalColor_YELLOW,
        ClustalColor_NUM_COLORS
    };
    int              objVersion;
    int              cacheVersion;
    int              aliLen;
    QVector<quint8>  colorsCache;
    QColor           colorByIdx[ClustalColor_NUM_COLORS];
};

//////////////////////////////////////////////////////////////////////////
// registry
class U2VIEW_EXPORT MSAColorSchemeRegistry : public QObject {
    Q_OBJECT
public:
    MSAColorSchemeRegistry();
    ~MSAColorSchemeRegistry();

    const QList<MSAColorSchemeFactory*>& getMSAColorSchemes() const {return colorers;}
    const QList<MSAColorSchemeFactory*>& getCustomColorSchemes() const {return customColorers;}
    
    QList<MSAColorSchemeFactory*> getMSAColorSchemes(DNAAlphabetType atype) const;
    QList<MSAColorSchemeFactory*> getMSACustomColorSchemes(DNAAlphabetType atype) const;

    MSAColorSchemeFactory* getMSAColorSchemeFactoryById(const QString& id) const;

    void addMSAColorSchemeFactory(MSAColorSchemeFactory* cs);
    void addMSACustomColorSchemeFactory(MSAColorSchemeFactory* cs);
signals:
    void si_customSettingsChanged();
private slots:
   void sl_onCustomSettingsChanged();
private:
    void deleteOldCustomFactories();
    void initBuiltInSchemes();
    void initCustomSchema();

    QList<MSAColorSchemeFactory*> colorers;
    QList<MSAColorSchemeFactory*> customColorers;
};


//////////////////////////////////////////////////////////////////////////
// SNP highitning

class MSAHighlightingSchemeFactory;
class MSAHighlightingScheme;
class MSAHighlightingSchemeEmpty;
class MSAHighlightingSchemeAgreements;
class MSAHighlightingSchemeDisagreements;
class MSAHighlightingSchemeTransitions;
class MSAHighlightingSchemeTransversions;

class U2VIEW_EXPORT MSAHighlightingSchemeRegistry : public QObject {
    Q_OBJECT
public:
    MSAHighlightingSchemeRegistry();

    MSAHighlightingSchemeFactory* getMSAHighlightingSchemeFactoryById(const QString& id) const;
    QList<MSAHighlightingSchemeFactory*> getMSAHighlightingSchemes(DNAAlphabetType atype);

private:   
    QList<MSAHighlightingSchemeFactory*> schemes;
};

class U2VIEW_EXPORT MSAHighlightingSchemeFactory : public QObject {
    Q_OBJECT
public:
    MSAHighlightingSchemeFactory(QObject* p, const QString& id, const QString& name, DNAAlphabetType atype, bool _refFree = false )
        :QObject(p), id(id), name(name), aType(atype), refFree(_refFree){};
    virtual MSAHighlightingScheme* create(QObject* p, MAlignmentObject* obj) = 0;

    const QString& getId() const {return id;}
    const QString& getName() const {return name;}
    DNAAlphabetType getAlphabetType() const {return aType;}
    bool isRefFree(){return refFree;};

private:
    QString         id;
    QString         name;
    DNAAlphabetType aType;
    bool            refFree;
};

class U2VIEW_EXPORT MSAHighlightingSchemeNoColorsFactory : public MSAHighlightingSchemeFactory {
public:
    MSAHighlightingSchemeNoColorsFactory(QObject* p, const QString& id, const QString& name, DNAAlphabetType atype):
        MSAHighlightingSchemeFactory(p, id, name, atype, true){};
    virtual MSAHighlightingScheme* create(QObject* p, MAlignmentObject* obj);
};

class U2VIEW_EXPORT MSAHighlightingSchemeAgreementsFactory : public MSAHighlightingSchemeFactory {
public:
    MSAHighlightingSchemeAgreementsFactory(QObject* p, const QString& id, const QString& name, DNAAlphabetType atype):
        MSAHighlightingSchemeFactory(p, id, name, atype){};
    virtual MSAHighlightingScheme* create(QObject* p, MAlignmentObject* obj);
};

class U2VIEW_EXPORT MSAHighlightingSchemeDisagreementsFactory : public MSAHighlightingSchemeFactory {
public:
    MSAHighlightingSchemeDisagreementsFactory(QObject* p, const QString& id, const QString& name, DNAAlphabetType atype):
        MSAHighlightingSchemeFactory(p, id, name, atype){};
    virtual MSAHighlightingScheme* create(QObject* p, MAlignmentObject* obj);
};

class U2VIEW_EXPORT MSAHighlightingSchemeTransitionsFactory : public MSAHighlightingSchemeFactory {
public:
    MSAHighlightingSchemeTransitionsFactory(QObject* p, const QString& id, const QString& name, DNAAlphabetType atype):
      MSAHighlightingSchemeFactory(p, id, name, atype){};
    virtual MSAHighlightingScheme* create(QObject* p, MAlignmentObject* obj);
};

class U2VIEW_EXPORT MSAHighlightingSchemeTransversionsFactory : public MSAHighlightingSchemeFactory {
public:
    MSAHighlightingSchemeTransversionsFactory(QObject* p, const QString& id, const QString& name, DNAAlphabetType atype):
      MSAHighlightingSchemeFactory(p, id, name, atype){};
    virtual MSAHighlightingScheme* create(QObject* p, MAlignmentObject* obj);
};

class U2VIEW_EXPORT MSAHighlightingSchemeGapsFactory : public MSAHighlightingSchemeFactory {
public:
    MSAHighlightingSchemeGapsFactory(QObject* p, const QString& id, const QString& name, DNAAlphabetType atype):
      MSAHighlightingSchemeFactory(p, id, name, atype, true){};
    virtual MSAHighlightingScheme* create(QObject* p, MAlignmentObject* obj);
};

class U2VIEW_EXPORT MSAHighlightingScheme: public QObject {
    Q_OBJECT
public:
    MSAHighlightingScheme(QObject* p, MSAHighlightingSchemeFactory* f, MAlignmentObject* o);
    virtual void process(const char refChar, char &seqChar, bool &color);
    MSAHighlightingSchemeFactory* getFactory() const {return factory;}
    void setUseDots(bool b){useDots = b;};

    static QString EMPTY_NUCL;
    static QString EMPTY_AMINO;
    static QString AGREEMENTS_NUCL;
    static QString AGREEMENTS_AMINO;
    static QString DISAGREEMENTS_NUCL;
    static QString DISAGREEMENTS_AMINO;
    static QString TRANSITIONS_NUCL;
    static QString TRANSVERSIONS_NUCL;
    static QString GAPS_NUCL;
    static QString GAPS_AMINO;

protected:
    MSAHighlightingSchemeFactory*  factory;
    MAlignmentObject*       maObj;
    bool useDots;
};

class MSAHighlightingSchemeEmpty : public MSAHighlightingScheme{
    Q_OBJECT
public:
    MSAHighlightingSchemeEmpty(QObject* p, MSAHighlightingSchemeFactory* f, MAlignmentObject* o):
        MSAHighlightingScheme(p, f, o){}
    virtual void process(const char /*refChar*/, char &/*seqChar*/, bool &/*color*/){}
};

class MSAHighlightingSchemeAgreements : public MSAHighlightingScheme{
    Q_OBJECT
public:
    MSAHighlightingSchemeAgreements(QObject* p, MSAHighlightingSchemeFactory* f, MAlignmentObject* o):
        MSAHighlightingScheme(p, f, o){}
    virtual void process(const char refChar, char &seqChar, bool &color);
};

class MSAHighlightingSchemeDisagreements : public MSAHighlightingScheme{
    Q_OBJECT
public:
    MSAHighlightingSchemeDisagreements(QObject* p, MSAHighlightingSchemeFactory* f, MAlignmentObject* o)
        :MSAHighlightingScheme(p, f, o){}
    virtual void process(const char refChar, char &seqChar, bool &color);
};

class MSAHighlightingSchemeTransitions : public MSAHighlightingScheme{
    Q_OBJECT
public:
    MSAHighlightingSchemeTransitions(QObject* p, MSAHighlightingSchemeFactory* f, MAlignmentObject* o)
        :MSAHighlightingScheme(p, f, o){}
    virtual void process(const char refChar, char &seqChar, bool &color);
};

class MSAHighlightingSchemeTransversions : public MSAHighlightingScheme{
    Q_OBJECT
public:
    MSAHighlightingSchemeTransversions(QObject* p, MSAHighlightingSchemeFactory* f, MAlignmentObject* o)
        :MSAHighlightingScheme(p, f, o){}
    virtual void process(const char refChar, char &seqChar, bool &color);
};

class MSAHighlightingSchemeGaps : public MSAHighlightingScheme{
    Q_OBJECT
public:
    MSAHighlightingSchemeGaps(QObject* p, MSAHighlightingSchemeFactory* f, MAlignmentObject* o)
        :MSAHighlightingScheme(p, f, o){}
    virtual void process(const char refChar, char &seqChar, bool &color);
};

}//namespace
#endif
