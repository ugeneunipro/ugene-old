#ifndef _U2_PHYLTREE_TEXT_SETTINGS_DIALOG_H_
#define _U2_PHYLTREE_TEXT_SETTINGS_DIALOG_H_

#include <ui/ui_TextSettingsDialog.h>
#include <QtGui/QFontInfo>
#include <QtGui/QDialog>

namespace U2 {

class TextSettings {
public:
    TextSettings();

    QColor textColor;
    QFont textFont;

    static QColor defaultColor;
    static QFont defaultFont;
};

class TextSettingsDialog : public QDialog, public Ui_TextSettingsDialog{
    Q_OBJECT
public:
    TextSettingsDialog(QWidget *parent, const TextSettings &textSettings);

    virtual void accept();
    TextSettings getSettings() const;

protected slots:
    void sl_colorButton();

private:
    TextSettings settings, changedSettings;

    void updateColorButton();
};

} //namespace

#endif
