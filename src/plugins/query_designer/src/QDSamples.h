#ifndef _U2_QD_SAMPLES_H_
#define _U2_QD_SAMPLES_H_

#include <U2Core/Task.h>

#include <U2Lang/Descriptor.h>

#include <U2Misc/GlassView.h>

#include <QtGui/QListWidget>


namespace U2 {

class QDDocument;
class QueryScene;

class QDSample {
public:
    Descriptor d;
    QDDocument* content;
};

class QDLoadSamplesTask : public Task {
    Q_OBJECT
public:
    QDLoadSamplesTask(const QStringList& _dirs);
    virtual QList<Task*> onSubTaskFinished(Task* subTask);
    ReportResult report();
private:
    QMap<Task*, QString> idMap;
    QList<QDSample> result;
};

class QDSamplesRegistry {
public:
    static const QList<QDSample>& getSamples() { return data; }
private:
    static QList<QDSample> data;
    friend class QDLoadSamplesTask;
};

class QDSamplePane : public GlassPane {
    Q_OBJECT
public:
    QDSamplePane(QueryScene* _scene);
    virtual void paint(QPainter* painter);
    virtual bool eventFilter(QObject*, QEvent* e) {
        this->event(e);
        return current != NULL;
    }
    void setItem(QListWidgetItem* item) { current=item; }
signals:
    void itemActivated(QListWidgetItem* item);
protected:
    void mouseDoubleClickEvent(QMouseEvent* e);
private:
    QueryScene* scene;
    QListWidgetItem* current;
};
    
class QDSamplesWidget : public QListWidget {
    Q_OBJECT
public:
    QDSamplesWidget(QueryScene* scene, QWidget* parent=NULL);
signals:
    void itemActivated(QDDocument*);
    void setupGlass(GlassPane* glass);
private:
    void addSample(const QDSample& sample);
private slots:
    void sl_onItemChanged(QListWidgetItem* item);
    void sl_onItemSelected(QListWidgetItem* item);
    void sl_cancel();
private:
    QDSamplePane* glass;

};

}//namespace

#endif
