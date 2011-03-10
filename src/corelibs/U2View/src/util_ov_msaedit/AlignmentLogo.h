#ifndef _U2_ALIGNMENT_LOGO_H_
#define _U2_ALIGNMENT_LOGO_H_

#include <U2Core/global.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/MAlignment.h>
#include <U2Core/MAlignmentInfo.h>

#include <QtGui/QMainWindow>
#include <QtGui/QGraphicsItem>

namespace U2 {

class MSAEditor;

enum SequenceType {Auto, NA, AA};
/************************************************************************/
/* Settings                                                             */
/************************************************************************/
class U2VIEW_EXPORT AlignmentLogoSettings {
public:

    AlignmentLogoSettings(const MAlignment& _ma) : ma(_ma) {
        for (int i = 0; i < 256; i++) {
            colorScheme[i] = Qt::black;
        }

        if (ma.getAlphabet()->isNucleic()) {
            sequenceType = NA;
            colorScheme['G'] = QColor(255, 128, 0);
            colorScheme['T'] = Qt::red;
            colorScheme['C'] = Qt::blue;
            colorScheme['A'] = Qt::green;
            colorScheme['U'] = Qt::red;
        } else if (ma.getAlphabet()->isAmino()) {
            sequenceType = AA;
        } else {
            sequenceType = Auto;
        }

        if (!ma.getAlphabet()->isNucleic()) {
            colorScheme['G'] = Qt::green;
            colorScheme['S'] = Qt::green;
            colorScheme['T'] = Qt::green;
            colorScheme['Y'] = Qt::green;
            colorScheme['C'] = Qt::green;
            colorScheme['N'] = QColor(192, 0, 192);
            colorScheme['Q'] = QColor(192, 0, 192);
            colorScheme['K'] = Qt::blue;
            colorScheme['R'] = Qt::blue;
            colorScheme['H'] = Qt::blue;
            colorScheme['D'] = Qt::red;
            colorScheme['E'] = Qt::red;
            colorScheme['P'] = Qt::black;
            colorScheme['A'] = Qt::black;
            colorScheme['W'] = Qt::black;
            colorScheme['F'] = Qt::black;
            colorScheme['L'] = Qt::black;
            colorScheme['I'] = Qt::black;
            colorScheme['M'] = Qt::black;
            colorScheme['V'] = Qt::black;
        }
        startPos = 0;
        len = ma.getLength();
        
        /*colorScheme.insert('S', Qt::green);
        colorScheme.insert('G', Qt::green);
        colorScheme.insert('H', Qt::green);
        colorScheme.insert('T', Qt::green);
        colorScheme.insert('A', Qt::green);
        colorScheme.insert('P', Qt::green);
        colorScheme.insert('Y', Qt::blue);
        colorScheme.insert('V', Qt::blue);
        colorScheme.insert('M', Qt::blue);
        colorScheme.insert('C', Qt::blue);
        colorScheme.insert('L', Qt::blue);
        colorScheme.insert('F', Qt::blue);
        colorScheme.insert('I', Qt::blue);
        colorScheme.insert('W', Qt::blue);*/
    }

    MAlignment              ma;
    SequenceType            sequenceType;
    int                     startPos;
    int                     len;
    QColor                  colorScheme[256];
};

/************************************************************************/
/* LogoRenderArea                                                       */
/************************************************************************/
class U2VIEW_EXPORT AlignmentLogoRenderArea : public QWidget {
public:
    AlignmentLogoRenderArea(const AlignmentLogoSettings& s, QWidget* p);
    void replaceSettings(const AlignmentLogoSettings& s);

protected:
    virtual void paintEvent(QPaintEvent* e);
    virtual void resizeEvent(QResizeEvent* e);

    void evaluateHeights();
    void sortCharsByHeight();
    qreal getH(int pos);

private:
    AlignmentLogoSettings           settings;
    qreal                           s;//4||20
    qreal                           error;
    QVector<char>*                  acceptableChars;
    QVector<char>                   bases;
    QVector<char>                   aminoacids;

    QVector<QVector<char> >         columns;
    //frequency and height of a particular char at position
    QVector<qreal>                  frequencies[256];
    QVector<qreal>                  heights[256];

    int                             bitWidth;
    int                             bitHeight;
};

/************************************************************************/
/* LogoItem                                                             */
/************************************************************************/
class AlignmentLogoItem : public QGraphicsItem {
public:
    AlignmentLogoItem(char _ch, QPointF _baseline, int _charWidth, int _charHeight, QFont _font, QColor _color=Qt::black);

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget /* = 0 */);
private:
    char            ch;
    QPointF         baseline;
    int             charWidth;
    int             charHeight;
    QFont           font;
    QColor          color;
    QPainterPath    path;
};

}//namespace

#endif
