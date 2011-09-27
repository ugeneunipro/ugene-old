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
#include <U2Core/U2SafePoints.h>

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
    virtual QPixmap cellImage(const U2AssemblyRead &read, char c, char ref);

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

QPixmap NucleotideColorsRenderer::cellImage(const U2AssemblyRead&, char c, char) {
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
    virtual QPixmap cellImage(const U2AssemblyRead &read, char c, char ref);

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
    GTIMER(c1, t1, "ComplementColorsRenderer::render");

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

QPixmap ComplementColorsRenderer::cellImage(const U2AssemblyRead &read, char c, char) {
    return cellImage(read, c);
}

class DiffNucleotideColorsRenderer : public AssemblyCellRenderer {
public:
    DiffNucleotideColorsRenderer();
    virtual ~DiffNucleotideColorsRenderer() {}

    virtual void render(const QSize &size, bool text, const QFont &font);

    virtual QPixmap cellImage(char c);
    virtual QPixmap cellImage(const U2AssemblyRead &read, char c);
    virtual QPixmap cellImage(const U2AssemblyRead &read, char c, char ref);

private:
    void update();

private:
    QMap<char, QColor> colorScheme;

    // images cache
    QHash<char, QPixmap> highlightedImages;
    QHash<char, QPixmap> normalImages;
    QPixmap unknownChar;

    // cached cells parameters
    QSize size;
    bool text;
    QFont font;
};

class PairedColorsRenderer : public AssemblyCellRenderer {
public:
    PairedColorsRenderer()
        : AssemblyCellRenderer(),
          pairedImages(), unpairedImages(), unknownChar(),
          size(), text(false), font() {}

    virtual ~PairedColorsRenderer() {}

    virtual void render(const QSize &size, bool text, const QFont &font);

    virtual QPixmap cellImage(char c);
    virtual QPixmap cellImage(const U2AssemblyRead &read, char c);
    virtual QPixmap cellImage(const U2AssemblyRead &read, char c, char ref);

private:
    void update();

private:
    // images cache
    QHash<char, QPixmap> pairedImages, unpairedImages;
    QPixmap unknownChar;

    // cached cells parameters
    QSize size;
    bool text;
    QFont font;

private:
    static const QColor pairedColor, unpairedColor;
};

const QColor PairedColorsRenderer::pairedColor("#4EE1AD");
const QColor PairedColorsRenderer::unpairedColor("#BBBBBB");

void PairedColorsRenderer::render(const QSize &_size, bool _text, const QFont &_font) {
    GTIMER(c1, t1, "PairedReadsColorsRenderer::render");

    if (_size != size || _text != text || (text && _font != font)) {
        // update cache
        size = _size, text = _text, font = _font;
        update();
    }
}

void PairedColorsRenderer::update() {
    pairedImages.clear();
    unpairedImages.clear();

    foreach(char c, assemblyAlphabet) {
        QPixmap pimg(size), npimg(size);
        QColor pcolor = pairedColor, ucolor = unpairedColor, textColor = Qt::black;

        if (isGap(c)) {
            pcolor = ucolor =  QColor("#FBFBFB");
            textColor = Qt::red;
        }

        drawCell(pimg, pcolor, text, c, font, textColor);
        drawCell(npimg, ucolor, text, c, font, textColor);

        pairedImages.insert(c, pimg);
        unpairedImages.insert(c, npimg);
    }

    unknownChar = QPixmap(size);
    drawCell(unknownChar, QColor("#FBFBFB"), text, '?', font, Qt::red);
}

QPixmap PairedColorsRenderer::cellImage(char c) {
    c = (!nucleotideColorScheme.contains(c)) ? 'N' : c;
    return unpairedImages.value(c, unknownChar);
}

QPixmap PairedColorsRenderer::cellImage(const U2AssemblyRead &read, char c) {
    c = (!nucleotideColorScheme.contains(c)) ? 'N' : c;

    if (ReadFlagsUtils::isPairedRead(read->flags)) {
        return pairedImages.value(c, unknownChar);
    } else {
        return unpairedImages.value(c, unknownChar);
    }
}

QPixmap PairedColorsRenderer::cellImage(const U2AssemblyRead &read, char c, char) {
    return cellImage(read, c);
}

DiffNucleotideColorsRenderer::DiffNucleotideColorsRenderer()
    : AssemblyCellRenderer(), colorScheme(nucleotideColorScheme),
  highlightedImages(), normalImages(), unknownChar(), size(), text(false), font() {}

void DiffNucleotideColorsRenderer::render(const QSize &_size, bool _text, const QFont &_font) {
    GTIMER(c1, t1, "DiffNucleotideColorsRenderer::render");

    if (_size != size || _text != text || (text && _font != font)) {
        // update cache
        size = _size, text = _text, font = _font;
        update();
    }
}

void DiffNucleotideColorsRenderer::update() {
    highlightedImages.clear();
    normalImages.clear();

    QColor normalColor("#BBBBBB");

    foreach(char c, colorScheme.keys()) {
        QPixmap himg(size), nimg(size);
        // make gaps more noticeable
        QColor textColor = isGap(c) ? Qt::white : Qt::black;
        QColor highlightColor = isGap(c) ? Qt::red : colorScheme.value(c);

        drawCell(himg, highlightColor, text, c, font, textColor);
        drawCell(nimg, normalColor, text, c, font, textColor);

        highlightedImages.insert(c, himg);
        normalImages.insert(c, nimg);
    }

    unknownChar = QPixmap(size);
    drawCell(unknownChar, QColor("#FBFBFB"), text, '?', font, Qt::red);
}

QPixmap DiffNucleotideColorsRenderer::cellImage(char c) {
    c = (!nucleotideColorScheme.contains(c)) ? 'N' : c;
    return highlightedImages.value(c, unknownChar);
}

QPixmap DiffNucleotideColorsRenderer::cellImage(const U2AssemblyRead&, char c) {
    return cellImage(c);
}

QPixmap DiffNucleotideColorsRenderer::cellImage(const U2AssemblyRead&, char c, char ref) {
    c = (!nucleotideColorScheme.contains(c)) ? 'N' : c;

    if(c == ref) {
        return normalImages.value(c, unknownChar);
    } else {
        return highlightedImages.value(c, unknownChar);
    }
    return cellImage(c);
}

//////////////////////////////////////////////////////////////////////////
// factories

AssemblyCellRendererFactory::AssemblyCellRendererFactory(const QString &_id, const QString &_name)
    : id(_id), name(_name)
{}

QString AssemblyCellRendererFactory::ALL_NUCLEOTIDES = "ASSEMBLY_RENDERER_ALL_NUCLEOTIDES";
QString AssemblyCellRendererFactory::DIFF_NUCLEOTIDES = "ASSEMBLY_RENDERER_DIFF_NUCLEOTIDES";
QString AssemblyCellRendererFactory::STRAND_DIRECTION = "ASSEMBLY_RENDERER_STRAND_DIRECTION";
QString AssemblyCellRendererFactory::PAIRED = "ASSEMBLY_RENDERER_PAIRED";

class NucleotideColorsRendererFactory : public AssemblyCellRendererFactory {
public:
    NucleotideColorsRendererFactory(const QString &_id, const QString &_name)
        : AssemblyCellRendererFactory(_id, _name) {}

    virtual AssemblyCellRenderer* create() {
        return new NucleotideColorsRenderer();
    }
};

class DiffNucleotideColorsRendererFactory : public AssemblyCellRendererFactory {
public:
    DiffNucleotideColorsRendererFactory(const QString &_id, const QString &_name)
        : AssemblyCellRendererFactory(_id, _name) {}

    virtual AssemblyCellRenderer* create() {
        return new DiffNucleotideColorsRenderer();
    }
};

class ComplementColorsRendererFactory : public AssemblyCellRendererFactory {
public:
    ComplementColorsRendererFactory(const QString &_id, const QString &_name)
        : AssemblyCellRendererFactory(_id, _name) {}

    virtual AssemblyCellRenderer* create() {
        return new ComplementColorsRenderer();
    }
};

class PairedColorsRendererFactory : public AssemblyCellRendererFactory {
public:
    PairedColorsRendererFactory(const QString &_id, const QString &_name)
        : AssemblyCellRendererFactory(_id, _name) {}

    virtual AssemblyCellRenderer* create() {
        return new PairedColorsRenderer();
    }
};

//////////////////////////////////////////////////////////////////////////
// registry

AssemblyCellRendererFactoryRegistry::AssemblyCellRendererFactoryRegistry(QObject * parent)
    : QObject(parent)
{
    initBuiltInRenderers();
}

AssemblyCellRendererFactory* AssemblyCellRendererFactoryRegistry::getFactoryById(const QString &id) const {
    foreach(AssemblyCellRendererFactory *f, factories) {
        if(f->getId() == id) {
            return f;
        }
    }
    return NULL;
}

void AssemblyCellRendererFactoryRegistry::addFactory(AssemblyCellRendererFactory *f) {
    assert(getFactoryById(f->getId()) == NULL);
    factories << f;
}

void AssemblyCellRendererFactoryRegistry::initBuiltInRenderers() {
    addFactory(new NucleotideColorsRendererFactory(AssemblyCellRendererFactory::ALL_NUCLEOTIDES,
                                                   tr("Nucleotide")));
    addFactory(new DiffNucleotideColorsRendererFactory(AssemblyCellRendererFactory::DIFF_NUCLEOTIDES,
                                                   tr("Difference")));
    addFactory(new ComplementColorsRendererFactory(AssemblyCellRendererFactory::STRAND_DIRECTION,
                                                   tr("Strand direction")));
    addFactory(new PairedColorsRendererFactory(AssemblyCellRendererFactory::PAIRED,
                                                   tr("Paired reads")));
}

AssemblyCellRendererFactoryRegistry::~AssemblyCellRendererFactoryRegistry() {
    foreach(AssemblyCellRendererFactory *f, factories) {
        delete f;
    }
}

}   // namespace U2
