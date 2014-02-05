/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#ifndef _U2_CLUSTALW_SUPPORT_H
#define _U2_CLUSTALW_SUPPORT_H

#include <U2Core/ExternalToolRegistry.h>
#include <U2View/MSAEditor.h>
#include "utils/ExternalToolSupportAction.h"

#define ET_CLUSTAL "ClustalW"
#define CLUSTAL_TMP_DIR "clustal"

namespace U2 {

class ClustalWSupport : public ExternalTool {
    Q_OBJECT
public:
    ClustalWSupport(const QString& name, const QString& path = "");
    GObjectViewWindowContext* getViewContext(){ return viewCtx; }

public slots:
    void sl_runWithExtFileSpecify();
private:
    GObjectViewWindowContext* viewCtx;
};

class ClustalWSupportContext: public GObjectViewWindowContext {
    Q_OBJECT
public:
    ClustalWSupportContext(QObject* p);

protected slots:
    void sl_align_with_ClustalW ();

protected:
    virtual void initViewContext(GObjectView* view);
    virtual void buildMenu(GObjectView* view, QMenu* m);
};

class ClustalWSupportAction : public ExternalToolSupportAction {
        Q_OBJECT
public:
    ClustalWSupportAction(QObject* p, GObjectView* v, const QString& text, int order)
        : ExternalToolSupportAction(p,v,text,order, QStringList(ET_CLUSTAL)) {}
    MSAEditor*  getMSAEditor() const;

private slots:
    void sl_lockedStateChanged();
};

}//namespace
#endif // _U2_CLUSTALW_SUPPORT_H
