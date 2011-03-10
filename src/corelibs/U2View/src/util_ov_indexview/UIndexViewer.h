
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
