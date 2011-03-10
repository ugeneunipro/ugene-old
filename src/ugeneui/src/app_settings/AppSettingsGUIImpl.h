#ifndef _U2_APPSETTINGS_GUI_IMPL_H_
#define _U2_APPSETTINGS_GUI_IMPL_H_

#include <U2Gui/AppSettingsGUI.h>

namespace U2 {

class AppSettingsGUIImpl : public AppSettingsGUI {
    Q_OBJECT
public:
    AppSettingsGUIImpl(QObject* p = NULL);

    ~AppSettingsGUIImpl();
    
    virtual bool registerPage(AppSettingsGUIPageController* page, const QString& beforePage = QString());

    virtual bool unregisterPage(AppSettingsGUIPageController* page);

    virtual void showSettingsDialog(const QString& pageId = QString()) const;

    AppSettingsGUIPageController* findPageById(const QString& pageId) const;
    
    virtual QList<AppSettingsGUIPageController*> getRegisteredPages() const  {return pages;}

private slots:
    void sl_showSettingsDialog() {showSettingsDialog();}

private:
    void registerBuiltinPages();

    QList<AppSettingsGUIPageController*>      pages;

};

}//namespace

#endif
