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

#include "StateLockableDataModel.h"


#include <QtGui/QApplication>
#include <QtCore/QThread>

namespace U2 {

#define MAIN_THREAD_THREAD_MODEL

StateLockableItem::StateLockableItem(QObject* p)  : QObject(p), itemIsModified(false), mainThreadModel(false), modificationVersion(0)
{
}

StateLockableItem::~StateLockableItem(){
    foreach(StateLock* lock, locks) {
        assert(!lock->getFlags().testFlag(StateLockFlag_LiveLock));
        delete lock;
    }
}

static void checkThread(StateLockableItem* i) {
#ifdef _DEBUG
    if (i->isMainThreadModel()) {
        QThread* appThread = QApplication::instance()->thread();
        QThread* thisThread = QThread::currentThread();
        assert (appThread == thisThread);
    }
#else 
    Q_UNUSED(i);
#endif    
}


void StateLockableItem::lockState(StateLock* lock) {
    assert(!locks.contains(lock));
    checkThread(this);

    locks.append(lock);
    if (locks.size() == 1) {
        emit si_lockedStateChanged();
    }
}


void StateLockableItem::unlockState(StateLock* lock) {
    assert(locks.contains(lock));
    checkThread(this);

    locks.removeOne(lock);
    
    if (locks.isEmpty()) {
        emit si_lockedStateChanged();
    }
}


void StateLockableItem::setModified(bool d) {
    assert(!d || !isStateLocked());
    checkThread(this);
    if (d) {
        modificationVersion++;
    }
    if (itemIsModified == d) {
        return;
    }
    itemIsModified = d;
    emit si_modifiedStateChanged();
}

//////////////////////////////////////////////////////////////////////////
// Tree

StateLockableTreeItem::~StateLockableTreeItem() {
    // TODO: check that there are no live-locks and delete all child items
    assert(!hasLocks(StateLockableTreeItemBranch_Item, StateLockFlag_LiveLock));
}

bool StateLockableTreeItem::isStateLocked() const {
    return StateLockableItem::isStateLocked() || (parentStateLockItem!=NULL ? parentStateLockItem->isStateLocked() : false);
}

void StateLockableTreeItem::lockState(StateLock* lock) {
    assert(!locks.contains(lock));
    checkThread(this);

    bool wasUnlocked = !isStateLocked();

    locks.append(lock); 
    
    //notify all children on parent lock state change
    if (wasUnlocked) {
        foreach(StateLockableTreeItem* c, childItems) {
            c->onParentStateLocked();
        }
    }
    emit si_lockedStateChanged();
}

void StateLockableTreeItem::unlockState(StateLock* lock) {
    assert(locks.contains(lock));
    checkThread(this);

    locks.removeOne(lock);

    bool becomeUnlocked = !isStateLocked();

    emit si_lockedStateChanged();

    if (becomeUnlocked) {
        foreach(StateLockableTreeItem* c, childItems) {
            c->onParentStateUnlocked();
        }
    }
}

void StateLockableTreeItem::onParentStateLocked() {
    //parent has become locked -> check if my state is changed
    if (!locks.isEmpty()) {
        return; //nothing changed - was locked too
    }

    //notify children
    assert(isStateLocked());

    foreach(StateLockableTreeItem* c, childItems) {
        c->onParentStateLocked();
    }

    emit si_lockedStateChanged();
}

void StateLockableTreeItem::onParentStateUnlocked() {
    //parent has become unlocked -> check if my state is changed
    if (!locks.isEmpty()) {
        return; //nothing chaged - was still locked 
    }

    //notify childred
    assert(!isStateLocked());
    
    emit si_lockedStateChanged();

    foreach(StateLockableTreeItem* c, childItems) {
        c->onParentStateUnlocked();
    }
}

void StateLockableTreeItem::setParentStateLockItem(StateLockableTreeItem* newParent, bool ignoreLocks, bool modify) { 
    assert(parentStateLockItem == NULL || newParent == NULL);
    assert(newParent == NULL || !newParent->isStateLocked() || ignoreLocks);
    assert(parentStateLockItem == NULL || !parentStateLockItem->isStateLocked() || ignoreLocks);

    StateLockableTreeItem* oldParent = parentStateLockItem; 
    parentStateLockItem = newParent;
    setParent(newParent);
    
    bool treeMod = isTreeItemModified();
    assert(modify || !treeMod);
    if (newParent!=NULL) {
        setMainThreadModel(newParent->isMainThreadModel());
        checkThread(this);
        newParent->childItems.insert(this);
        if (modify) {
            newParent->setModified(true, ignoreLocks);
            if (treeMod) {
                newParent->increaseNumModifiedChilds(numModifiedChildren + 1);
            }
        } 
    } else if (oldParent!=NULL) {
        oldParent->childItems.remove(this);
        if (modify) {
            oldParent->setModified(true, ignoreLocks);
            if (treeMod) {
                oldParent->decreaseNumModifiedChilds(numModifiedChildren + 1);
            }
        } 
    }
}


void StateLockableTreeItem::setModified(bool d) {
    setModified(d, false);
}

void StateLockableTreeItem::setModified(bool d, bool ignoreLocks) {
    assert(!d|| !isStateLocked() || ignoreLocks); Q_UNUSED(ignoreLocks);
    if (d) {
        modificationVersion++;
    }
    if (itemIsModified == d) {
        return;
    }
    itemIsModified = d;
    if (parentStateLockItem && numModifiedChildren == 0) {
        if (itemIsModified) {
            parentStateLockItem->increaseNumModifiedChilds(1);
        } else {
            parentStateLockItem->decreaseNumModifiedChilds(1);
        }
    }
    if (!hasModifiedChildren()) {
        checkThread(this);
        emit si_modifiedStateChanged();
    }
}

void StateLockableTreeItem::increaseNumModifiedChilds(int n) {
    assert(n > 0);
    numModifiedChildren+=n;

    bool becomeModified = numModifiedChildren == n && !itemIsModified;
    if (parentStateLockItem) {
        parentStateLockItem->increaseNumModifiedChilds(n + (becomeModified  ? 1 : 0) );
    }
    if (becomeModified) {
        assert(isTreeItemModified());
        checkThread(this);
        emit si_modifiedStateChanged();
    }
}

void StateLockableTreeItem::decreaseNumModifiedChilds(int n) {
    assert(n > 0);
    assert(numModifiedChildren >= n);
    numModifiedChildren-=n;

    bool becomeClean = numModifiedChildren == 0 && !itemIsModified;

    if (parentStateLockItem) {
        parentStateLockItem->decreaseNumModifiedChilds(n + (becomeClean ? 1 : 0));
    }

    if (becomeClean)  {
        assert(!isTreeItemModified());
        checkThread(this);
        emit si_modifiedStateChanged();
    }

}

QList<StateLock*> StateLockableTreeItem::findLocks(StateLockableTreeItemBranchFlags tf, StateLockFlag lf) const {
    QList<StateLock*> res;

    if (tf.testFlag(StateLockableTreeItemBranch_Item)) {
        foreach(StateLock* lock, locks) {
            if (lock->getFlags().testFlag(lf)) {
                res.append(lock);
            }
        }
    }
    
    if (tf.testFlag(StateLockableTreeItemBranch_Parents) && parentStateLockItem!=NULL) {
        StateLockableTreeItemBranchFlags tflags(StateLockableTreeItemBranch_Parents | StateLockableTreeItemBranch_Item);
        res+=parentStateLockItem->findLocks(tflags, lf);
    }

    if (tf.testFlag(StateLockableTreeItemBranch_Children)) {
        StateLockableTreeItemBranchFlags tflags(StateLockableTreeItemBranch_Children| StateLockableTreeItemBranch_Item);
        foreach(const StateLockableTreeItem* child, childItems) {
            res+=child->findLocks(tflags, lf);
        }
    }

    return res;
}


}//namespace

