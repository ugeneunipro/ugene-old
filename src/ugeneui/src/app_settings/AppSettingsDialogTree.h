#ifndef _U2_APP_SETTINGS_DIALOG_TREE_H_
#define _U2_APP_SETTINGS_DIALOG_TREE_H_

#include <QtGui/QTreeWidget>

namespace U2 {
    class AppSettingsDialogController;
}

class AppSettingsDialogTree : public QTreeWidget {
    Q_OBJECT
public:
    AppSettingsDialogTree(QWidget* w = NULL) : QTreeWidget(w){}
    void setController(U2::AppSettingsDialogController* c) {controller = c;}
private:
    U2::AppSettingsDialogController* controller;
};

#endif
