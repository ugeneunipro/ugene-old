/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#ifndef _U2_CLUSTALO_SUPPORT_H
#define _U2_CLUSTALO_SUPPORT_H

#include <U2Core/ExternalToolRegistry.h>
#include <U2View/MSAEditor.h>
#include "utils/ExternalToolSupportAction.h"

#define ET_CLUSTALO "ClustalO"
#define CLUSTALO_TMP_DIR "clustalo"

namespace U2 {

class ClustalOSupport : public ExternalTool {
    Q_OBJECT
public:
    ClustalOSupport(const QString& name, const QString& path = "");
    GObjectViewWindowContext* getViewContext(){ return viewCtx; }

public slots:
    void sl_runWithExtFileSpecify();
private:
    GObjectViewWindowContext* viewCtx;
};

class ClustalOSupportContext: public GObjectViewWindowContext {
    Q_OBJECT
public:
    ClustalOSupportContext(QObject* p);

protected slots:
    void sl_align_with_ClustalO ();

protected:
    virtual void initViewContext(GObjectView* view);
    virtual void buildMenu(GObjectView* view, QMenu* m);
};

class ClustalOSupportAction : public ExternalToolSupportAction {
        Q_OBJECT
public:
    ClustalOSupportAction(QObject* p, GObjectView* v, const QString& text, int order)
        : ExternalToolSupportAction(p,v,text,order, QStringList(ET_CLUSTALO)) {}
    MSAEditor*  getMSAEditor() const;

private slots:
    void sl_lockedStateChanged();
};

}//namespace
#endif // _U2_CLUSTALO_SUPPORT_H
