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

#ifndef _U2_EXPORT_PROJECT_VIEW_ITEMS_H_
#define _U2_EXPORT_PROJECT_VIEW_ITEMS_H_

#include <U2Core/global.h>

#include <QtGui/QAction>
#include <QtGui/QMenu>

namespace U2 {

class ExportProjectViewItemsContoller : public QObject {
    Q_OBJECT
public:
    ExportProjectViewItemsContoller(QObject* p);

private slots:
    void sl_addToProjectViewMenu(QMenu&);

    void sl_saveSequencesToSequenceFormat();
    void sl_saveSequencesAsAlignment();
    void sl_saveAlignmentAsSequences();
    void sl_exportNucleicAlignmentToAmino();
    void sl_importAnnotationsFromCSV();
    void sl_exportChromatogramToSCF();
    void sl_exportAnnotations();

private:
    void addExportMenu(QMenu& m);
    void addImportMenu(QMenu& m);

    QAction* exportSequencesToSequenceFormatAction;
    QAction* exportSequencesAsAlignmentAction;
    QAction* exportAlignmentAsSequencesAction;
    QAction* exportNucleicAlignmentToAminoAction;
    QAction* importAnnotationsFromCSVAction;
    QAction* exportDNAChromatogramAction;
    QAction* exportAnnotations2CSV;

};

}//namespace

#endif
