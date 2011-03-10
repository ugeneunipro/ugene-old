#ifndef _U2_ENZYMES_QUERY_H_
#define _U2_ENZYMES_QUERY_H_

#include "EnzymeModel.h"
#include "FindEnzymesDialog.h"
#include <ui/ui_EnzymesSelectorDialog.h>

#include <U2Lang/QDScheme.h>
#include <U2Lang/QueryDesignerRegistry.h>

#include <U2Designer/DelegateEditors.h>

#include <U2Core/Task.h>


namespace U2 {

class FindEnzymesTask;
class EnzymesSelectorDialogHandler;
class QDEnzymesActor : public QDActor {
    Q_OBJECT
public:
    QDEnzymesActor(QDActorPrototype const* proto);
    int getMinResultLen() const { return 1; }
    int getMaxResultLen() const { return 20; }
    QString getText() const;
    Task* getAlgorithmTask(const QVector<U2Region>& location);
    QColor defaultColor() const { return QColor(0xB4, 0x9F, 0xD4); }
    virtual bool hasStrand() const { return false; }
private slots:
    void sl_onAlgorithmTaskFinished();
private:
    QStringList ids;
    QList<FindEnzymesTask*> enzymesTasks;
    EnzymesSelectorDialogHandler* selectorFactory;
};

class QDEnzymesActorPrototype : public QDActorPrototype {
public:
    QDEnzymesActorPrototype();
    QIcon getIcon() const { return QIcon(":enzymes/images/enzymes.png"); }
    QDActor* createInstance() const { return new QDEnzymesActor(this); }
};

class EnzymesSelectorDialog : public QDialog, public Ui_EnzymesSelectorDialog {
    Q_OBJECT
public:
    EnzymesSelectorDialog(EnzymesSelectorDialogHandler* parent);
    QString getSelectedString() const;
private:
    EnzymesSelectorDialogHandler* factory;
    EnzymesSelectorWidget* enzSel;
};

class EnzymesSelectorDialogHandler : public SelectorDialogHandler {
public:
    EnzymesSelectorDialogHandler() {}
    virtual QDialog* createSelectorDialog(const QString& init = QString()) { return new EnzymesSelectorDialog(this); }
    virtual QString getSelectedString(QDialog* dlg);
};

}//namespace

#endif
