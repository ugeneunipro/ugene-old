#if !defined(GRAPHUTILS_H)
#define GRAPHUTILS_H

#include <U2Core/global.h>

#include <QtGui/QPainter>
#include <QtGui/QFont>

namespace U2 {

#define PI 3.1415926535897932384626433832795

class U2MISC_EXPORT GraphUtils {
public:

    enum Direction { LTR, RTL, BTT, TTB };

    enum TextPos { LEFT, RIGHT };
    
    struct RulerConfig {
        RulerConfig() {
            drawArrow = false;
            arrowLen = 5;
            arrowWidth = 2;
            notchSize = 2;
            textOffset = 3;
            drawNumbers = true;
            drawNotches = true;
            drawBorderNotches = true;
            singleSideNotches = false;
            drawAxis = true;
            direction = LTR;
            textPosition = RIGHT;
            extraAxisLenBefore =0;
            extraAxisLenAfter =0;
            textBorderStart = 2;
            textBorderEnd = 2;
        }
        bool    drawArrow;
        int     arrowLen;
        int     arrowWidth;
        int     notchSize;
        int     textOffset;
        int     extraAxisLenBefore;
        int     extraAxisLenAfter;
        int     textBorderStart; //offset in pixels for the first text label at the start pos
        int     textBorderEnd;   //offset in pixels for the last text label at the end pos
        bool    drawNumbers;
        bool    drawNotches;
        bool    drawBorderNotches;
        bool    singleSideNotches;
        bool    drawAxis;
        Direction direction;
        TextPos   textPosition;
    };

    static void drawRuler(QPainter& p, const QPoint& pos, qint64 len, qint64 start, qint64 end, const QFont& f, const RulerConfig& config);
    static int findChunk(qint64 len, qint64 span, int N);

    //static void drawDensityPlot(QPainter& p, QRect& drawRect, QRect& calcRect, quint32 n, quint32* x, quint32 *y, quint32* len);

    static QColor proposeLightColorByKey(const QString& key);
};

} //namespace

#endif
