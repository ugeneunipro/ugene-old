#ifndef _U2_CREATE_OBJECT_RELATION_DIALOG_CONTROLLER_H_
#define _U2_CREATE_OBJECT_RELATION_DIALOG_CONTROLLER_H_

#include <U2Core/global.h>

#include <QtGui/QDialog>

class Ui_CreateObjectRelationDialog;

namespace U2 {

class GObject;

class U2GUI_EXPORT CreateObjectRelationDialogController : public QDialog {
    Q_OBJECT
public:
    CreateObjectRelationDialogController(GObject* obj, const QList<GObject*>& objects, 
                                        const QString& role, bool removeDuplicates, const QString& relationHint, QWidget* p = NULL);
    ~CreateObjectRelationDialogController();

    GObject*        selectedObject;
    GObject*        assObj;
    QList<GObject*> objects;
    QString         role;
    bool            removeDuplicates;

public slots:
    virtual void accept();

private:
    Ui_CreateObjectRelationDialog* ui;
};

} //namespace

#endif
