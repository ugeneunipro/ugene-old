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

#include "TableWidget.h"

namespace U2 {

static const int MIN_ROW_COUNT = 3;

TableWidget::TableWidget(const QWebElement &container, Dashboard *parent)
: DashboardWidget(container, parent), useEmptyRows(true)
{

}

void TableWidget::createTable() {
    QString table = "<table class=\"table table-bordered table-fixed\">";
    foreach (int w, widths()) {
        table += "<col width=\"" + QString("%1").arg(w) + "%\" />";
    }
    table += "<thead><tr>";
    foreach (const QString &h, header()) {
        table += "<th><span class=\"text\">" + h + "</span></th>";
    }
    table += "</tr></thead>";
    table += "<tbody scroll=\"yes\"/>";
    table += "</table>";
    container.setInnerXml(table);

    rows.clear();
    if (useEmptyRows) {
        addEmptyRows();
    }
}

void TableWidget::fillTable() {
    rows.clear();
    foreach (const QStringList &row, data()) {
        addRow(row.first(), row.mid(1));
    }
}

void TableWidget::addEmptyRows() {
    QWebElement body = container.findFirst("tbody");
    int rowIdx = rows.size();
    while (rowIdx < MIN_ROW_COUNT) {
        QString row = "<tr class=\"empty-row\">";
        foreach (const QString &h, header()) {
            Q_UNUSED(h);
            row += "<td>&nbsp;</td>";
        }
        row += "</tr>";
        body.appendInside(row);
        rowIdx++;
    }
}

void TableWidget::addRow(const QString &dataId, const QStringList &ds) {
    QString row;
    row += "<tr class=\"filled-row\">";
    row += createRow(ds);
    row += "</tr>";

    QWebElement body = container.findFirst("tbody");
    QWebElement emptyRow = body.findFirst(".empty-row");
    if (emptyRow.isNull()) {
        body.appendInside(row);
        rows[dataId] = body.lastChild();
    } else {
        emptyRow.setOuterXml(row);
        rows[dataId] = body.findAll(".filled-row").last();
    }
}

void TableWidget::updateRow(const QString &dataId, const QStringList &d) {
    if (rows.contains(dataId)) {
        rows[dataId].setInnerXml(createRow(d));
    } else {
        addRow(dataId, d);
    }
}

QString TableWidget::createRow(const QStringList &ds) {
    QString row;
    foreach (const QString &d, ds) {
        row += "<td>" + d + "</td>";
    }
    return row;
}

QString TableWidget::wrapLongText(const QString &text) {
    return "<div class=\"long-text\" title=\"" + text + "\">" + text + "</div>";
}

} // U2
