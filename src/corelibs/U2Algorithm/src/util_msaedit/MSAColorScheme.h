/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#include <QVector>
#include <QColor>

#include <U2Core/global.h>

namespace U2 {


class MAlignmentObject;
class MSAColorScheme;
class MAlignment;
class MAlignmentModInfo;

#define COLOR_SCHEME_SETTINGS_ROOT QString("/color_schema_settings/")
#define COLOR_SCHEME_SETTINGS_SUB_DIRECTORY QString("MSA_schemes")
#define COLOR_SCHEME_COLOR_SCHEMA_DIR QString("colors_scheme_dir")

//WARNING if add more then one filter, change corresponding functions
#define COLOR_SCHEME_NAME_FILTERS QString(".csmsa")
#define COLOR_SCHEME_AMINO_KEYWORD QString("AMINO")
#define COLOR_SCHEME_NUCL_KEYWORD QString("NUCL")
#define COLOR_SCHEME_NUCL_DEFAULT_KEYWORD QString("NUCL_DEFAULT")
#define COLOR_SCHEME_NUCL_EXTENDED_KEYWORD QString("NUCL_EXTENDED")

class U2ALGORITHM_EXPORT CustomColorSchema{
public:
    QString name;
    DNAAlphabetType type;
    bool defaultAlpType;
    QMap<char, QColor> alpColors;
};

class U2ALGORITHM_EXPORT ColorSchemaSettingsUtils {
public:
    static QList<CustomColorSchema> getSchemas();
    static QString getColorsDir();
    static void getDefaultUgeneColors(DNAAlphabetType type, QMap<char, QColor>& alphColors);
    static QMap<char, QColor> getDefaultSchemaColors(DNAAlphabetType type, bool defaultAlpType);
    static void setColorsDir(const QString &colorsDir);

};



//////////////////////////////////////////////////////////////////////////
// factories

class U2ALGORITHM_EXPORT MSAColorSchemeFactory : public QObject {
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

class U2ALGORITHM_EXPORT MSAColorSchemeStaticFactory : public MSAColorSchemeFactory {
    Q_OBJECT
public:
    MSAColorSchemeStaticFactory(QObject* p, const QString& id, const QString& name, DNAAlphabetType atype, const QVector<QColor>& colorsPerChar);
    virtual MSAColorScheme* create(QObject* p, MAlignmentObject* obj);
private:
    QVector<QColor> colorsPerChar;
};

class U2ALGORITHM_EXPORT MSAColorSchemeCustomSettingsFactory : public MSAColorSchemeFactory {
    Q_OBJECT
public:
    MSAColorSchemeCustomSettingsFactory(QObject* p, const QString& id, const QString& name, DNAAlphabetType atype, const QVector<QColor>& colorsPerChar);
    virtual MSAColorScheme* create(QObject* p, MAlignmentObject* obj);
    bool isEqualTo(const CustomColorSchema& schema) const;
    static QVector<QColor> colorMapToColorVector(const QMap<char, QColor> &map);
private:
    QVector<QColor> colorsPerChar;
};

class MSAColorSchemePercIdentFactory : public MSAColorSchemeFactory {
    Q_OBJECT
public:
    MSAColorSchemePercIdentFactory(QObject* p, const QString& id, const QString& name, DNAAlphabetType atype);
    virtual MSAColorScheme* create(QObject* p, MAlignmentObject* obj);
};

class MSAColorSchemePercIdentGrayscaleFactory : public MSAColorSchemeFactory {
    Q_OBJECT
public:
    MSAColorSchemePercIdentGrayscaleFactory(QObject* p, const QString& id, const QString& name, DNAAlphabetType atype);
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

class U2ALGORITHM_EXPORT MSAColorScheme : public QObject {
    Q_OBJECT
public:
    MSAColorScheme(QObject* p, MSAColorSchemeFactory* f, MAlignmentObject* o);
    //Get color for symbol "c" on position [seq, pos]. Variable "c" has been added for optimization.
    virtual QColor getColor(int seq, int pos, char c) = 0;
    MSAColorSchemeFactory* getFactory() const {return factory;}

