/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#include "MSAColorScheme.h"
#include "MSAEditorFactory.h"
#include "ColorSchemaSettingsController.h"

#include <U2Core/AppContext.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/FeatureColors.h>
#include <U2Algorithm/MSAConsensusUtils.h>

namespace U2 {


#define SET_C(ch, cl) colorsPerChar[ch]=colorsPerChar[ch+('a'-'A')]=cl

//////////////////////////////////////////////////////////////////////////
// factories

MSAColorSchemeFactory::MSAColorSchemeFactory(QObject* p, const QString& _id, const QString& _name, DNAAlphabetType _at)
: QObject(p), id(_id), name(_name), aType(_at)
{
}

MSAColorSchemeStaticFactory::MSAColorSchemeStaticFactory(QObject* p, const QString& _id, const QString& _name, 
                                                         DNAAlphabetType _atype, const QVector<QColor>& _colorsPerChar) 
: MSAColorSchemeFactory(p, _id, _name, _atype), colorsPerChar(_colorsPerChar)
{
}

MSAColorScheme* MSAColorSchemeStaticFactory::create(QObject* p, MAlignmentObject* o) {
    return new MSAColorSchemeStatic(p, this, o, colorsPerChar);
}

MSAColorSchemeCustomSettingsFactory::MSAColorSchemeCustomSettingsFactory(QObject* p, const QString& _id, const QString& _name, 
                                                         DNAAlphabetType _atype, const QVector<QColor>& _colorsPerChar) 
                                                         : MSAColorSchemeFactory(p, _id, _name, _atype), colorsPerChar(_colorsPerChar)
{    
}

MSAColorScheme* MSAColorSchemeCustomSettingsFactory::create(QObject* p, MAlignmentObject* o) {
    return new MSAColorSchemeStatic(p, this, o, colorsPerChar);
}



MSAColorSchemePercIdentFactory::MSAColorSchemePercIdentFactory(QObject* p, const QString& _id, const QString& _name, DNAAlphabetType _atype) 
: MSAColorSchemeFactory(p, _id, _name, _atype)
{
}

MSAColorScheme* MSAColorSchemePercIdentFactory::create(QObject* p, MAlignmentObject* o) {
    return new MSAColorSchemePercIdent(p, this, o);
}


MSAColorSchemeClustalXFactory::MSAColorSchemeClustalXFactory(QObject* p, const QString& _id, const QString& _name, DNAAlphabetType _atype) 
: MSAColorSchemeFactory(p, _id, _name, _atype)
{
}

MSAColorScheme* MSAColorSchemeClustalXFactory::create(QObject* p, MAlignmentObject* o) {
    return new MSAColorSchemeClustalX(p, this, o);
}


//////////////////////////////////////////////////////////////////////////
// schemes

MSAColorScheme::MSAColorScheme(QObject* p, MSAColorSchemeFactory* f, MAlignmentObject* o) : QObject(p), factory(f), maObj(o) {
}


MSAColorSchemeStatic::MSAColorSchemeStatic(QObject* p, MSAColorSchemeFactory* f, MAlignmentObject* o, const QVector<QColor>& _cp)
: MSAColorScheme(p, f, o), colorsPerChar(_cp)
{
}

QColor MSAColorSchemeStatic::getColor(int seq, int pos) {
    char c = maObj->getMAlignment().charAt(seq, pos);
    return getColor(c);
}


/// PERCENT
MSAColorSchemePercIdent::MSAColorSchemePercIdent(QObject* p, MSAColorSchemeFactory* f, MAlignmentObject* o) 
: MSAColorScheme(p, f, o)
{
    cacheVersion = 0;
    objVersion = 1;
    mask4[0]=81;
    mask4[1]=61;
    mask4[2]=41;
    mask4[3]=25;

    colorsByRange[0] = QColor("#6464FF");
    colorsByRange[1] = QColor("#9999FF");
    colorsByRange[2] = QColor("#CCCCFF");
    colorsByRange[3] = QColor();

    connect(maObj, SIGNAL(si_alignmentChanged(const MAlignment&, const MAlignmentModInfo&)), 
                  SLOT(sl_alignmentChanged(const MAlignment&, const MAlignmentModInfo&)));
}

QColor MSAColorSchemePercIdent::getColor(int seq, int pos) {
    updateCache();
    char c = maObj->getMAlignment().charAt(seq, pos);
    if (c == MAlignment_GapChar) {
        return QColor();
    }
    quint32 packedVal = indentCache[pos];
    MSAConsensusUtils::unpackConsensusCharsFromInt(packedVal, tmpChars, tmpRanges);
    for (int i=0; i < 4; i++) {
        if (c == tmpChars[i]) {
            int range = tmpRanges[i];
            return colorsByRange[range];
        }
    }
    return QColor();
}

void MSAColorSchemePercIdent::updateCache() {
    if (cacheVersion == objVersion) {
        return;
    }
    const MAlignment& ma = maObj->getMAlignment();
    int aliLen = ma.getLength();
    indentCache.resize(aliLen);
    for (int i=0; i < aliLen; i++) {
        indentCache[i] = MSAConsensusUtils::packConsensusCharsToInt(ma, i, mask4, true);
    }
    cacheVersion = objVersion;
}

/// CLUSTAL
MSAColorSchemeClustalX::MSAColorSchemeClustalX(QObject* p, MSAColorSchemeFactory* f, MAlignmentObject* maObj) 
: MSAColorScheme(p, f, maObj)
{
    objVersion = 1;
    cacheVersion = 0;
    aliLen = maObj->getLength();

    colorByIdx[ClustalColor_BLUE]    = "#80a0f0";
    colorByIdx[ClustalColor_RED]     = "#f01505";
    colorByIdx[ClustalColor_GREEN]   = "#15c015";
    colorByIdx[ClustalColor_PINK]    = "#f08080";
    colorByIdx[ClustalColor_MAGENTA] = "#c048c0";
    colorByIdx[ClustalColor_ORANGE]  = "#f09048";
    colorByIdx[ClustalColor_CYAN]    = "#15a4a4";
    colorByIdx[ClustalColor_YELLOW]  = "#c0c000";

    connect(maObj, SIGNAL(si_alignmentChanged(const MAlignment&, const MAlignmentModInfo&)), 
        SLOT(sl_alignmentChanged(const MAlignment&, const MAlignmentModInfo&)));
}

QColor MSAColorSchemeClustalX::getColor(int seq, int pos) {
    if (cacheVersion!=objVersion) {
        updateCache();
    }
    int idx = getColorIdx(seq, pos);
    assert(idx >=0 && idx < ClustalColor_NUM_COLORS);
    return colorByIdx[idx];
}

int MSAColorSchemeClustalX::getColorIdx(int seq, int pos) {
    bool low = false;
    int cacheIdx = getCacheIdx(seq, pos, low);
    quint8 val = colorsCache[cacheIdx];
    int colorIdx = low ? val & 0x0F : (val & 0xF0) >> 4;
    assert(colorIdx >=0 && colorIdx < ClustalColor_NUM_COLORS);
    return colorIdx;
}

void MSAColorSchemeClustalX::setColorIdx(int seq, int pos, int colorIdx) {
    assert(colorIdx >=0 && colorIdx < ClustalColor_NUM_COLORS);
    bool low = false;
    int cacheIdx = getCacheIdx(seq, pos, low);
    quint8 val = colorsCache[cacheIdx];
    if (low) {
        val = (val & 0xF0) | colorIdx;
    } else {
        val = (val & 0x0F) | (colorIdx << 4);
    }
    colorsCache[cacheIdx] = val; 
}

static int basesContent(const int* freqs, const char* str, int len) {
    int res = 0;
    for (int i=0; i < len; i++) {
        uchar c = str[i];
        res +=freqs[c];
    }
    return res;
}

void MSAColorSchemeClustalX::updateCache() {
    if (cacheVersion == objVersion) {
        return;
    }
    // compute colors for whole ali
    // use 4 bits per color
    const MAlignment& ma = maObj->getMAlignment();
    int nSeq = ma.getNumRows();
    aliLen = maObj->getLength();
    cacheVersion = objVersion;

    bool stub = false;
    int cacheSize = getCacheIdx(nSeq, aliLen, stub) + 1;
    colorsCache.resize(cacheSize);

    /*  source: http://ekhidna.biocenter.helsinki.fi/pfam2/clustal_colours

        BLUE
            (W,L,V,I,M,F):  {50%, P}{60%, WLVIMAFCYHP}
            (A):            {50%, P}{60%, WLVIMAFCYHP}{85%, T,S,G}
            (C):            {50%, P}{60%, WLVIMAFCYHP}{85%, S}
        RED
            (K,R):          {60%, KR}{85%, Q}
        GREEN
            (T):            {50%, TS}{60%, WLVIMAFCYHP}
            (S):            {50%, TS}{80%, WLVIMAFCYHP}
            (N):            {50%, N}{85%, D}
            (Q):            {50%, QE}{60%, KR}
        PINK
            (C):            {85%, C}
        MAGENTA
            (D):            {50%, DE,N}
            (E):            {50%, DE,QE}
        ORANGE
            (G):            {ALWAYS}
        CYAN
            (H,Y):          {50%, P}{60%, WLVIMAFCYHP}
        YELLOW
            (P):            {ALWAYS}
    
        WARN: do not count gaps in percents!
    */


    QVector<int> freqsByChar(256);
    const int* freqs = freqsByChar.data();
    
    for (int pos = 0; pos < aliLen; pos++) {
        int nonGapChars = 0;
        MSAConsensusUtils::getColumnFreqs(ma, pos, freqsByChar, nonGapChars);
        int content50 = int(nonGapChars * 50.0 / 100);
        int content60 = int(nonGapChars * 60.0 / 100);
        int content80 = int(nonGapChars * 80.0 / 100);
        int content85 = int(nonGapChars * 85.0 / 100);

        for (int seq = 0; seq < nSeq; seq++) {
            char c = ma.charAt(seq, pos);
            int colorIdx = ClustalColor_NO_COLOR;
            switch(c) {
                case 'W': //(W,L,V,I,M,F): {50%, P}{60%, WLVIMAFCYHP} -> BLUE
                case 'L':
                case 'V':
                case 'I':
                case 'M':
                case 'F':
                    if (freqs['P'] > content50 || basesContent(freqs, "WLVIMAFCYHP", 11) > content60) {
                        colorIdx = ClustalColor_BLUE;
                    }
                    break;
                case 'A': // {50%, P}{60%, WLVIMAFCYHP}{85%, T,S,G} -> BLUE
                    if (freqs['P'] > content50 || basesContent(freqs, "WLVIMAFCYHP", 11) > content60) {
                        colorIdx = ClustalColor_BLUE;
                    } else if (freqs['T'] > content85 || freqs['S'] > content85 || freqs['G']>85) {
                        colorIdx = ClustalColor_BLUE;
                    }
                    break;

                case 'K': //{60%, KR}{85%, Q} -> RED
                case 'R': 
                    if ((freqs['K'] + freqs['R'] > content60) || freqs['Q'] > content85) {
                        colorIdx = ClustalColor_RED;
                    }
                    break;
                
                case 'T': // {50%, TS}{60%, WLVIMAFCYHP} -> GREEN
                    if ((freqs['T'] + freqs['S'] > content50) || basesContent(freqs, "WLVIMAFCYHP", 11) > content60) {
                        colorIdx = ClustalColor_GREEN;
                    }
                    break;
                
                case 'S': // {50%, TS}{80%, WLVIMAFCYHP} -> GREEN
                    if ((freqs['T'] + freqs['S'] > content50) || basesContent(freqs, "WLVIMAFCYHP", 11) > content80) {
                        colorIdx = ClustalColor_GREEN;
                    }
                    break;

                case 'N': // {50%, N}{85%, D} -> GREEN
                    if (freqs['N'] > content50 || freqs['D'] > content85) {
                        colorIdx = ClustalColor_GREEN;
                    }
                    break;

                case 'Q': // {50%, QE}{60%, KR} -> GREEN
                    if ((freqs['Q'] + freqs['E']) > content50 || (freqs['K'] + freqs['R']) > content60) {
                        colorIdx = ClustalColor_GREEN;
                    }
                    break;

                case 'C': //{85%, C} -> PINK
                          //{50%, P}{60%, WLVIMAFCYHP}{85%, S} -> BLUE
                    if (freqs['C'] > content85) {
                        colorIdx = ClustalColor_PINK;
                    } else if (freqs['P'] > content50 || basesContent(freqs, "WLVIMAFCYHP", 11) > content60 || freqs['S'] > content85) {
                        colorIdx = ClustalColor_BLUE;
                    }
                    break;                          

                case 'D': //{50%, DE,N} -> MAGENTA
                    if ((freqs['D'] + freqs['E']) > content50 || freqs['N'] > content50) {
                        colorIdx = ClustalColor_MAGENTA;
                    }
                    break;
                case 'E': //{50%, DE,QE} -> MAGENTA
                    if ((freqs['D'] + freqs['E']) > content50 || (freqs['Q'] + freqs['E']) > content50) {
                        colorIdx = ClustalColor_MAGENTA;
                    }
                    break;
                case 'G': //{ALWAYS} -> ORANGE
                    colorIdx = ClustalColor_ORANGE;
                    break;

                case 'H': // {50%, P}{60%, WLVIMAFCYHP} -> CYAN 
                case 'Y':
                    if (freqs['P'] > content50 || basesContent(freqs, "WLVIMAFCYHP", 11) > content60) {
                        colorIdx = ClustalColor_CYAN;
                    }
                    break;

                case 'P': //{ALWAYS} -> YELLOW
                    colorIdx = ClustalColor_YELLOW;
                    break;
                default: 
                    break;

            }
            setColorIdx(seq, pos, colorIdx);
        }
    }
}


//////////////////////////////////////////////////////////////////////////
// registry
MSAColorSchemeRegistry::MSAColorSchemeRegistry() {
    ColorSchemaSettingsPageController* controller = new ColorSchemaSettingsPageController();
    connect(controller, SIGNAL(si_customSettingsChanged()), SLOT(sl_onCustomSettingsChanged()));
    AppContext::getAppSettingsGUI()->registerPage(controller);
    initBuiltInSchemes();
    initCustomSchema();
}

MSAColorSchemeRegistry::~MSAColorSchemeRegistry(){
    deleteOldCustomFactories();
}

void MSAColorSchemeRegistry::deleteOldCustomFactories(){
    foreach(MSAColorSchemeFactory* f, customColorers){
        delete f;
    }
    customColorers.clear();
}


QList<MSAColorSchemeFactory*> MSAColorSchemeRegistry::getMSAColorSchemes(DNAAlphabetType atype) const {
    QList<MSAColorSchemeFactory*> res;
    foreach(MSAColorSchemeFactory* f, colorers) {
        if (f->getAlphabetType() == atype) {
            res.append(f);
        }
    }
    return res;
}

QList<MSAColorSchemeFactory*> MSAColorSchemeRegistry::getMSACustomColorSchemes(DNAAlphabetType atype) const{
    QList<MSAColorSchemeFactory*> res;
    foreach(MSAColorSchemeFactory* f, customColorers) {
        if (f->getAlphabetType() == atype) {
            res.append(f);
        }
    }
    return res;
}

MSAColorSchemeFactory* MSAColorSchemeRegistry::getMSAColorSchemeFactoryById(const QString& id) const {
    foreach(MSAColorSchemeFactory* csf, colorers) {
        if(csf->getId() == id) {
            return csf;
        }
    }
    foreach(MSAColorSchemeFactory* csf, customColorers) {
        if(csf->getId() == id) {
            return csf;
        }
    }
    return NULL;
}

static bool compareNames(const MSAColorSchemeFactory* a1, const MSAColorSchemeFactory* a2) {
    if (a1->getId() == MSAColorScheme::EMPTY_NUCL) {
        return true;
    }
    if (a2->getId() == MSAColorScheme::EMPTY_NUCL) {
        return false;
    }
    if (a1->getId() == MSAColorScheme::EMPTY_AMINO) {
        return true;
    }
    if (a2->getId() == MSAColorScheme::EMPTY_AMINO) {
        return false;
    }
    return a1->getName() < a2->getName();
}

void MSAColorSchemeRegistry::addMSAColorSchemeFactory(MSAColorSchemeFactory* csf) {
    assert(getMSAColorSchemeFactoryById(csf->getId()) == NULL);
    colorers.append(csf);
    qStableSort(colorers.begin(), colorers.end(), compareNames);
}

void MSAColorSchemeRegistry::addMSACustomColorSchemeFactory(MSAColorSchemeFactory* csf){
    assert(getMSAColorSchemeFactoryById(csf->getId()) == NULL);
    customColorers.append(csf);
    qStableSort(colorers.begin(), colorers.end(), compareNames);
}

static void fillEmptyCS(QVector<QColor>& colorsPerChar) {
    colorsPerChar.fill(QColor(), 256);
}

static void fillLightColorsCS(QVector<QColor>& colorsPerChar) {
    for (int i = 0; i < 256; i++) {
        colorsPerChar[i] = FeatureColors::genLightColor(QString((char)i));
    }
    colorsPerChar[MAlignment_GapChar] = QColor(); //invalid color -> no color at all
}


static void addUGENEAmino(QVector<QColor>& colorsPerChar) {
    //amino groups: "KRH", "GPST", "FWY", "ILM"
    QColor krh("#FFEE00");
    SET_C('K', krh);
    SET_C('R', krh.darker(120));
    SET_C('H', krh.lighter(120));

    QColor gpst("#FF5082");
    SET_C('G', gpst);
    SET_C('P', gpst.darker(120));
    SET_C('S', gpst.lighter(120));
    SET_C('T', gpst.lighter(150));

    QColor fwy("#3DF490");
    SET_C('F', fwy);
    SET_C('W', fwy.darker(120));
    SET_C('Y', fwy.lighter(120));

    QColor ilm("#00ABED");
    SET_C('I', ilm);
    SET_C('L', ilm.darker(120));
    SET_C('M', ilm.lighter(120));

    //fix some color overlaps:
    //e looks like q by default
    SET_C('E', "#C0BDBB"); //gray
    SET_C('X', "#FCFCFC");
}


static void addUGENENucl(QVector<QColor>& colorsPerChar) {
    SET_C('A', "#FCFF92"); // yellow
    SET_C('C', "#70F970"); // green
    SET_C('T', "#FF99B1"); // light red
    SET_C('G', "#4EADE1"); // light blue
    SET_C('U', colorsPerChar['T'].lighter(120));
    SET_C('N', "#FCFCFC");
}

//TODO: check extended AMINO chars!!! (O/B/U?)

static void addZappoAmino(QVector<QColor>& colorsPerChar) {
    //Aliphatic/hydrophobic:    ILVAM       #ffafaf
    SET_C('I', "#ffafaf"); 
    SET_C('L', "#ffafaf"); 
    SET_C('V', "#ffafaf"); 
    SET_C('A', "#ffafaf"); 
    SET_C('M', "#ffafaf"); 

    //Aromatic:  FWY         #ffc800
    SET_C('F', "#ffc800"); 
    SET_C('W', "#ffc800"); 
    SET_C('Y', "#ffc800"); 

    //Positive   KRH         #6464ff
    SET_C('K', "#6464ff"); 
    SET_C('R', "#6464ff"); 
    SET_C('H', "#6464ff"); 

    //Negative   DE          #ff0000
    SET_C('D', "#ff0000"); 
    SET_C('E', "#ff0000"); 
    
    //Hydrophil  STNQ        #00ff00
    SET_C('S', "#00ff00"); 
    SET_C('T', "#00ff00"); 
    SET_C('N', "#00ff00"); 
    SET_C('Q', "#00ff00"); 
    
    //conformat  PG          #ff00ff
    SET_C('P', "#ff00ff"); 
    SET_C('G', "#ff00ff"); 

    //Cysteine   C           #ffff00
    SET_C('C', "#ffff00"); 
}

static void addTailorAmino(QVector<QColor>& colorsPerChar) {
    SET_C('A', "#ccff00"); 
    SET_C('V', "#99ff00"); 
    SET_C('I', "#66ff00"); 
    SET_C('L', "#33ff00"); 
    SET_C('M', "#00ff00"); 
    SET_C('F', "#00ff66"); 
    SET_C('Y', "#00ffcc"); 
    SET_C('W', "#00ccff"); 
    SET_C('H', "#0066ff"); 
    SET_C('R', "#0000ff"); 
    SET_C('K', "#6600ff"); 
    SET_C('N', "#cc00ff"); 
    SET_C('Q', "#ff00cc"); 
    SET_C('E', "#ff0066"); 
    SET_C('D', "#ff0000"); 
    SET_C('S', "#ff3300"); 
    SET_C('T', "#ff6600"); 
    SET_C('G', "#ff9900"); 
    SET_C('P', "#ffcc00"); 
    SET_C('C', "#ffff00"); 
}

static void addHydroAmino(QVector<QColor>& colorsPerChar) {
//The most hydrophobic residues according to this table are colored red and the most hydrophilic ones are colored blue.
    SET_C('I', "#ff0000"); 
    SET_C('V', "#f60009"); 
    SET_C('L', "#ea0015"); 
    SET_C('F', "#cb0034"); 
    SET_C('C', "#c2003d"); 
    SET_C('M', "#b0004f"); 
    SET_C('A', "#ad0052"); 
    SET_C('G', "#6a0095"); 
    SET_C('X', "#680097"); 
    SET_C('T', "#61009e"); 
    SET_C('S', "#5e00a1"); 
    SET_C('W', "#5b00a4"); 
    SET_C('Y', "#4f00b0"); 
    SET_C('P', "#4600b9"); 
    SET_C('H', "#1500ea"); 
    SET_C('E', "#0c00f3"); 
    SET_C('Z', "#0c00f3"); 
    SET_C('Q', "#0c00f3"); 
    SET_C('D', "#0c00f3"); 
    SET_C('B', "#0c00f3"); 
    SET_C('N', "#0c00f3"); 
    SET_C('K', "#0000ff"); 
    SET_C('R', "#0000ff"); 
}

static void addHelixAmino(QVector<QColor>& colorsPerChar) {
    SET_C('E', "#ff00ff"); 
    SET_C('M', "#ef10ef"); 
    SET_C('A', "#e718e7"); 
    SET_C('Z', "#c936c9"); 
    SET_C('L', "#ae51ae"); 
    SET_C('K', "#a05fa0"); 
    SET_C('F', "#986798"); 
    SET_C('Q', "#926d92"); 
    SET_C('I', "#8a758a"); 
    SET_C('W', "#8a758a"); 
    SET_C('V', "#857a85"); 
    SET_C('D', "#778877"); 
    SET_C('X', "#758a75"); 
    SET_C('H', "#758a75"); 
    SET_C('R', "#6f906f"); 
    SET_C('B', "#49b649"); 
    SET_C('T', "#47b847"); 
    SET_C('S', "#36c936"); 
    SET_C('C', "#23dc23"); 
    SET_C('Y', "#21de21"); 
    SET_C('N', "#1be41b"); 
    SET_C('G', "#00ff00"); 
    SET_C('P', "#00ff00"); 
}

static void addStrandAmino(QVector<QColor>& colorsPerChar) {
    SET_C('V', "#ffff00"); 
    SET_C('I', "#ecec13"); 
    SET_C('Y', "#d3d32c"); 
    SET_C('F', "#c2c23d"); 
    SET_C('W', "#c0c03f"); 
    SET_C('L', "#b2b24d"); 
    SET_C('T', "#9d9d62"); 
    SET_C('C', "#9d9d62"); 
    SET_C('Q', "#8c8c73"); 
    SET_C('M', "#82827d"); 
    SET_C('X', "#797986"); 
    SET_C('R', "#6b6b94"); 
    SET_C('N', "#64649b"); 
    SET_C('H', "#60609f"); 
    SET_C('A', "#5858a7"); 
    SET_C('S', "#4949b6"); 
    SET_C('G', "#4949b6"); 
    SET_C('Z', "#4747b8"); 
    SET_C('K', "#4747b8"); 
    SET_C('B', "#4343bc"); 
    SET_C('P', "#2323dc"); 
    SET_C('D', "#2121de"); 
    SET_C('E', "#0000ff"); 
}

static void addTurnAmino(QVector<QColor>& colorsPerChar) {
    SET_C('N', "#ff0000"); 
    SET_C('G', "#ff0000"); 
    SET_C('P', "#f60909"); 
    SET_C('B', "#f30c0c"); 
    SET_C('D', "#e81717"); 
    SET_C('S', "#e11e1e"); 
    SET_C('C', "#a85757"); 
    SET_C('Y', "#9d6262"); 
    SET_C('K', "#7e8181"); 
    SET_C('X', "#7c8383"); 
    SET_C('Q', "#778888"); 
    SET_C('W', "#738c8c"); 
    SET_C('T', "#738c8c"); 
    SET_C('R', "#708f8f"); 
    SET_C('H', "#708f8f"); 
    SET_C('Z', "#5ba4a4"); 
    SET_C('E', "#3fc0c0"); 
    SET_C('A', "#2cd3d3"); 
    SET_C('F', "#1ee1e1"); 
    SET_C('M', "#1ee1e1"); 
    SET_C('L', "#1ce3e3"); 
    SET_C('V', "#07f8f8"); 
    SET_C('I', "#00ffff"); 
}

static void addBuriedAmino(QVector<QColor>& colorsPerChar) {
    SET_C('C', "#0000ff"); 
    SET_C('I', "#0054ab"); 
    SET_C('V', "#005fa0"); 
    SET_C('L', "#007b84"); 
    SET_C('F', "#008778"); 
    SET_C('M', "#009768"); 
    SET_C('G', "#009d62"); 
    SET_C('A', "#00a35c"); 
    SET_C('W', "#00a857"); 
    SET_C('X', "#00b649"); 
    SET_C('S', "#00d52a"); 
    SET_C('H', "#00d52a"); 
    SET_C('T', "#00db24"); 
    SET_C('P', "#00e01f"); 
    SET_C('Y', "#00e619"); 
    SET_C('N', "#00eb14"); 
    SET_C('B', "#00eb14"); 
    SET_C('D', "#00eb14"); 
    SET_C('Q', "#00f10e"); 
    SET_C('Z', "#00f10e"); 
    SET_C('E', "#00f10e"); 
    SET_C('R', "#00fc03"); 
    SET_C('K', "#00ff00"); 
}

static void addJalviewNucl(QVector<QColor>& colorsPerChar) {
    SET_C('A', "#64F73F"); 
    SET_C('C', "#FFB340"); 
    SET_C('G', "#EB413C"); 
    SET_C('T', "#3C88EE"); 
    SET_C('U', colorsPerChar['T'].lighter(105)); 
}


//SET_C('', "#"); 

QString MSAColorScheme::EMPTY_NUCL      = "COLOR_SCHEME_EMPTY_NUCL";
QString MSAColorScheme::UGENE_NUCL      = "COLOR_SCHEME_UGENE_NUCL";
QString MSAColorScheme::JALVIEW_NUCL    = "COLOR_SCHEME_JALVIEW_NUCL";
QString MSAColorScheme::IDENTPERC_NUCL  = "COLOR_SCHEME_IDENTPERC_NUCL";
QString MSAColorScheme::CUSTOM_NUCL       = "COLOR_SCHEME_CUSTOM_NUCL";

QString MSAColorScheme::EMPTY_AMINO     = "COLOR_SCHEME_EMPTY_AMINO";
QString MSAColorScheme::UGENE_AMINO     = "COLOR_SCHEME_UGENE_AMINO";
QString MSAColorScheme::ZAPPO_AMINO     = "COLOR_SCHEME_ZAPPO_AMINO";
QString MSAColorScheme::TAILOR_AMINO    = "COLOR_SCHEME_TAILOR_AMINO";
QString MSAColorScheme::HYDRO_AMINO     = "COLOR_SCHEME_HYDRO_AMINO";
QString MSAColorScheme::HELIX_AMINO     = "COLOR_SCHEME_HELIX_AMINO";
QString MSAColorScheme::STRAND_AMINO    = "COLOR_SCHEME_STRAND_AMINO";
QString MSAColorScheme::TURN_AMINO      = "COLOR_SCHEME_TURN_AMINO";
QString MSAColorScheme::BURIED_AMINO    = "COLOR_SCHEME_BURIED_AMINO";
QString MSAColorScheme::IDENTPERC_AMINO = "COLOR_SCHEME_IDENTPERC_AMINO";
QString MSAColorScheme::CLUSTALX_AMINO  = "COLOR_SCHEME_CLUSTALX_AMINO";
QString MSAColorScheme::CUSTOM_AMINO      = "COLOR_SCHEME_CUSTOM_AMINO";


void MSAColorSchemeRegistry::initCustomSchema(){
    QVector<QColor> colorsPerChar;

    foreach(const CustomColorSchema& schema, ColorSchemaSettingsUtils::getSchemas()){
        fillEmptyCS(colorsPerChar);
        QMapIterator<char, QColor> it(schema.alpColors);
        while(it.hasNext()){
            it.next();
            SET_C(it.key(), it.value());
        }
        addMSACustomColorSchemeFactory(new MSAColorSchemeCustomSettingsFactory(NULL, schema.name, schema.name, schema.type, colorsPerChar));
    }
}

void MSAColorSchemeRegistry::sl_onCustomSettingsChanged(){
    deleteOldCustomFactories();
    initCustomSchema();
    emit si_customSettingsChanged(); 
}

void MSAColorSchemeRegistry::initBuiltInSchemes() {
    QVector<QColor> colorsPerChar;

    //nucleic
    fillEmptyCS(colorsPerChar);
    addMSAColorSchemeFactory(new MSAColorSchemeStaticFactory(this, MSAColorScheme::EMPTY_NUCL, tr("No colors"), DNAAlphabet_NUCL, colorsPerChar));

    fillLightColorsCS(colorsPerChar);
    addUGENENucl(colorsPerChar);
    addMSAColorSchemeFactory(new MSAColorSchemeStaticFactory(this, MSAColorScheme::UGENE_NUCL, U2_APP_TITLE, DNAAlphabet_NUCL, colorsPerChar));

    fillEmptyCS(colorsPerChar);
    addJalviewNucl(colorsPerChar);
    addMSAColorSchemeFactory(new MSAColorSchemeStaticFactory(this, MSAColorScheme::JALVIEW_NUCL, tr("Jalview"), DNAAlphabet_NUCL, colorsPerChar));

    addMSAColorSchemeFactory(new MSAColorSchemePercIdentFactory(this, MSAColorScheme::IDENTPERC_NUCL,  tr("Percentage Identity"), DNAAlphabet_NUCL));

    //amino
    fillEmptyCS(colorsPerChar);
    addMSAColorSchemeFactory(new MSAColorSchemeStaticFactory(this, MSAColorScheme::EMPTY_AMINO, tr("No colors"), DNAAlphabet_AMINO, colorsPerChar));

    fillLightColorsCS(colorsPerChar);
    addUGENEAmino(colorsPerChar);
    addMSAColorSchemeFactory(new MSAColorSchemeStaticFactory(this, MSAColorScheme::UGENE_AMINO, U2_APP_TITLE, DNAAlphabet_AMINO, colorsPerChar));
    
    fillEmptyCS(colorsPerChar);
    addZappoAmino(colorsPerChar);
    addMSAColorSchemeFactory(new MSAColorSchemeStaticFactory(this, MSAColorScheme::ZAPPO_AMINO, tr("Zappo"), DNAAlphabet_AMINO, colorsPerChar));

    fillEmptyCS(colorsPerChar);
    addTailorAmino(colorsPerChar);
    addMSAColorSchemeFactory(new MSAColorSchemeStaticFactory(this, MSAColorScheme::TAILOR_AMINO, tr("Tailor"), DNAAlphabet_AMINO, colorsPerChar));

    fillEmptyCS(colorsPerChar);
    addHydroAmino(colorsPerChar);
    addMSAColorSchemeFactory(new MSAColorSchemeStaticFactory(this, MSAColorScheme::HYDRO_AMINO, tr("Hydrophobicity"), DNAAlphabet_AMINO, colorsPerChar));

    fillEmptyCS(colorsPerChar);
    addHelixAmino(colorsPerChar);
    addMSAColorSchemeFactory(new MSAColorSchemeStaticFactory(this, MSAColorScheme::HELIX_AMINO, tr("Helix propensity"), DNAAlphabet_AMINO, colorsPerChar));

    fillEmptyCS(colorsPerChar);
    addStrandAmino(colorsPerChar);
    addMSAColorSchemeFactory(new MSAColorSchemeStaticFactory(this, MSAColorScheme::STRAND_AMINO, tr("Strand propensity"), DNAAlphabet_AMINO, colorsPerChar));

    fillEmptyCS(colorsPerChar);
    addTurnAmino(colorsPerChar);
    addMSAColorSchemeFactory(new MSAColorSchemeStaticFactory(this, MSAColorScheme::TURN_AMINO, tr("Turn propensity"), DNAAlphabet_AMINO, colorsPerChar));

    fillEmptyCS(colorsPerChar);
    addBuriedAmino(colorsPerChar);
    addMSAColorSchemeFactory(new MSAColorSchemeStaticFactory(this, MSAColorScheme::BURIED_AMINO, tr("Buried index"), DNAAlphabet_AMINO, colorsPerChar));

    addMSAColorSchemeFactory(new MSAColorSchemePercIdentFactory(this, MSAColorScheme::IDENTPERC_AMINO, tr("Percentage Identity"), DNAAlphabet_AMINO));
    
    addMSAColorSchemeFactory(new MSAColorSchemeClustalXFactory(this, MSAColorScheme::CLUSTALX_AMINO,  tr("Clustal X"), DNAAlphabet_AMINO));   

}
////////////
//SNP

MSAHighlightingSchemeRegistry::MSAHighlightingSchemeRegistry(){
    schemes.append(new MSAHighlightingSchemeNoColorsFactory(this, MSAHighlightingScheme::EMPTY_NUCL, tr("No highlighting"), DNAAlphabet_NUCL));
    schemes.append(new MSAHighlightingSchemeNoColorsFactory(this, MSAHighlightingScheme::EMPTY_AMINO, tr("No highlighting"), DNAAlphabet_AMINO));

    schemes.append(new MSAHighlightingSchemeAgreementsFactory(this, MSAHighlightingScheme::AGREEMENTS_NUCL, tr("Agreements"), DNAAlphabet_NUCL));
    schemes.append(new MSAHighlightingSchemeAgreementsFactory(this, MSAHighlightingScheme::AGREEMENTS_AMINO, tr("Agreements"), DNAAlphabet_AMINO));

    schemes.append(new MSAHighlightingSchemeDisagreementsFactory(this, MSAHighlightingScheme::DISAGREEMENTS_NUCL, tr("Disagreements"), DNAAlphabet_NUCL));
    schemes.append(new MSAHighlightingSchemeDisagreementsFactory(this, MSAHighlightingScheme::DISAGREEMENTS_AMINO, tr("Disagreements"), DNAAlphabet_AMINO));

    schemes.append(new MSAHighlightingSchemeGapsFactory (this, MSAHighlightingScheme::GAPS_NUCL, tr("Gaps"), DNAAlphabet_NUCL));
    schemes.append(new MSAHighlightingSchemeGapsFactory (this, MSAHighlightingScheme::GAPS_AMINO, tr("Gaps"), DNAAlphabet_AMINO));

    schemes.append(new MSAHighlightingSchemeTransitionsFactory(this, MSAHighlightingScheme::TRANSITIONS_NUCL, tr("Transitions"), DNAAlphabet_NUCL));
    schemes.append(new MSAHighlightingSchemeTransversionsFactory(this, MSAHighlightingScheme::TRANSVERSIONS_NUCL, tr("Transversions"), DNAAlphabet_NUCL));
}

MSAHighlightingSchemeFactory* MSAHighlightingSchemeRegistry::getMSAHighlightingSchemeFactoryById( const QString& id ) const {
    foreach(MSAHighlightingSchemeFactory* f, schemes) {
        if(f->getId() == id) {
            return f;
        }
    }
    return NULL;
}

QList<MSAHighlightingSchemeFactory*> MSAHighlightingSchemeRegistry::getMSAHighlightingSchemes( DNAAlphabetType atype ){
    QList<MSAHighlightingSchemeFactory*> res;
    foreach(MSAHighlightingSchemeFactory* f, schemes) {
        if (f->getAlphabetType() == atype) {
            res.append(f);
        }
    }
    return res;
}

//Factories
MSAHighlightingScheme* MSAHighlightingSchemeNoColorsFactory::create( QObject* p, MAlignmentObject* obj ){
    return new MSAHighlightingSchemeEmpty(p, this, obj);
}

MSAHighlightingScheme* MSAHighlightingSchemeAgreementsFactory::create( QObject* p, MAlignmentObject* obj ){
    return new MSAHighlightingSchemeAgreements(p, this, obj);
}

MSAHighlightingScheme* MSAHighlightingSchemeDisagreementsFactory::create(QObject* p, MAlignmentObject* obj){
    return new MSAHighlightingSchemeDisagreements(p, this, obj);
}


MSAHighlightingScheme* MSAHighlightingSchemeTransitionsFactory::create( QObject* p, MAlignmentObject* obj ){
    return new MSAHighlightingSchemeTransitions(p, this, obj);
}

MSAHighlightingScheme* MSAHighlightingSchemeTransversionsFactory::create( QObject* p, MAlignmentObject* obj ){
    return new MSAHighlightingSchemeTransversions(p, this, obj);
}

MSAHighlightingScheme* MSAHighlightingSchemeGapsFactory::create( QObject* p, MAlignmentObject* obj ){
    return new MSAHighlightingSchemeGaps(p, this, obj);
}

//Schemes

MSAHighlightingScheme::MSAHighlightingScheme( QObject* p, MSAHighlightingSchemeFactory* f, MAlignmentObject* o ): 
    QObject(p), factory(f), maObj(o), useDots(false){
    
}

void MSAHighlightingScheme::process( const char /*refChar*/, char &seqChar, bool &color ){
    if (useDots && !color){
        seqChar = '.';
    }
}

QString MSAHighlightingScheme::EMPTY_NUCL = "HIGHLIGHT_SCHEME_EMPTY_NUCL";
QString MSAHighlightingScheme::EMPTY_AMINO = "HIGHLIGHT_SCHEME_EMPTY_AMINO";
QString MSAHighlightingScheme::AGREEMENTS_NUCL = "HIGHLIGHT_SCHEME_AGREEMENTS_NUCL";
QString MSAHighlightingScheme::AGREEMENTS_AMINO = "HIGHLIGHT_SCHEME_AGREEMENTS_AMINO";
QString MSAHighlightingScheme::DISAGREEMENTS_NUCL = "HIGHLIGHT_SCHEME_DISAGREEMENTS_NUCL";
QString MSAHighlightingScheme::DISAGREEMENTS_AMINO = "HIGHLIGHT_SCHEME_DISAGREEMENTS_AMINO";
QString MSAHighlightingScheme::TRANSITIONS_NUCL = "HIGHLIGHT_SCHEME_TRANSITIONS_AMINO";
QString MSAHighlightingScheme::TRANSVERSIONS_NUCL = "HIGHLIGHT_SCHEME_TRANSVERSIONS_AMINO";
QString MSAHighlightingScheme::GAPS_AMINO = "HIGHLIGHT_SCHEME_GAPS_AMINO";
QString MSAHighlightingScheme::GAPS_NUCL = "HIGHLIGHT_SCHEME_GAPS_NUCL";

void MSAHighlightingSchemeAgreements::process( const char refChar, char &seqChar, bool &color ){
    if(refChar == seqChar){
        color = true;
    }else{
        color = false;
    }
    MSAHighlightingScheme::process(refChar, seqChar, color);
}


void MSAHighlightingSchemeDisagreements::process( const char refChar, char &seqChar, bool &color ){
    if(refChar == seqChar){
        color = false;
    }else{
        color = true;
    }
    MSAHighlightingScheme::process(refChar, seqChar, color);
}


void MSAHighlightingSchemeTransitions::process( const char refChar, char &seqChar, bool &color ){
    switch (refChar){
        case 'N':
            color = true;
            break;
        case 'A':
            color = (seqChar == 'G');
            break;
        case 'C':
            color = (seqChar == 'T');
            break;
        case 'G':
            color = (seqChar == 'A');
            break;
        case 'T':
            color = (seqChar == 'C');
            break;
        default:
            color = false;
            break;
    }
    MSAHighlightingScheme::process(refChar, seqChar, color);
}


void MSAHighlightingSchemeTransversions::process( const char refChar, char &seqChar, bool &color ){
    switch (refChar){
        case 'N':
            color = true;
            break;
        case 'A':
            color = (seqChar == 'C' || seqChar == 'T');
            break;
        case 'C':
            color = (seqChar == 'A' || seqChar == 'G');
            break;
        case 'G':
            color = (seqChar == 'C' || seqChar == 'T');
            break;
        case 'T':
            color = (seqChar == 'A' || seqChar == 'G');
            break;
        default:
            color = false;
            break;
    }
    MSAHighlightingScheme::process(refChar, seqChar, color);
}

void MSAHighlightingSchemeGaps::process( const char refChar, char &seqChar, bool &color ){
    if(seqChar == '-'){
        color = true;
    }else{
        color = false;
    }
    MSAHighlightingScheme::process(refChar, seqChar, color);
}

}//namespace
