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

#ifndef _U2_GT_CREATE_ELEMENT_WITH_COMMAND_LINE_TOOL_FILLER_H_
#define _U2_GT_CREATE_ELEMENT_WITH_COMMAND_LINE_TOOL_FILLER_H_

#include "GTUtilsDialog.h"

#include <api/GTComboBox.h>
#include <api/GTKeyboardDriver.h>
#include <api/GTLineEdit.h>
#include <api/GTMouseDriver.h>
#include <api/GTTableView.h>
#include <api/GTWidget.h>

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QApplication>
#include <QtGui/QTableView>
#else
#include <QtWidgets/QApplication>
#include <QtWidgets/QTableView>
#endif

namespace U2 {

class CreateElementWithCommandLineToolFiller : public Filler {
public:
    template <typename DataType>
    class Data {
    public:
        Data(QString name, DataType type, QString desc = QString())
            : name(name),
              type(type),
              description(desc) {}
        QString name;
        DataType type;
        QString description;
    };

    enum InOutType {
        MultipleAlignment,
        Sequence,
        SequenceWithAnnotations,
        SetOfAnnotations,
        DataString
    };
    typedef QPair<InOutType, QString> InOutDataType;
    typedef Data<InOutDataType> InOutData;

    enum ParameterType {
        Boolean,
        Number,
        ParameterString,
        URL
    };
    typedef Data<ParameterType> Parameter;

    struct ElementWithCommandLineSettings {
        QString             elementName;
        QList<InOutData>         input;
        QList<InOutData>         output;
        QList<Parameter>    parameters;
        QString             executionString;
        QString             parameterizedDescription;
    };

public:
    CreateElementWithCommandLineToolFiller(U2OpStatus& os,
                                           const ElementWithCommandLineSettings& settings);
    CreateElementWithCommandLineToolFiller(U2OpStatus &os, CustomScenario *scenario);
    void commonScenario();
private:
    QString dataTypeToString(const InOutType &type) const;
    QString dataTypeToString(const ParameterType &type) const;

    void processDataType(QTableView *table, int row, const InOutDataType &type);
    void processDataType(QTableView *table, int row, const ParameterType &type);

    template <typename DataType>
    void setType(QTableView *table, int row, const DataType &type) {
        GTMouseDriver::moveTo(os, GTTableView::getCellPosition(os, table, 1, row));
        GTMouseDriver::doubleClick(os);

        QComboBox* box = qobject_cast<QComboBox*>(QApplication::focusWidget());
        GTComboBox::setIndexWithText(os, box, dataTypeToString(type));
#ifdef Q_OS_WIN
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["enter"]);
#endif
    }

    template <typename DataType>
    void fillTheTable(QTableView* table,
                      QWidget* addRowButton,
                      QList< Data<DataType> >& rowItems) {
        QAbstractItemModel* model = table->model();
        int row = model->rowCount();

        foreach (const Data<DataType> &rowData, rowItems) {
            GTWidget::click(os, addRowButton);
            table->scrollTo(model->index(row, 0));

            GTMouseDriver::moveTo(os, GTTableView::getCellPosition(os, table, 0, row));
            GTMouseDriver::click(os);

            GTKeyboardDriver::keySequence(os, rowData.name);
            GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["enter"]);

            processDataType(table, row, rowData.type);

            GTMouseDriver::moveTo(os, GTTableView::getCellPosition(os, table, model->columnCount() - 1, row));
            GTMouseDriver::click(os);

            if (!rowData.description.isEmpty()) {
                GTKeyboardDriver::keySequence(os, rowData.description);
                GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["enter"]);
            }
            row++;
        }
    }

    ElementWithCommandLineSettings  settings;
};

} // namespace

#endif // _U2_GT_CREATE_ELEMENT_WITH_COMMAND_LINE_TOOL_FILLER_H_