    static const QString EMPTY_NUCL;
    static const QString UGENE_NUCL;
    static const QString JALVIEW_NUCL;
    static const QString IDENTPERC_NUCL;
    static const QString IDENTPERC_NUCL_GRAY;
    static const QString CUSTOM_NUCL;

    static const QString EMPTY_AMINO;
    static const QString UGENE_AMINO;
    static const QString ZAPPO_AMINO;
    static const QString TAILOR_AMINO;
    static const QString HYDRO_AMINO;
    static const QString HELIX_AMINO;
    static const QString STRAND_AMINO;
    static const QString TURN_AMINO;
    static const QString BURIED_AMINO;
    static const QString IDENTPERC_AMINO;
    static const QString IDENTPERC_AMINO_GRAY;
    static const QString CLUSTALX_AMINO;
    static const QString CUSTOM_AMINO;

    static const QString EMPTY_RAW;
protected:
    MSAColorSchemeFactory*  factory;
    MAlignmentObject*       maObj;
};

class U2ALGORITHM_EXPORT MSAColorSchemeStatic : public MSAColorScheme {
    Q_OBJECT
public:
    MSAColorSchemeStatic(QObject* p, MSAColorSchemeFactory* f, MAlignmentObject* o, const QVector<QColor>& colorsPerChar);
    virtual QColor getColor(int seq, int pos, char c);
private:

    QVector<QColor> colorsPerChar;
};


//PERCENT
class U2ALGORITHM_EXPORT MSAColorSchemePercIdent : public MSAColorScheme {
    Q_OBJECT
public:
    MSAColorSchemePercIdent(QObject* p, MSAColorSchemeFactory* f, MAlignmentObject* o);
    virtual QColor getColor(int seq, int pos, char c);

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

class U2ALGORITHM_EXPORT MSAColorSchemePercIdentGrayscale : public MSAColorSchemePercIdent {
public:
    MSAColorSchemePercIdentGrayscale(QObject* p, MSAColorSchemeFactory* f, MAlignmentObject* o);
};

//CLUSTALX
// 0.5 * alisize mem use, slow update
class U2ALGORITHM_EXPORT MSAColorSchemeClustalX: public MSAColorScheme {
    Q_OBJECT
public:
    MSAColorSchemeClustalX(QObject* p, MSAColorSchemeFactory* f, MAlignmentObject* o);
    virtual QColor getColor(int seq, int pos, char c);
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
class U2ALGORITHM_EXPORT MSAColorSchemeRegistry : public QObject {
    Q_OBJECT
public:
    MSAColorSchemeRegistry();
    ~MSAColorSchemeRegistry();

    const QList<MSAColorSchemeFactory*>& getMSAColorSchemes() const {return colorers;}
    const QList<MSAColorSchemeFactory*>& getCustomColorSchemes() const {return customColorers;}

    QList<MSAColorSchemeFactory*> getMSAColorSchemes(DNAAlphabetType atype) const;
    QList<MSAColorSchemeFactory*> getMSACustomColorSchemes(DNAAlphabetType atype) const;

    MSAColorSchemeFactory* getMSAColorSchemeFactoryById(const QString& id) const;

    void addCustomSchema(const CustomColorSchema& schema);
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

class U2ALGORITHM_EXPORT MSAHighlightingSchemeRegistry : public QObject {
    Q_OBJECT
public:
    MSAHighlightingSchemeRegistry();

    MSAHighlightingSchemeFactory* getMSAHighlightingSchemeFactoryById(const QString& id) const;
    QList<MSAHighlightingSchemeFactory*> getMSAHighlightingSchemes(DNAAlphabetType atype);

private:
    QList<MSAHighlightingSchemeFactory*> schemes;
};

class U2ALGORITHM_EXPORT MSAHighlightingSchemeFactory : public QObject {
    Q_OBJECT
public:
    MSAHighlightingSchemeFactory(QObject* p, const QString& id, const QString& name, DNAAlphabetType atype,
        bool _refFree = false, bool _needThreshold = false )
        :QObject(p), id(id), name(name), aType(atype), refFree(_refFree), needThreshold(_needThreshold){};
    virtual MSAHighlightingScheme* create(QObject* p, MAlignmentObject* obj) = 0;

