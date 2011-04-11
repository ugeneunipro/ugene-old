#ifndef __BACKGROUND_RENDERER_H__
#define __BACKGROUND_RENDERER_H__

#include <QtCore/QSharedPointer>
#include <QtGui/QPixmap>

#include <U2Core/Task.h>

#include "AssemblyBrowserSettings.h"

namespace U2 {

class AssemblyModel;

class BackgroundRenderTask : public Task {
    Q_OBJECT
public:
    inline QImage getResult() const {return result;};
protected:
    BackgroundRenderTask(const QString& _name, TaskFlags f);
    QImage result;
};

class BackgroundRenderer: public QObject {
    Q_OBJECT
public:
    BackgroundRenderer();
    void render(BackgroundRenderTask * task);
    QImage getImage() const;
signals:
    void si_rendered();
    private slots:
    void sl_redrawFinished();
    
private:
    BackgroundRenderTask * renderTask;
    QImage result;
    QSize size;
    bool redrawRunning;
    bool redrawNeeded;
};


}

#endif
