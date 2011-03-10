#ifndef _U2_LOG_SETTINGS_WIDGET_IMPL_H_
#define _U2_LOG_SETTINGS_WIDGET_IMPL_H_

#include <ui/ui_LogSettingsWidget.h>

#include <LogSettings.h>
#include <U2Gui/AppSettingsGUI.h>

#include <QtGui/QTableWidgetItem>
#include <QtGui/QLabel>
#include <QtCore/QUrl>

namespace U2 {

class LogSettingsPageController : public AppSettingsGUIPageController {
    Q_OBJECT
public:
    LogSettingsPageController(LogSettingsHolder* target, QObject* p = NULL);

    virtual AppSettingsGUIPageState* getSavedState();

    virtual void saveState(AppSettingsGUIPageState* s);

    virtual AppSettingsGUIPageWidget* createWidget(AppSettingsGUIPageState* data);

private:
    LogSettingsHolder* target;
};


class LogSettingsPageState : public AppSettingsGUIPageState {
    Q_OBJECT
public:
    LogSettingsPageState();
    LogSettings settings;
};


class LogSettingsPageWidget: public AppSettingsGUIPageWidget, public Ui_LogSettingsWidget {
    Q_OBJECT
public:
    LogSettingsPageWidget();

    virtual void setState(AppSettingsGUIPageState* state);

    virtual AppSettingsGUIPageState* getState(QString& err) const;

private slots:
    void sl_currentCellChanged ( int currentRow, int currentColumn, int previousRow, int previousColumn);
    void sl_levelStateChanged(int state);
    void sl_catItemStateChanged(QTableWidgetItem *item);
    void sl_changeColor(const QString& v);

private:
    void updateColorLabel(QLabel* l, const QString& color);
};

class LogSettingsTopLineWidget : public QWidget {
    Q_OBJECT
public:
    LogSettingsTopLineWidget(QWidget* parent, const QString& _color, LogLevel _level) 
        : QWidget(parent), color(_color), level(_level), cb(NULL){}
    QString     color;
    LogLevel    level;
    QCheckBox*  cb;
};

}//namespace

#endif
