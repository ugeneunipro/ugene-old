/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#ifndef _U2_SEQUENCEPROTOTYPE_H_
#define _U2_SEQUENCEPROTOTYPE_H_

#include <U2Core/U2OpStatus.h>

#include "DbiClassPrototype.h"

#include <QObject>
#include <QtScript>

namespace U2 {

/************************************************************************/
/* SequencePrototype */
/************************************************************************/
class SequencePrototype : public DbiClassPrototype {
    Q_OBJECT
public:
    SequencePrototype(QObject *parent = NULL);

public slots:
    /** No arguments. Returns sequence length */
    qint64 length();

    /**
     * Arguments: [startPos [, length]]
     * If length is not specified than returns the data from startPos
     * up to the end os the sequence. If startPos is not specified then
     * returns the whole sequence data.
     */
    QString getData();

private:
    U2SequenceObject * getSequenceObject() const;
    /** Returns NULL if this object is invalid */
    U2SequenceObject * getValidSequenceObject() const;
};

/************************************************************************/
/* SequenceScriptClass */
/************************************************************************/
class SequenceScriptClass : public DbiScriptClass {
public:
    SequenceScriptClass(QScriptEngine *engine);

    QString name() const;
    QScriptValue newInstance(const QString &data, const QString &name);

    static QScriptValue constructor(QScriptContext *ctx, QScriptEngine *engine);
    static const QString SCRIPT_CLASS_NAME;

private:
    WorkflowScriptEngine * workflowEngine() const;
};

} // U2

Q_DECLARE_METATYPE(U2::SequenceScriptClass*)

#endif // _U2_SEQUENCEPROTOTYPE_H_
