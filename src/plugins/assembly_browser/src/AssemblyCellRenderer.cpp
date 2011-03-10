#include "AssemblyCellRenderer.h"

#include <QtGui/QPainter>
#include <QtGui/QFont>

#include <U2Core/Timer.h>

namespace U2 {

namespace {
    QMap<char, QColor> initDefaultColorSheme() {
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
}

const QMap<char, QColor> defaultColorScheme = initDefaultColorSheme();


AssemblyCellRenderer::AssemblyCellRenderer(const QMap<char, QColor> & colorScheme_) :
colorScheme(colorScheme_), cachedTextFlag(false) {
}

QVector<QImage> AssemblyCellRenderer::render(const QSize & size, bool text /*= false*/, const QFont & font /*= QFont()*/) {
    GTIMER(c1, t1, "AssemblyCellRenderer::render");
    GCOUNTER(c2, t2, "AssemblyCellRenderer::render -> calls");
    if(images.empty() || (cachedSize != size || cachedTextFlag != text)) {
        drawCells(size, font, text);
    }
    return images;
}

void AssemblyCellRenderer::drawCells(const QSize & size, const QFont & font, bool text) {
    images = QVector<QImage>(256, QImage(size, QImage::Format_RGB32));
    foreach(char c, colorScheme.keys()) {
        drawCell(c, colorScheme.value(c));
        if(text) {
            drawText(c, font);
        }
    }
}

void AssemblyCellRenderer::drawText(char c, const QFont & f) {
    QImage & result = images[c];
    QPainter p(&result);
    p.setFont(f);
    if('-' == c || 'N' == c) { //TODO : get rid of hardcoded values!
        p.setPen(Qt::red);
    } 
    p.drawText(result.rect(), Qt::AlignCenter, QString(c));

}

void AssemblyCellRenderer::drawCell(char c, const QColor & color) {
    QImage & result = images[c];
    QPainter p(&result);

    //TODO invent something greater
    QLinearGradient linearGrad(QPointF(0, 0), QPointF(result.width(), result.height()));
    linearGrad.setColorAt(0, QColor::fromRgb(color.red()-70,color.green()-70,color.blue()-70));
    linearGrad.setColorAt(1, color);
    QBrush br(linearGrad);

    QRect imgRect(0, 0, result.width(), result.height());
    p.fillRect(imgRect, br);
}


} //ns
