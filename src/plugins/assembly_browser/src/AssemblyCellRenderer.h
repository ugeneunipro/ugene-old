#ifndef __ASSEMBLY_CELL_RENDERER__
#define __ASSEMBLY_CELL_RENDERER__

#include <QtCore/QMap>
#include <QtCore/QVector>

#include <QtGui/QColor>
#include <QtGui/QImage>
#include <QtGui/QFont>

namespace U2 {

//TODO: implement real color schemes as for MSA 
extern const QMap<char, QColor> defaultColorScheme;

class AssemblyCellRenderer {
public:
    AssemblyCellRenderer(const QMap<char, QColor> & colorScheme_ = defaultColorScheme);

    QVector<QImage> render(const QSize & size, bool text = false, const QFont & font = QFont());

private:
    void drawCells(const QSize & size, const QFont & font, bool text);

    void drawText(char c, const QFont & f);

    void drawCell(char c, const QColor & color);

private:
    QMap<char, QColor> colorScheme;
    QVector<QImage> images;
    QSize cachedSize;
    bool cachedTextFlag;
};

} //ns

#endif
