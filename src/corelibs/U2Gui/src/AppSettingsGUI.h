#ifndef _U2_APPSETTINGS_GUI_H_
#define _U2_APPSETTINGS_GUI_H_

#include <U2Core/global.h>
#include <QtGui/QWidget>

namespace U2 {

class AppSettingsGUIPageController;

/** public class to access GUI */
class U2GUI_EXPORT AppSettingsGUI : public QObject {
    Q_OBJECT
public:
    AppSettingsGUI(QObject* p = NULL) : QObject(p){};
    
    virtual bool registerPage(AppSettingsGUIPageController* page, const QString& beforePage = QString()) = 0;

    virtual bool unregisterPage(AppSettingsGUIPageController* page) = 0;

    virtual void showSettingsDialog(const QString& pageId = QString()) const = 0;

    virtual QList<AppSettingsGUIPageController*> getRegisteredPages() const = 0;
};


/** Data stored on the page */
class U2GUI_EXPORT AppSettingsGUIPageState: public QObject {
    Q_OBJECT
public:
    AppSettingsGUIPageState(QObject* p = NULL) : QObject(p) {}
};

/** Page widget */
class U2GUI_EXPORT AppSettingsGUIPageWidget: public QWidget {
    Q_OBJECT
public:
    AppSettingsGUIPageWidget(QWidget* w = NULL) : QWidget(w){}

    //reads data from 'state' and initializes the widget state
    virtual void setState(AppSettingsGUIPageState* state) = 0;

    //returns 'state' of the widget or error if state is not complete
    // the result structure is automatically deleted by controller
    virtual AppSettingsGUIPageState* getState(QString& errMsg) const = 0;
};

/** Page factory/controller */
class U2GUI_EXPORT AppSettingsGUIPageController: public QObject {
    Q_OBJECT
public:
    AppSettingsGUIPageController(const QString& s, const QString& _id, QObject* p = NULL) : QObject(p), name(s), id(_id) {}

    const QString& getPageName() const {return name;}

    const QString& getPageId() const {return id;}

    virtual AppSettingsGUIPageState* getSavedState() = 0;

    virtual void saveState(AppSettingsGUIPageState* s) = 0;

    //creates widget and initializes its values with 'data' content
    virtual AppSettingsGUIPageWidget* createWidget(AppSettingsGUIPageState* data) = 0;

private:
    QString name;
    QString id;
};

#define APP_SETTINGS_GUI_NETWORK    "network"
#define APP_SETTINGS_GUI_LOG        "log"
#define APP_SETTINGS_USER_APPS      "user_apps"
#define APP_SETTINGS_RESOURCES      "resources"

}//namespace

#endif
