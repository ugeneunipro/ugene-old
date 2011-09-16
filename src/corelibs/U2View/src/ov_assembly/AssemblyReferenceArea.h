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

#ifndef __ASSEMBLY_REFERENCE_AREA_H__
#define __ASSEMBLY_REFERENCE_AREA_H__

#include <QtGui/QWidget>
#include <QtGui/QMenu>
#include <QtCore/QSharedPointer>

#include "AssemblyCellRenderer.h"
#include "AssemblyModel.h"

#include <memory>
using std::auto_ptr;

namespace U2 {

class AssemblyBrowserUi;
class AssemblyBrowser;

class AssemblyReferenceArea: public QWidget {
    Q_OBJECT
public:
    AssemblyReferenceArea(AssemblyBrowserUi * ui);

protected:
    void paintEvent(QPaintEvent * e);
    void resizeEvent(QResizeEvent * e);
    void mouseMoveEvent(QMouseEvent * e);
    void mousePressEvent(QMouseEvent* e);
    
signals:
    void si_mouseMovedToPos(const QPoint &);
    void si_unassociateReference();
    
private slots:
    void sl_redraw();

private:
    void connectSlots();
    void drawAll();
    void drawReference(QPainter & p);

private:
    AssemblyBrowserUi * ui;
    AssemblyBrowser * browser;
    QSharedPointer<AssemblyModel> model;

    QPixmap cachedView;
    bool redraw;
    auto_ptr<AssemblyCellRenderer> cellRenderer;

    const static int FIXED_HEIGHT = 25;

    QMenu * referenceAreaMenu;
};

} //ns

#endif 
