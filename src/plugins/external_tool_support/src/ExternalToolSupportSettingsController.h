#ifndef _U2_EXTERNAL_TOOL_SUPPORT_SETTINGS_CONTROLLER_H
#define _U2_EXTERNAL_TOOL_SUPPORT_SETTINGS_CONTROLLER_H

#include <U2Gui/AppSettingsGUI.h>
#include <U2Core/ExternalToolRegistry.h>
#include <ui/ui_ETSSettingsWidget.h>

#include <QtGui/QLineEdit>

namespace U2
{
#define ExternalToolSupportSettingsPageId QString("ets")
struct ExternalToolInfo{
    QString name;
    QString path;
    QString description;
    QString version;
    bool    valid;
};

class ExternalToolSupportSettingsPageController : public AppSettingsGUIPageController {
    Q_OBJECT
public:
    ExternalToolSupportSettingsPageController(QObject* p = NULL);

    AppSettingsGUIPageState* getSavedState();
    void saveState(AppSettingsGUIPageState* s);
    AppSettingsGUIPageWidget* createWidget(AppSettingsGUIPageState* state);

};

class ExternalToolSupportSettingsPageState : public AppSettingsGUIPageState {
    Q_OBJECT
public:
    QList<ExternalTool*>    externalTools;
};

class ExternalToolSupportSettingsPageWidget: public AppSettingsGUIPageWidget, public Ui_ETSSettingsWidget {
    Q_OBJECT
public:
    ExternalToolSupportSettingsPageWidget(ExternalToolSupportSettingsPageController* ctrl);

    virtual void setState(AppSettingsGUIPageState* state);

    virtual AppSettingsGUIPageState* getState(QString& err) const;
private:
    QWidget* createPathEditor(QWidget *parent, const QString& path) const;
    void insertChild(QTreeWidgetItem* rootItem, QString name, int pos);
private slots:
    void sl_toolPathCanged();//QString path);
    void sl_validateTaskStateChanged();
    void sl_itemSelectionChanged();
    void sl_onPathEditWidgetClick();
    void sl_onBrowseToolKitPath();
private:
    QMap<QString, ExternalToolInfo> externalToolsInfo;
    mutable int buttonsWidth;
};

class PathLineEdit : public QLineEdit {
    Q_OBJECT
public:
    PathLineEdit(const QString& filter, const QString& type, bool multi, QWidget *parent)
        : QLineEdit(parent), FileFilter(filter), type(type), multi(multi) {}

private slots:
    void sl_onBrowse();
    void sl_clear();

private:
    QString FileFilter;
    QString type;
    bool    multi;
    QString path;
};
}//namespace

#endif // _U2_EXTERNAL_TOOL_SUPPORT_SETTINGS_CONTROLLER_H
