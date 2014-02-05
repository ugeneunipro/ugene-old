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

#ifndef _NEW_GROUPER_SLOT_DIALOG_H_
#define _NEW_GROUPER_SLOT_DIALOG_H_

#include <U2Lang/Descriptor.h>
#include <U2Lang/GrouperOutSlot.h>

#include "ui_AnnsActionDialog.h"
#include "ui_MsaActionDialog.h"
#include "ui_NewGrouperSlotDialog.h"
#include "ui_SequenceActionDialog.h"
#include "ui_StringActionDialog.h"

namespace U2 {

class NewGrouperSlotDialog : public QDialog, public Ui::NewGrouperSlotDialog {
    Q_OBJECT
public:
    NewGrouperSlotDialog(QWidget *parent, QList<Descriptor> &inSlots, QStringList &names);
    void accept();

    QString getInSlotId() const;
    QString getOutSlotName() const;

private:
    QList<Descriptor> inSlots;
    QStringList names;
};

class ActionDialog : public QDialog {
    Q_OBJECT
public:
    ActionDialog(QWidget *parent);
    virtual GrouperSlotAction getAction() const = 0;

    static ActionDialog *getActionDialog(QWidget *parent, GrouperSlotAction *action, DataTypePtr type, QAbstractTableModel *grouperModel);
};

/************************************************************************/
/* Action dialogs */
/************************************************************************/
class AnnsActionDialog : public ActionDialog, public Ui::AnnsActionDialog {
    Q_OBJECT
public:
    AnnsActionDialog(QWidget *parent, GrouperSlotAction *action, QStringList mergeSeqSlots);
    virtual GrouperSlotAction getAction() const;
};

class SequeceActionDialog : public ActionDialog, public Ui::SequeceActionDialog {
    Q_OBJECT
public:
    SequeceActionDialog(QWidget *parent, GrouperSlotAction *action);
    virtual GrouperSlotAction getAction() const;
};

class MsaActionDialog : public ActionDialog, public Ui::MsaActionDialog {
    Q_OBJECT
public:
    MsaActionDialog(QWidget *parent, GrouperSlotAction *action);
    virtual GrouperSlotAction getAction() const;
};

class StringActionDialog : public ActionDialog, public Ui::StringActionDialog {
    Q_OBJECT
public:
    StringActionDialog(QWidget *parent, GrouperSlotAction *action);
    virtual GrouperSlotAction getAction() const;
};

} // U2

#endif // _NEW_GROUPER_SLOT_DIALOG_H_
