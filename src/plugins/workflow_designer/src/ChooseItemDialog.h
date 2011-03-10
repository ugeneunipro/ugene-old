#ifndef _U2_CHOOSE_DIALOG_CONTROLLER_H_
#define _U2_CHOOSE_DIALOG_CONTROLLER_H_

#include <U2Core/global.h>

#include <ui/ui_ChooseItemDialog.h>
#include <U2Lang/ActorModel.h>

namespace U2 {

    class ChooseItemDialog : public QDialog,  public Ui_ChooseItemDialog {
        Q_OBJECT
    public:
        ChooseItemDialog(QWidget* p = NULL);
        //virtual int select(const QList<QString>& items);
        virtual QString select(const QMap<QString,QIcon>& items);
        Workflow::ActorPrototype* select(const QList<Workflow::ActorPrototype*>& items);
    };

} //namespace

#endif
