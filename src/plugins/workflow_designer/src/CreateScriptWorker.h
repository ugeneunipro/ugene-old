#ifndef _U2_CREATE_SCRIPT_WORKER_DIALOG_H_
#define _U2_CREATE_SCRIPT_WORKER_DIALOG_H_

#include "ui/ui_CreateScriptBlockDialog.h"
#include <U2Lang/ActorModel.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Designer/DelegateEditors.h>
#include <U2Lang/BaseTypes.h>

#include <U2Core/AppContext.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/Settings.h>

#include <QtXml/qdom.h>

namespace U2 {
using namespace Workflow;

class CreateScriptElementDialog : public QDialog,  public Ui_CreateScriptBlockDialog {
    Q_OBJECT
public:
    //CreateScriptElementDialog(QWidget* p = NULL);
    CreateScriptElementDialog(QWidget* p = NULL, ActorPrototype* proto = NULL);
    QList<DataTypePtr > getInput() const;
    QList<DataTypePtr > getOutput() const;
    QList<Attribute*> getAttributes() const;
    const QString getName() const;
    const QString getDescription() const;

private slots:
    void sl_addInputClicked();
    void sl_addOutputClicked();
    void sl_okClicked();
    void sl_cancelClicked();
    void sl_addAttribute();
    void sl_deleteInputClicked();
    void sl_deleteOutputClicked();
    void sl_deleteAttributeClicked();
    void sl_getDirectory();

private:
    QDomDocument saveXml();
    bool saveParams();
    void fillFields(ActorPrototype *proto);
    void changeDirectoryForActors();

    QList<DataTypePtr > input;
    QList<DataTypePtr > output;
    QList<Attribute*> attrs;
    QString name;
    QString description;
    bool editing;
};

} //namespace

#endif
