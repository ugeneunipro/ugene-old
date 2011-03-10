#include "ChooseItemDialog.h"

#include <U2Lang/WorkflowUtils.h>

namespace U2 {

ChooseItemDialog::ChooseItemDialog(QWidget* p) : QDialog(p)
{
    setupUi(this);
    connect(listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), SLOT(accept()));
}

QString ChooseItemDialog::select(const QMap<QString, QIcon>& items) {
    listWidget->clear();
    QMapIterator<QString, QIcon> it(items);
    while (it.hasNext())
    {
        it.next();
        listWidget->addItem(new QListWidgetItem(it.value(), it.key()));
    }
    listWidget->setItemSelected(listWidget->item(0), true);
    if (exec() == QDialog::Accepted) {
        return listWidget->currentItem()->text();
    } else {
        return QString();
    }
}

Workflow::ActorPrototype* ChooseItemDialog::select(const QList<Workflow::ActorPrototype*>& items) {
    listWidget->clear();
    foreach(Workflow::ActorPrototype* a, items) {
        QListWidgetItem* it = new QListWidgetItem(a->getIcon(), a->getDisplayName());
        it->setToolTip(a->getDocumentation());
        listWidget->addItem(it);
    }
    listWidget->setItemSelected(listWidget->item(0), true);
    if (exec() == QDialog::Accepted) {
        return items.at(listWidget->currentRow());
    } else {
        return NULL;
    }
}

}//namespace
