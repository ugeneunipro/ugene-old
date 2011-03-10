#ifndef _U2_SIMPLE_TEXT_VIEW_H_
#define _U2_SIMPLE_TEXT_VIEW_H_

#include <U2Core/global.h>
#include <U2Gui/ObjectViewModel.h>
#include <U2Core/TextSelection.h>

#include <QtGui/QPlainTextEdit>

namespace U2 {

class OpenSimpleTextObjectViewTask;

class U2VIEW_EXPORT SimpleTextObjectViewFactory : public GObjectViewFactory {
    Q_OBJECT
public:
    SimpleTextObjectViewFactory(QObject* p = NULL);

    static const GObjectViewFactoryId ID;

    virtual bool canCreateView(const MultiGSelection& multiSelection);

    virtual bool isStateInSelection(const MultiGSelection& multiSelection, const QVariantMap& stateData);

    virtual Task* createViewTask(const MultiGSelection& multiSelection, bool single = false);

    virtual Task* createViewTask(const QString& viewName, const QVariantMap& state);

    virtual bool supportsSavedStates() const {return true;}
};

class U2VIEW_EXPORT SimpleTextObjectView: public GObjectView {
    Q_OBJECT

    friend class SimpleTextObjectViewFactory;
public:
    SimpleTextObjectView(const QString& name, TextObject* to, const QVariantMap& state);

    virtual QVariantMap saveState();

    virtual Task* updateViewTask(const QString& stateName, const QVariantMap& stateData);

    virtual bool checkAddToView(const MultiGSelection& multiSelection);

    virtual Task* addToViewTask(const MultiGSelection& multiSelection);

    virtual const TextSelection& getSelectedText() {return selection;}

    virtual QWidget* createWidget();


    void updateView(const QVariantMap& stateData);

    // saved state accessors -> todo: extract into separate model class
    static QString getDocumentUrl(const QVariantMap& savedState);
    static QString getObjectName(const QVariantMap& savedState);

    static void setDocumentUrl(QVariantMap& savedState, const QString& url);

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void sl_onTextEditTextChanged();
    void sl_onTextObjStateLockChanged();
private:
    TextObject*     textObject;
    QVariantMap     openState;
    TextSelection   selection;
    QPlainTextEdit* textEdit;
    bool            firstShow;
};


}//namespace

#endif
