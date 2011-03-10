#ifndef _U2_CSV_COLUMN_CONFIGURATION_DIALOG_H_
#define _U2_CSV_COLUMN_CONFIGURATION_DIALOG_H_

#include <ui/ui_CSVColumnConfigurationDialog.h>
#include "CSVColumnConfiguration.h"

namespace U2 {

class CSVColumnConfigurationDialog : public QDialog, Ui_CSVColumnConfigurationDialog {
    Q_OBJECT
public:
    CSVColumnConfigurationDialog(QWidget* w, const ColumnConfig& config);

    ColumnConfig config;

public slots:
    virtual void accept();

private slots:
    virtual void sl_complMarkToggle(bool);
    virtual void sl_startToggle(bool);

};

} // namespace U2

#endif 
