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

#ifndef _U2_QOBJECT_SCOPED_POINTER_H_
#define _U2_QOBJECT_SCOPED_POINTER_H_

#include <QPointer>

#include <U2Core/global.h>

namespace U2 {

template <class T>
class U2GUI_EXPORT QObjectScopedPointer {
public:
    QObjectScopedPointer(T *dialog = NULL) :
        dialogPointer(dialog)
    {

    }
    ~QObjectScopedPointer() {
        delete dialogPointer.data();
    }

    bool isNull() const {
        return dialogPointer.isNull();
    }

    T * operator->() const {return dialogPointer.operator ->();}
    T & operator*() const {return *dialogPointer.operator *();}
    T * data() const {
        return dialogPointer.data();
    }
    QObjectScopedPointer<T> &operator=(T* p) {
        dialogPointer = p;
        return *this;
    }

private:
    QPointer<T> dialogPointer;
};

template <class T>
inline bool operator==(const T *o, const QObjectScopedPointer<T> &p)
{ return o == p.operator->(); }

template<class T>
inline bool operator==(const QObjectScopedPointer<T> &p, const T *o)
{ return p.operator->() == o; }

}   // namespace U2

#endif // _U2_QOBJECT_SCOPED_POINTER_H_
