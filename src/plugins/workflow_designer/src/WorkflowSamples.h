#ifndef _U2_WORKFLOW_SAMPLES_H_
#define _U2_WORKFLOW_SAMPLES_H_

#include <U2Lang/Descriptor.h>
#include <U2Misc/GlassView.h>
#include <U2Core/Task.h>

#include <QtGui/QAction>
#include <QtGui/QToolBox>
#include <QtGui/QButtonGroup>
#include <QtGui/QTreeWidget>

#include <QtXml/qdom.h>

namespace U2 {
class WorkflowView;
class WorkflowScene;

class Sample {
public:
    Descriptor d;
    QIcon ico;
    QString data;
    QString content;
};

class SampleCategory {
public:
    SampleCategory(const QString& dir, const QString& name) : d(dir, name, ""){}
    Descriptor d;
    QList<Sample> items;
};

class LoadSamplesTask : public Task {
    Q_OBJECT
public:
    LoadSamplesTask(const QStringList&);
    void run();
    ReportResult report();
private:
    void scanDir(const QString&);
    QStringList dirs;
    QList<SampleCategory> result;
};

class SampleRegistry {
public:
    static Task* init(const QStringList&);
    static QList<SampleCategory> getCategories() {return data;}
private:
    static QList<SampleCategory> data;
    friend class LoadSamplesTask;
};

class SamplePane : public GlassPane {
    Q_OBJECT
public:
    SamplePane(WorkflowScene *scene);
    virtual void paint(QPainter*);
    virtual bool eventFilter(QObject*, QEvent* e) {
        this->event(e);
        return item != NULL;
    }

public slots:
    void test();
    void setItem(QTreeWidgetItem*);
signals:
    void itemActivated(QTreeWidgetItem * item);
    void cancel();
protected:
    virtual void mouseDoubleClickEvent ( QMouseEvent * event );
    virtual void keyPressEvent ( QKeyEvent * event );
    //    virtual void mousePressEvent ( QMouseEvent * event );
    //virtual void mouseReleaseEvent ( QMouseEvent * event );
private:
    QTextDocument*      m_document;
    QTreeWidgetItem*    item;
    WorkflowScene *scene;
};


class SamplesWidget : public QTreeWidget {
    Q_OBJECT
public:
    static const QString MIME_TYPE;

    SamplesWidget(WorkflowScene *scene, QWidget *parent = 0);
//     QList<Descriptor> getCategories() const;
//     QList<Descriptor> getItems(const Descriptor& cat) const;
//     QMenu* createMenu(const QString& name);
// 
//     QVariant saveState() const;
//     void restoreState(const QVariant&);
// 
//     public slots:
//         void resetSelection();
// 
// signals:
//         void processSelected(Workflow::ActorPrototype*);
// 
// protected:
//     void contextMenuEvent(QContextMenuEvent *e);
public slots:
    void cancelItem();
protected:
    void resizeEvent(QResizeEvent *e) {
        QTreeWidget::resizeEvent(e);
        doItemsLayout();
    }

private slots:
    void handleTreeItem(QTreeWidgetItem * item);
    void activateItem(QTreeWidgetItem * item);
    
signals:
    void setupGlass(GlassPane*);
    void sampleSelected(const QString&);
private:
    void addCategory(const SampleCategory& cat);

    SamplePane* glass;
};

}//namespace

//Q_DECLARE_METATYPE(QAction *)

#endif
