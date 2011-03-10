#ifndef _U2_PHYLTREE_BUTTON_SETTINGS_DIALOG_H_
#define _U2_PHYLTREE_BUTTON_SETTINGS_DIALOG_H_

#include <ui/ui_ButtonSettingsDialog.h>
#include <QtGui/QDialog>

namespace U2 {

class ButtonSettings {
public:
    ButtonSettings();

    QColor col;
    int radius;

    static QColor defaultColor;
    static int defaultRadius;
};

class ButtonSettingsDialog : public QDialog, public Ui_ButtonSettingsDialog{
    Q_OBJECT
public:
    ButtonSettingsDialog(QWidget *parent, const ButtonSettings& buttonSettings);

    virtual void accept();
    ButtonSettings getSettings() const;

protected slots:
    void sl_colorButton();

private:
    ButtonSettings settings, changedSettings;

    void updateColorButton();
};

} //namespace

#endif
