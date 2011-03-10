#ifndef _U2_QUERY_EDITOR_H_
#define _U2_QUERY_EDITOR_H_

#include "ui_QueryEditorWidget.h"


namespace U2 {

class Descriptor;
class QueryProcCfgModel;
class QDParameters;
class QDActor;
class QDConstraint;
class QueryViewController;
class ProcessNameValidator;
class QDActorPrototype;

class QueryEditor : public QWidget, public Ui_QueryEditorWidget {
    Q_OBJECT
friend class ProcessNameValidator;
public:
    QueryEditor(QWidget* parent=0);
    void showProto(QDActorPrototype* proto);
    void edit(QDConstraint* constraint);
    void edit(QDActor* a);
    void reset();
    void setCurrentAttribute(const QString& id);
private slots:
    void sl_showPropDoc();
    void sl_setLabel();
    void sl_setKey();
    void sl_setDirection(int);
signals:
    void modified();
private:
    void setDescriptor(const Descriptor* d, const QString& hint = QString());

    QueryProcCfgModel* cfgModel;
    QDActor* current;
};

class ProcessNameValidator : public QValidator {
    Q_OBJECT
public:
    ProcessNameValidator(QObject* parent, QueryViewController* view)
        : QValidator(parent), view(view) {}
    QValidator::State validate(QString& input, int& pos) const;
private:
    QueryViewController* view;
};

}//namespace

#endif
