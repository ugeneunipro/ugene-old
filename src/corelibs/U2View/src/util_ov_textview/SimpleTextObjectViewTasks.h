#ifndef _U2_SIMPLE_TEXT_OBJECT_VIEW_TASKS_H_
#define _U2_SIMPLE_TEXT_OBJECT_VIEW_TASKS_H_

#include <U2Gui/ObjectViewTasks.h>

namespace U2 {

class OpenSimpleTextObjectViewTask : public ObjectViewTask {
    Q_OBJECT
public:
    OpenSimpleTextObjectViewTask(Document* doc);
    virtual void open();

private:
    QPointer<Document> doc;
};

class OpenSavedTextObjectViewTask : public ObjectViewTask {
    Q_OBJECT
public:
    OpenSavedTextObjectViewTask(const QString& viewName, const QVariantMap& stateData);
    virtual void open();

private:
    QPointer<Document> doc;
};

class UpdateSimpleTextObjectViewTask : public ObjectViewTask {
public:
    UpdateSimpleTextObjectViewTask(GObjectView* v, const QString& stateName, const QVariantMap& stateData);

    virtual void update();
};


} // namespace

#endif

