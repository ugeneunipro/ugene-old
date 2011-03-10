#ifndef _U2_WORKFLOW_GUI_UTILS_H_
#define _U2_WORKFLOW_GUI_UTILS_H_

#include <U2Core/global.h>

class QPainter;
class QTextDocument;
class QPalette;
class QIcon;

namespace U2 {

class Descriptor;

class U2DESIGNER_EXPORT DesignerGUIUtils {
public:
    static void paintSamplesArrow(QPainter* painter);
    static void paintSamplesDocument(QPainter* painter, QTextDocument* doc, int pageWidth, int pageHeight, const QPalette& pal);
    static void setupSamplesDocument(const Descriptor& d, const QIcon& ico, QTextDocument* res);
};

}//namespace

#endif
