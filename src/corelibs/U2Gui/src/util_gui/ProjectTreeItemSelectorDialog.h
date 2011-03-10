#ifndef _U2_PROJECT_TREE_ITEM_SELECTOR_DIALOG_H_
#define _U2_PROJECT_TREE_ITEM_SELECTOR_DIALOG_H_

#include <U2Core/global.h>

class QWidget;

namespace U2 {

class ProjectTreeControllerModeSettings;
class Document;
class GObject;

class U2GUI_EXPORT ProjectTreeItemSelectorDialog {
public:
    static QList<Document*> selectDocuments(const ProjectTreeControllerModeSettings& s, QWidget* p);
    static QList<GObject*> selectObjects(const ProjectTreeControllerModeSettings& s, QWidget* p);
};



}//namespace
#endif
