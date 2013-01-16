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

#ifndef _U2_DIFFERENTIALFORMAT_H_
#define _U2_DIFFERENTIALFORMAT_H_

#include "AbstractDifferentialFormat.h"

namespace U2 {

// http://cufflinks.cbcb.umd.edu/manual.html#gene_exp_diff
class U2FORMATS_EXPORT DifferentialExpressionFormat : public AbstractDifferentialFormat {
    Q_OBJECT
public:
    DifferentialExpressionFormat(QObject *parent);

    DocumentFormatId getFormatId() const;
    const QString & getFormatName() const;

protected:
    QList<ColumnDataParser::Column> getColumns() const;
    QString getAnnotationName() const;

private:
    QString formatName;
    QString annotationName;
    QList<ColumnDataParser::Column> columns;
};

} // U2

#endif // _U2_DIFFERENTIALFORMAT_H_
