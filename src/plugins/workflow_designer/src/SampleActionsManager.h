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

#ifndef _U2_SAMPLE_ACTION_H_
#define _U2_SAMPLE_ACTION_H_

#include <QStringList>

#include <U2Core/U2OpStatus.h>

namespace U2 {

class SampleAction {
public:
    enum LoadingMode {Select, Load, OpenWizard};
    SampleAction(const QString &actionName, const QString &toolsCategory, const QString &samplePath, LoadingMode mode);

    QString actionName;
    QString toolsCategory;
    QString samplePath;
    LoadingMode mode;
    QStringList requiredPlugins;
};

class SampleActionsManager : public QObject {
    Q_OBJECT
public:
    SampleActionsManager(QObject *parent);
    void registerAction(const SampleAction &action);

signals:
    void si_clicked(const SampleAction &action);

private slots:
    void sl_clicked();

private:
    int getValidClickedActionId(U2OpStatus &os) const;
    SampleAction getClickedAction(U2OpStatus &os) const;
    QStringList getAbsentPlugins(const QStringList &requiredPlugins) const;

private:
    QList<SampleAction> actions;
};

} // U2

#endif // _U2_SAMPLE_ACTION_H_
