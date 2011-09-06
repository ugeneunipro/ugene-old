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

#ifndef _U2_TCOFFEE_SUPPORT_H
#define _U2_TCOFFEE_SUPPORT_H

#include <U2Core/ExternalToolRegistry.h>
#include <U2View/MSAEditor.h>
#include "utils/ExternalToolSupportAction.h"

#define TCOFFEE_TOOL_NAME "T-Coffee"

namespace U2 {

class TCoffeeSupport : public ExternalTool {
    Q_OBJECT
public:
    TCoffeeSupport(const QString& name, const QString& path = "");
    GObjectViewWindowContext* getViewContext(){ return viewCtx; }
public slots:
    void sl_runWithExtFileSpecify();
private:
    GObjectViewWindowContext* viewCtx;
};

class TCoffeeSupportContext: public GObjectViewWindowContext {
    Q_OBJECT
public:
    TCoffeeSupportContext(QObject* p);

protected slots:
    void sl_align_with_TCoffee();

protected:
    virtual void initViewContext(GObjectView* view);
    virtual void buildMenu(GObjectView* view, QMenu* m);
};

class TCoffeeSupportAction : public ExternalToolSupportAction {
        Q_OBJECT
public:
    TCoffeeSupportAction(QObject* p, GObjectView* v, const QString& text, int order)
        : ExternalToolSupportAction(p,v,text,order, QStringList(TCOFFEE_TOOL_NAME)) {}
    MSAEditor*  getMSAEditor() const;

private slots:
    void sl_lockedStateChanged();
};

}//namespace
#endif // _U2_TCOFFEE_SUPPORT_H