    const QString& getId() const {return id;}
    const QString& getName() const {return name;}
    DNAAlphabetType getAlphabetType() const {return aType;}
    bool isRefFree(){return refFree;};
    bool isNeedThreshold(){return needThreshold;};

private:
    QString         id;
    QString         name;
    DNAAlphabetType aType;
    bool            refFree;
    bool            needThreshold;
};

class U2ALGORITHM_EXPORT MSAHighlightingSchemeNoColorsFactory : public MSAHighlightingSchemeFactory {
public:
    MSAHighlightingSchemeNoColorsFactory(QObject* p, const QString& id, const QString& name, DNAAlphabetType atype):
        MSAHighlightingSchemeFactory(p, id, name, atype, true){};
    virtual MSAHighlightingScheme* create(QObject* p, MAlignmentObject* obj);
};

class U2ALGORITHM_EXPORT MSAHighlightingSchemeAgreementsFactory : public MSAHighlightingSchemeFactory {
public:
    MSAHighlightingSchemeAgreementsFactory(QObject* p, const QString& id, const QString& name, DNAAlphabetType atype):
        MSAHighlightingSchemeFactory(p, id, name, atype){};
    virtual MSAHighlightingScheme* create(QObject* p, MAlignmentObject* obj);
};

class U2ALGORITHM_EXPORT MSAHighlightingSchemeDisagreementsFactory : public MSAHighlightingSchemeFactory {
public:
    MSAHighlightingSchemeDisagreementsFactory(QObject* p, const QString& id, const QString& name, DNAAlphabetType atype):
        MSAHighlightingSchemeFactory(p, id, name, atype){};
    virtual MSAHighlightingScheme* create(QObject* p, MAlignmentObject* obj);
};

class U2ALGORITHM_EXPORT MSAHighlightingSchemeTransitionsFactory : public MSAHighlightingSchemeFactory {
public:
    MSAHighlightingSchemeTransitionsFactory(QObject* p, const QString& id, const QString& name, DNAAlphabetType atype):
      MSAHighlightingSchemeFactory(p, id, name, atype){};
    virtual MSAHighlightingScheme* create(QObject* p, MAlignmentObject* obj);
};

class U2ALGORITHM_EXPORT MSAHighlightingSchemeTransversionsFactory : public MSAHighlightingSchemeFactory {
public:
    MSAHighlightingSchemeTransversionsFactory(QObject* p, const QString& id, const QString& name, DNAAlphabetType atype):
      MSAHighlightingSchemeFactory(p, id, name, atype){};
    virtual MSAHighlightingScheme* create(QObject* p, MAlignmentObject* obj);
};

class U2ALGORITHM_EXPORT MSAHighlightingSchemeGapsFactory : public MSAHighlightingSchemeFactory {
public:
    MSAHighlightingSchemeGapsFactory(QObject* p, const QString& id, const QString& name, DNAAlphabetType atype):
      MSAHighlightingSchemeFactory(p, id, name, atype, true){};
    virtual MSAHighlightingScheme* create(QObject* p, MAlignmentObject* obj);
};

class U2ALGORITHM_EXPORT MSAHighlightingSchemeConservationFactory : public MSAHighlightingSchemeFactory {
public:
    MSAHighlightingSchemeConservationFactory(QObject* p, const QString& id, const QString& name, DNAAlphabetType atype):
      MSAHighlightingSchemeFactory(p, id, name, atype, true, true){};
      virtual MSAHighlightingScheme* create(QObject* p, MAlignmentObject* obj);
};

class U2ALGORITHM_EXPORT MSAHighlightingScheme: public QObject {
    Q_OBJECT
public:
    MSAHighlightingScheme(QObject* p, MSAHighlightingSchemeFactory* f, MAlignmentObject* o);
    virtual void process(const char refChar, char &seqChar, bool &color, int refCharColumn, int refCharRow);
    MSAHighlightingSchemeFactory* getFactory() const {return factory;}
    void setUseDots(bool b) { useDots = b; }
    bool getUseDots() const { return useDots; }
    virtual void applySettings(const QVariantMap &settings);
    virtual QVariantMap getSettings() const;

