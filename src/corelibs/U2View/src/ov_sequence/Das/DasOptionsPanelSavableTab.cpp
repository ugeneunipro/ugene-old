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

#include <QListWidget>

#include <U2Core/U2SafePoints.h>

#include <U2Gui/U2WidgetStateStorage.h>

#include "DasOptionsPanelWidget.h"

#include "DasOptionsPanelSavableTab.h"

static const QString CHANGE_MODE_LABEL_NAME = "lblShowMoreLess";

struct ListWidgetItemData {
    ListWidgetItemData()
        : checkState(Qt::Unchecked)
    {

    }

    ListWidgetItemData(const QString &text, const QString &userData, const QString &toolTip, Qt::CheckState checkState)
        : text(text), userData(userData), toolTip(toolTip), checkState(checkState)
    {

    }

    QString text;
    QString userData;
    QString toolTip;
    Qt::CheckState checkState;
};

Q_DECLARE_METATYPE(QVector<ListWidgetItemData>)

namespace U2 {

DasOptionsPanelSavableTab::DasOptionsPanelSavableTab(QWidget *wrappedWidget, MWMDIWindow *contextWindow)
    : U2SavableWidget(wrappedWidget, contextWindow)
{
    SAFE_POINT(NULL != qobject_cast<DasOptionsPanelWidget *>(wrappedWidget), "Invalid widget provided", );
}

DasOptionsPanelSavableTab::~DasOptionsPanelSavableTab() {
    U2WidgetStateStorage::saveWidgetState(*this);
    widgetStateSaved = true;
}

namespace {

QVector<ListWidgetItemData> getListWidgetContent(QListWidget *listWidget) {
    QVector<ListWidgetItemData> result;

    const int rowCount = listWidget->count();
    result.reserve(rowCount);

    for (int i = 0; i < rowCount; ++i) {
        QListWidgetItem *item = listWidget->item(i);
        const QString text = item->data(Qt::DisplayRole).toString();
        const QString userData = item->data(Qt::UserRole).toString();
        const QString toolTip = item->toolTip();
        const Qt::CheckState checked = item->checkState();
        result.append(ListWidgetItemData(text, userData, toolTip, checked));
    }
    return result;
}

void setListWidgetContent(QListWidget *listWidget, const QVector<ListWidgetItemData> &content) {
    listWidget->clear();
    for (int i = 0, n = content.size(); i < n; ++i) {
        QListWidgetItem *item = new QListWidgetItem(content[i].text);
        item->setData(Qt::UserRole, content[i].userData);
        item->setToolTip(content[i].toolTip);
        item->setCheckState(content[i].checkState);
        listWidget->addItem(item);
    }
}

}

QVariant DasOptionsPanelSavableTab::getChildValue(const QString &childId) const {
    SAFE_POINT(childExists(childId), "Child widget expected", QVariant());

    QWidget *child = getChildWidgetById(childId);
    if (NULL != qobject_cast<QListWidget *>(child)) {
        return QVariant::fromValue<QVector<ListWidgetItemData> >(getListWidgetContent(qobject_cast<QListWidget *>(child)));
    } else if (CHANGE_MODE_LABEL_NAME == child->objectName()) {
        return qobject_cast<DasOptionsPanelWidget *>(wrappedWidget)->isExtendedMode();
    } else {
        return U2SavableWidget::getChildValue(childId);
    }
}

void DasOptionsPanelSavableTab::setChildValue(const QString &childId, const QVariant &value) {
    SAFE_POINT(childExists(childId), "Child widget expected", );

    QWidget *child = getChildWidgetById(childId);
    if (NULL != qobject_cast<QListWidget *>(child)) {
        setListWidgetContent(qobject_cast<QListWidget *>(child), value.value<QVector<ListWidgetItemData> >());
    } else if (CHANGE_MODE_LABEL_NAME == child->objectName()) {
        qobject_cast<DasOptionsPanelWidget *>(wrappedWidget)->setExtendedMode(value.toBool());
    } else {
        U2SavableWidget::setChildValue(childId, value);
    }
}

bool DasOptionsPanelSavableTab::childCanBeSaved(QWidget *child) const {
    return U2SavableWidget::childCanBeSaved(child)
        || NULL != qobject_cast<QListWidget *>(child)
        || CHANGE_MODE_LABEL_NAME == child->objectName();
}

} // namespace U2
