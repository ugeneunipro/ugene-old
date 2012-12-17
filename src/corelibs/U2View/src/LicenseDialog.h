#ifndef LICENSEDIALOG_H
#define LICENSEDIALOG_H

#include <U2Core/GUrl.h>
#include <U2Core/PluginModel.h>

#include <QtGui/QDialog>
#include <ui/ui_LicenseDialog.h>

namespace U2 {

class LicenseDialog : public QDialog, private Ui::LicenseDialog
{
    Q_OBJECT

public:
    explicit LicenseDialog(Plugin * plugin, QWidget *parent = 0);
    ~LicenseDialog();
public slots:
    void sl_accept();

private:
    Ui::LicenseDialog   *ui;
    Plugin              *plugin;

};
}//namespace
#endif // LICENSEDIALOG_H