    static const QString EMPTY_NUCL;
    static const QString EMPTY_AMINO;
    static const QString EMPTY_RAW;
    static const QString AGREEMENTS_NUCL;
    static const QString AGREEMENTS_AMINO;
    static const QString DISAGREEMENTS_NUCL;
    static const QString DISAGREEMENTS_AMINO;
    static const QString TRANSITIONS_NUCL;
    static const QString TRANSVERSIONS_NUCL;
    static const QString GAPS_NUCL;
    static const QString GAPS_AMINO;
    static const QString GAPS_RAW;
    static const QString CONSERVATION_NUCL;
    static const QString CONSERVATION_AMINO;

    static const QString THRESHOLD_PARAMETER_NAME;
    static const QString LESS_THEN_THRESHOLD_PARAMETER_NAME;
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
    virtual void process(const char refChar, char &seqChar, bool &color, int refCharColumn, int refCharRow);
};

class MSAHighlightingSchemeAgreements : public MSAHighlightingScheme{
    Q_OBJECT
public:
    MSAHighlightingSchemeAgreements(QObject* p, MSAHighlightingSchemeFactory* f, MAlignmentObject* o):
        MSAHighlightingScheme(p, f, o){}
    virtual void process(const char refChar, char &seqChar, bool &color, int refCharColumn, int refCharRow);
};

class MSAHighlightingSchemeDisagreements : public MSAHighlightingScheme{
    Q_OBJECT
public:
    MSAHighlightingSchemeDisagreements(QObject* p, MSAHighlightingSchemeFactory* f, MAlignmentObject* o)
        :MSAHighlightingScheme(p, f, o){}
    virtual void process(const char refChar, char &seqChar, bool &color, int refCharColumn, int refCharRow);
};

class MSAHighlightingSchemeTransitions : public MSAHighlightingScheme{
    Q_OBJECT
public:
    MSAHighlightingSchemeTransitions(QObject* p, MSAHighlightingSchemeFactory* f, MAlignmentObject* o)
        :MSAHighlightingScheme(p, f, o){}
    virtual void process(const char refChar, char &seqChar, bool &color, int refCharColumn, int refCharRow);
};

class MSAHighlightingSchemeTransversions : public MSAHighlightingScheme{
    Q_OBJECT
public:
    MSAHighlightingSchemeTransversions(QObject* p, MSAHighlightingSchemeFactory* f, MAlignmentObject* o)
        :MSAHighlightingScheme(p, f, o){}
    virtual void process(const char refChar, char &seqChar, bool &color, int refCharColumn, int refCharRow);
};

class MSAHighlightingSchemeGaps : public MSAHighlightingScheme{
    Q_OBJECT
public:
    MSAHighlightingSchemeGaps(QObject* p, MSAHighlightingSchemeFactory* f, MAlignmentObject* o)
        :MSAHighlightingScheme(p, f, o){}
    virtual void process(const char refChar, char &seqChar, bool &color, int refCharColumn, int refCharRow);
};

typedef QMap<char, int> CharCountMap;

class MSAHighlightingSchemeConservation : public MSAHighlightingScheme{
    Q_OBJECT
public:
    MSAHighlightingSchemeConservation(QObject* p, MSAHighlightingSchemeFactory* f, MAlignmentObject* o);
    virtual void process(const char refChar, char &seqChar, bool &color, int refCharColumn, int refCharRow);
    virtual void applySettings(const QVariantMap &settings);
    virtual QVariantMap getSettings() const;
private slots:
    void sl_resetMap();
private:
    void calculateStatisticForColumn(int refCharColumn);
    QMap<int, CharCountMap> msaCharCountMap;
    int threshold;
    bool lessThenThreshold;
};

}//namespace
#endif
