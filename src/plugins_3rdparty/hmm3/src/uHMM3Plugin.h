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
