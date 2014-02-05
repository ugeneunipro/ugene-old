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

#ifndef  _U2_PAN_VIEW_ROWS_H_
#define  _U2_PAN_VIEW_ROWS_H_

#include <QtCore/QString>
#include <QtCore/QMap>
#include <QtCore/QList>

#include <U2Core/Annotation.h>
#include <U2Core/U2Region.h>

namespace U2 {

class PVRowData {
public:
    PVRowData(const QString& _key) : key(_key){}

    bool fitToRow(const QVector<U2Region>& locations);

    QString key;
    //invariant: keep the ranges in ascending order
    QVector<U2Region> ranges;
    QList<Annotation> annotations;
};

class PVRowsManager {
public:
    PVRowsManager() {}
    ~PVRowsManager();
    void clear();

    void addAnnotation( const Annotation &f, const QString &key );
    void removeAnnotation( const Annotation &f );

    bool contains(const QString& key) const;
    int getNumRows() const {return rows.size();}
    PVRowData* getRow(int row) const;

    /**
     * returns -1 if @a is not found
     */
    int getAnnotationRowIdx( const Annotation &a ) const;
    PVRowData* getAnnotationRow( const Annotation &a ) const {return rowByAnnotation.value(a, NULL);}
    
    int getNumAnnotationsInRow(int rowNum) const;

private:
    QList<PVRowData*> rows;
    QMap<Annotation, PVRowData*> rowByAnnotation;
};

} // namespace

#endif
