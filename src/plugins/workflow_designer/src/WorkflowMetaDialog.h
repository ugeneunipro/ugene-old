#ifndef _U2_WMETADIALOG_H_
#define _U2_WMETADIALOG_H_

#include "ui/ui_WorkflowMetaDialog.h"

#include <U2Lang/ActorModel.h>
#include <U2Lang/Schema.h>

namespace U2 {
using namespace Workflow;
class WorkflowMetaDialog : public QDialog, public Ui_WorkflowMetaDialog {
    Q_OBJECT

public:
    WorkflowMetaDialog(const Workflow::Metadata& meta);

public:
    Workflow::Metadata meta;

private slots:
   
    void sl_onBrowse();
    void sl_onSave();
    void sl_onURLChanged(const QString & text);
};

}//namespace

#endif
