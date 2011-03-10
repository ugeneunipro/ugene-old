#pragma once

#include <ui/ui_TreeSettingsDialog.h>
#include <QtGui/QDialog>

namespace U2 {


class TreeSettings {
public:
    TreeSettings();

    QString type;
    int width_coef;
    int height_coef;

    static QString default_type;
    static int default_width_coef;
    static int default_height_coef;

    static const QString CLADO_TYPE;
    static const QString PHYLO_TYPE;

};

class TreeSettingsDialog : public QDialog, public Ui_TreeSettingsDialog{
    Q_OBJECT
public:
    TreeSettingsDialog(QWidget *parent, const TreeSettings &treeSettings, bool isRectLayout);
    
    virtual void accept();
    TreeSettings getSettings() const;

protected slots:
   // void sl_colorButton();

private:
    TreeSettings settings, changedSettings;

};

} //namespace


