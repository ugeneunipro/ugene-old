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


#ifndef _U2_UINDEX_VIEWER_H_
#define _U2_UINDEX_VIEWER_H_

#include <U2Gui/ObjectViewModel.h>
#include <U2Core/UIndexObject.h>

#include "UIndexViewWidgetImpl.h"

namespace U2 {

class U2VIEW_EXPORT UIndexViewer : public GObjectView {
    Q_OBJECT
public:
    UIndexViewer( const QString& viewName, UIndexObject* obj );
    
    virtual void buildStaticToolbar(QToolBar* tb);
    
    virtual void buildStaticMenu(QMenu* n);
    
    virtual Task* updateViewTask(const QVariantMap& stateData);
    
    UIndexObject* getUindexObject() const { return indexObject; }
    
    const UIndexViewWidgetImpl* getUI() const { return ui; }
    
protected slots:
    void sl_onContextMenuRequested( const QPoint& pos );
    
protected:
    virtual QWidget* createWidget();
    
private:
    static const QString EXPORT_MENU_NAME;
    
    void addExportMenu( QMenu* m );
    
    UIndexObject* indexObject;
    UIndexViewWidgetImpl* ui;

private slots:
    void sl_exportToNewDoc();
    
}; // UIndexViewer

} // U2

#endif // _U2_UINDEX_VIEWER_H_
