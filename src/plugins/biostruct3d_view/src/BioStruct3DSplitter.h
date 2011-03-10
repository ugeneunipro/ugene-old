#include <U2Core/Task.h>
#include <U2View/ADVSplitWidget.h>

#include <QtGui/QSplitter>
#include <QtGui/QToolBar>
#include <QtGui/QToolButton>
#include <QtGui/QComboBox>
#include <QtGui/QAction>
#include <memory>

namespace U2 {

class BioStruct3DObject;
class BioStruct3DGLWidget;
class AnnotatedDNAView;
class GObject;
class GObjectView;
class SplitterHeaderWidget;
class Document;
class GLFrameManager;

/*!
* @class BioStruct3DSplitter BioStruct3DSplitter.h
* @brief Class for multiple BioStruct3DWidget layout.
*
* This widget provides insertion of BioStruct3DGLWidgets into active UGENE view. There
* could be one or more glWidgets controlled by the splitter. One can add new glWidgets using 
* the splitter.
*/ 
class BioStruct3DSplitter : public ADVSplitWidget {

    Q_OBJECT
    
public:
    /*!
    * Constructor.
    * @param closeAction QAction provided for correct widget closing
    * @param view DnaView context for connecting structure 3D representation and sequence view
    */ 
    BioStruct3DSplitter(QAction* closeAction, AnnotatedDNAView* view);
    /*!
    * Destructor.
    */ 
    ~BioStruct3DSplitter();
    /*!
    * @return True, if GObject has type BioStruct3D and could be added to splitter.
    */ 
    virtual bool acceptsGObject(GObject* obj);
    /*!
    * Creates BioStruct3DWidget for object visualization and adds it to the splitter.
    */ 
    BioStruct3DGLWidget* addBioStruct3DGLWidget(BioStruct3DObject* obj);
    /*!
    * @return First found BioStruct3DObject with corresponding name, NULL if nothing is found.
    */ 
    BioStruct3DObject* findBioStruct3DObjByName(const QString& objName);
    virtual void saveState(QVariantMap& m);
    virtual void updateState(const QVariantMap& m);
    /*!
    * @return List of splitter children widgets.
    */ 
    QList<BioStruct3DGLWidget*> getChildWidgets();
    /*!
    * @return Current active view.
    */ 
    BioStruct3DGLWidget* getActiveWidget();
    const QList<QAction* > getSettingsMenuActions() const;
    /*!
    * @return ADVDNAView splitter, parent widget for BioStruct3DSplitter.
    */ 
    QSplitter* getParentSplitter();
    /*!
    * @return Number of visible splitter child widgets.
    */
    int getNumVisibleWidgets();
    /*!
    * @return If number of visible children is null, sets splitter view collapsed, else restores it.
    */
    void adaptSize(int numVisibleWidgets);
    /*!
    * @return GLFrameManager for splitter child widgets.
    */
    GLFrameManager* getGLFrameManager();
    QAction* getCloseSplitterAction() { return closeAction; }
    /*!
    * QWidget virtual function, returns preferred widget size.
    */
    virtual QSize sizeHint () const {return QSize(0, 400);}
    /*!
    * Adds object and its new view.
    */
    void addObject(BioStruct3DObject* obj);
    /*!
    * AddModelTask helper function.
    */
    void addModelFromObject(BioStruct3DObject* obj);
    /*!
    * Removes object and its views.
    */
    bool removeObject(BioStruct3DObject* obj);

signals:
    void si_bioStruct3DGLWidgetAdded(BioStruct3DGLWidget* w);

private slots:
    //void sl_toggleBioStruct3DWidget(bool show);
    //void sl_closeBioStruct3DGlWidget(BioStruct3DGLWidget* glWidget);
    
protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    bool eventFilter(QObject* o, QEvent* e);

private:
    QMultiMap<BioStruct3DObject*, BioStruct3DGLWidget*> biostrucViewMap;
    std::auto_ptr<GLFrameManager> glFrameManager;
    QSplitter*  splitter;
    QSplitter*  parentSplitter;
    QAction*    closeAction;
    int         splitterHeight;
    bool        isViewCollapsed;
    
    SplitterHeaderWidget* header;
};


struct DBLink {
    DBLink(const QString& _name, QString _url) : name(_name), url(_url) { }
    QString name;
    QString url;
};

class DBLinksFile {
    QList<DBLink> links;
public:
    bool load();
    QList<DBLink> getLinks() { return links; };
};


// Implemented as Task for unloaded documents support
class AddModelToSplitterTask : public Task {
    Q_OBJECT
public:
    AddModelToSplitterTask(GObject* o, BioStruct3DSplitter* s);
    virtual void prepare();
    virtual void run();
    virtual ReportResult report();
private:
    Document* doc;
    GObject* obj;
    BioStruct3DObject* bObj;
    BioStruct3DSplitter* splitter;
};


class SplitterHeaderWidget : public QWidget {
    Q_OBJECT

public:
    SplitterHeaderWidget(BioStruct3DSplitter* splitter);
    QMap<BioStruct3DGLWidget*, QAction*> showWidgetActionMap;
    void updateToolbar();
    void enableToolbar();
    void updateWidgetBox();

public:    
    BioStruct3DSplitter* splitter;
    QToolButton* widgetStateMenuButton;
    QToolButton* addModelButton;
    QToolButton* settingsMenuButton;
    QToolButton* webMenuButton;
    QToolButton* displayMenuButton;
    QToolButton* restoreDefaultsButton;
    QToolButton* zoomInButton;
    QToolButton* zoomOutButton;
    QToolButton* syncLockButton;
    QComboBox* activeWidgetBox;
    QList<BioStruct3DGLWidget*> widgets;
    QMap<QAction*,QString> webActionMap;

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    
    void sl_addBioStruct3DGLWidget(BioStruct3DGLWidget* glWidget);
    void sl_toggleBioStruct3DWidget(bool visible);
    void sl_closeWidget3DGLWidget(BioStruct3DGLWidget* glWidget);
    void sl_toggleSyncLock(bool on);
    void sl_addModel();
    void sl_showStateMenu();
    void sl_showDisplayMenu();
    void sl_showSettingsMenu();
    void sl_showWebMenu();
    void sl_zoomIn();
    void sl_zoomOut();
    void sl_restoreDefaults();
    void sl_openBioStructUrl();
    

private:
    BioStruct3DGLWidget* getActiveWidget();
    void registerWebUrls();
    void setActiveView(BioStruct3DGLWidget* glWidget);

};





} //namespace
