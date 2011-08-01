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

#include "AssemblyCellRenderer.h"

#include <QtGui/QPainter>
#include <QtGui/QFont>

#include <U2Core/Timer.h>

namespace U2 {

namespace {

static QMap<char, QColor> initDefaultColorSheme() {
    QMap<char, QColor> colors;

    //TODO other chars ??
    //TODO = symbol
    colors['a'] = QColor("#FCFF92");
    colors['c'] = QColor("#70F970");
    colors['g'] = QColor("#4EADE1");
    colors['t'] = QColor("#FF99B1");
    colors['A'] = QColor("#FCFF92");
    colors['C'] = QColor("#70F970");
    colors['G'] = QColor("#4EADE1");
    colors['T'] = QColor("#FF99B1");

    colors['-'] = QColor("#FBFBFB");
    colors['N'] = QColor("#FBFBFB");

    return colors;
}

static QList<char> initAssemblyAlphabet() {
    QList<char> alphabet;
    alphabet << 'a' << 'c' << 'g' << 't'
             << 'A' << 'C' << 'G' << 'T'
             << '-' << 'N';
    return alphabet;
}

inline static bool isGap(char c) {
    //TODO : get rid of hardcoded values!
    //TODO: smarter analysis. Don't forget about '=' symbol and IUPAC codes
    return (c == '-' || c == 'N');
}

}   // namespace {

static const QMap<char, QColor> nucleotideColorScheme = initDefaultColorSheme();
static const QList<char> assemblyAlphabet = initAssemblyAlphabet();

void AssemblyCellRenderer::drawCell(QPixmap &img, const QColor &color, bool text, char c, const QFont &font, const QColor &textColor) {
    drawBackground(img, color);

    if(text) {
        drawText(img, c, font, textColor);
    }
}

void AssemblyCellRenderer::drawBackground(QPixmap &img, const QColor & color) {
    QPainter p(&img);

    //TODO invent something greater
    QLinearGradient linearGrad( QPointF(0, 0), QPointF(img.width(), img.height()));
    linearGrad.setColorAt(0, QColor::fromRgb(color.red()-70,color.green()-70,color.blue()-70));
    linearGrad.setColorAt(1, color);
    QBrush br(linearGrad);

    p.fillRect(img.rect(), br);
}

void AssemblyCellRenderer::drawText(QPixmap &img, char c, const QFont &font, const QColor &color) {
    QPainter p(&img);
    p.setFont(font);
    p.setPen(color);
    p.drawText(img.rect(), Qt::AlignCenter, QString(c));
}

class NucleotideColorsRenderer : public AssemblyCellRenderer {
public:
    NucleotideColorsRenderer()
        : AssemblyCellRenderer(), colorScheme(nucleotideColorScheme),
          images(), unknownChar(), size(), text(false), font() {}
    virtual ~NucleotideColorsRenderer() {}

    virtual void render(const QSize &size, bool text, const QFont &font);

    virtual QPixmap cellImage(char c);
    virtual QPixmap cellImage(const U2AssemblyRead &read, char c);

private:
    void update();

private:
    QMap<char, QColor> colorScheme;

    // images cache
    QHash<char, QPixmap> images;
    QPixmap unknownChar;

    // cached cells parameters
    QSize size;
    bool text;
    QFont font;
};

void NucleotideColorsRenderer::render(const QSize &_size, bool _text, const QFont &_font) {
    GTIMER(c1, t1, "NucleotideColorsRenderer::render");
    GCOUNTER(c2, t2, "NucleotideColorsRenderer::render -> calls");

    if (_size != size || _text != text || (text && _font != font)) {
        // update cache
        size = _size, text = _text, font = _font;
        update();
    }
}

void NucleotideColorsRenderer::update() {
    images.clear();

    foreach(char c, colorScheme.keys()) {
        QPixmap img(size);
        QColor textColor = isGap(c) ? Qt::red : Qt::black;
        drawCell(img, colorScheme.value(c), text, c, font, textColor);
        images.insert(c, img);
    }

    unknownChar = QPixmap(size);
    drawCell(unknownChar, QColor("#FBFBFB"), text, '?', font, Qt::red);
}

QPixmap NucleotideColorsRenderer::cellImage(char c) {
    c = (!nucleotideColorScheme.contains(c)) ? 'N' : c;
    return images.value(c, unknownChar);
}

QPixmap NucleotideColorsRenderer::cellImage(const U2AssemblyRead&, char c) {
    return cellImage(c);
}

class ComplementColorsRenderer : public AssemblyCellRenderer {
public:
    ComplementColorsRenderer()
        : AssemblyCellRenderer(),
          directImages(), complementImages(), unknownChar(),
          size(), text(false), font() {}

    virtual ~ComplementColorsRenderer() {}

    virtual void render(const QSize &size, bool text, const QFont &font);

    virtual QPixmap cellImage(char c);
    virtual QPixmap cellImage(const U2AssemblyRead &read, char c);

private:
    void update();

private:    
    // images cache
    QHash<char, QPixmap> directImages, complementImages;
    QPixmap unknownChar;

    // cached cells parameters
    QSize size;
    bool text;
    QFont font;

private:
    static const QColor directColor, complementColor;
};

const QColor ComplementColorsRenderer::directColor("#4EADE1");
const QColor ComplementColorsRenderer::complementColor("#70F970");

void ComplementColorsRenderer::render(const QSize &_size, bool _text, const QFont &_font) {
    GTIMER(c1, t1, "NucleotideColorsRenderer::render");
    GCOUNTER(c2, t2, "NucleotideColorsRenderer::render -> calls");

    if (_size != size || _text != text || (text && _font != font)) {
        // update cache
        size = _size, text = _text, font = _font;
        update();
    }
}

void ComplementColorsRenderer::update() {
    directImages.clear();
    complementImages.clear();

    foreach(char c, assemblyAlphabet) {
        QPixmap dimg(size), cimg(size);
        QColor dcolor = directColor, ccolor = complementColor, textColor = Qt::black;

        if (isGap(c)) {
            dcolor = ccolor =  QColor("#FBFBFB");
            textColor = Qt::red;
        }

        drawCell(dimg, dcolor, text, c, font, textColor);
        drawCell(cimg, ccolor, text, c, font, textColor);

        directImages.insert(c, dimg);
        complementImages.insert(c, cimg);
    }

    unknownChar = QPixmap(size);
    drawCell(unknownChar, QColor("#FBFBFB"), text, '?', font, Qt::red);
}

QPixmap ComplementColorsRenderer::cellImage(char c) {
    c = (!nucleotideColorScheme.contains(c)) ? 'N' : c;
    return directImages.value(c, unknownChar);
}

QPixmap ComplementColorsRenderer::cellImage(const U2AssemblyRead &read, char c) {
    c = (!nucleotideColorScheme.contains(c)) ? 'N' : c;

    if (ReadFlagsUtils::isComplementaryRead(read->flags)) {
        return complementImages.value(c, unknownChar);
    }
    else {
        return directImages.value(c, unknownChar);
    }
}


extern AssemblyCellRenderer* createAssemblyCellRenderer() {
    return new NucleotideColorsRenderer();
    //return new ComplementColorsRenderer();
}

}   // namespace U2
