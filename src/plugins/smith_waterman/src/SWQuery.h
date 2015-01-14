/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#ifndef _U2_SW_QUERY_H_
#define _U2_SW_QUERY_H_

#include <U2Lang/QDScheme.h>
#include <U2Lang/QueryDesignerRegistry.h>
#include <U2Algorithm/SmithWatermanSettings.h>
#include <U2Algorithm/SmithWatermanTaskFactory.h>
#include <U2Designer/DelegateEditors.h>

namespace U2 {

class Attribute;

class SWAlgoEditor : public ComboBoxDelegate {
    Q_OBJECT
public:
    SWAlgoEditor(Attribute* algAttr) : ComboBoxDelegate(QVariantMap()), algAttr(algAttr) {}
public slots:
    void populate();
private:
    Attribute* algAttr;
};

class QDSWActor : public QDActor {
    Q_OBJECT
public:
    QDSWActor(QDActorPrototype const* proto);
    int getMinResultLen() const;
    int getMaxResultLen() const;
    QString getText() const;
    Task* getAlgorithmTask(const QVector<U2Region>& location);
    QColor defaultColor() const { return QColor(0xff,0xf8,0); }
private slots:
    void sl_onAlgorithmTaskFinished(Task*);
private:
    SmithWatermanSettings settings;
    QString transId, mtrx;
    SmithWatermanTaskFactory* algo;
    QMap<Task*, SmithWatermanReportCallbackAnnotImpl*> callbacks;
};

class SWQDActorFactory : public QDActorPrototype {
public:
    SWQDActorFactory();
    QIcon getIcon() const { return QIcon(":core/images/sw.png"); }
    virtual QDActor* createInstance() const { return new QDSWActor(this); }
};

}//namespace

#endif
