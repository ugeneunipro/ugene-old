/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

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
    ~BackgroundRenderer();

    void render(BackgroundRenderTask * task);
    QImage getImage() const;
signals:
    void si_rendered();
    private slots:
    void sl_redrawFinished();
    
private:
    BackgroundRenderTask * renderTask;
    QImage result;
};


}

#endif
