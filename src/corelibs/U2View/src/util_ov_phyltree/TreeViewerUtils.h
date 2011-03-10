#ifndef _TREE_VIEWER_UTILS_H_
#define _TREE_VIEWER_UTILS_H

#include <QtGui/QFont>

namespace U2 {

class TreeViewerUtils {
    static QFont* font;
public:
    static const char* IMAGE_FILTERS;
    static void saveImageDialog(const QString& filters, QString &fileName, QString &format);
    static const QFont& getFont();
};

}

#endif