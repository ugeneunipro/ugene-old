#ifndef _GB2_UHMMER3_PLUGIN_H_
#define _GB2_UHMMER3_PLUGIN_H_

#include <U2Core/PluginModel.h>
#include <U2Core/AppContext.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Gui/ObjectViewModel.h>

namespace U2 {

class UHMM3MSAEditorContext;
class UHMM3ADVContext;

class UHMM3Plugin : public Plugin {
    Q_OBJECT
public:
    UHMM3Plugin();
    ~UHMM3Plugin();
    
private:
    DNASequenceObject * getDnaSequenceObject() const;
    
private slots:
    void sl_buildProfile();
    void sl_searchHMMSignals();
    void sl_phmmerSearch();

private:
    UHMM3MSAEditorContext *     msaEditorCtx;
    UHMM3ADVContext *           advCtx;
    
}; // uHMM3Plugin

class UHMM3MSAEditorContext : public GObjectViewWindowContext {
    Q_OBJECT
public:
    UHMM3MSAEditorContext( QObject * p );
    
protected slots:
    void sl_build();
    
protected:
    virtual void initViewContext( GObjectView * view );
    virtual void buildMenu( GObjectView * v, QMenu * m );
    
}; // UHMM3MSAEditorContext

class UHMM3ADVContext : public GObjectViewWindowContext {
    Q_OBJECT
public:
    UHMM3ADVContext( QObject * p );
    
protected slots:
    void sl_search();
    
private:
    QWidget * getParentWidget( QObject * sender );
    DNASequenceObject * getSequenceInFocus( QObject * sender );
    
protected:
    virtual void initViewContext( GObjectView * view );
    
}; // UHMM3ADVContext

} // U2

#endif // _GB2_UHMMER3_PLUGIN_H_
