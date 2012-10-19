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

#ifndef _U2_GENBANK_LOCATION_PARSER_H
#define _U2_GENBANK_LOCATION_PARSER_H

#include <U2Core/global.h>
#include <U2Core/U2Region.h>
#include <U2Core/U2Annotation.h>

#include <QtCore/QList>

namespace U2 {

class AnnotationData;

namespace Genbank {

class U2FORMATS_EXPORT LocationParser : public QObject {
    Q_OBJECT
public:
    /**/
    static QString parseLocation(const char* str, int len, U2Location& location, qint64 seqlenForCircular = -1);

    static QString buildLocationString(const AnnotationData* a);

    static QString buildLocationString(const QVector<U2Region>& regions);
};


}}//namespace
#endif
