#ifndef _U2_APPSETTINGS_DIALOG_CONTROLLER_H_
#define _U2_APPSETTINGS_DIALOG_CONTROLLER_H_

#include <ui/ui_AppSettingsDialog.h>

#include <QtGui/QTreeWidgetItem>
#include <QtCore/QTimerEvent>

class AppSettingsDialogTree;

namespace U2 {

class AppSettingsGUIPageController;
class AppSettingsGUIPageState;
class AppSettingsGUIPageWidget;
class AppSettingsTreeItem;

class AppSettingsDialogController: public QDialog, public Ui_AppSettingsDialog {
    Q_OBJECT

public:
    AppSettingsDialogController(const QString& pageId = QString(), QWidget *p = NULL);

public slots:
    virtual void accept();
    virtual void reject();
    void sl_currentItemChanged ( QTreeWidgetItem * current, QTreeWidgetItem * previous );

protected:
    void timerEvent(QTimerEvent* e);

private:
    bool checkCurrentState(bool saveStateInItem, bool showError);
    bool turnPage(AppSettingsTreeItem* page);
    void registerPage(AppSettingsGUIPageController* page);
    
    AppSettingsTreeItem* findPageItem(const QString& id) const;

private:
    AppSettingsTreeItem*    currentPage;
};

class AppSettingsTreeItem : public QTreeWidgetItem {
public:
    AppSettingsTreeItem(AppSettingsGUIPageController* pageController);
    AppSettingsGUIPageController*   pageController;
    AppSettingsGUIPageState*        pageState;
    AppSettingsGUIPageWidget*       pageWidget;
};

}//namespace

#endif
