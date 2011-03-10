#ifndef _U2_WCFG_DIALOG_CONTROLLER_H_
#define _U2_WCFG_DIALOG_CONTROLLER_H_

#include <ui/ui_SchemaConfigurationDialog.h>
#include <U2Lang/ActorModel.h>
#include <U2Lang/Schema.h>

namespace U2 {
class IterationListWidget;
using namespace Workflow;

class SchemaConfigurationDialog : public QDialog,  public Ui_SchemaConfigurationDialog {
    Q_OBJECT
public:
    SchemaConfigurationDialog(const Schema& schema, 
        const QList<Iteration> &lst, QWidget* p = NULL);

    QList<Iteration>& getIterations() const;
    bool hasModifications() const {return modified;}

private slots:
    void finishPropertyEditing();
    void commit();
    void setModified() {modified = true;}

private:
    IterationListWidget* iterationList;
    bool modified;
};

} //namespace

#endif
