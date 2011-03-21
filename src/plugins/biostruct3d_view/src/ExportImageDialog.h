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

#ifndef _U2_EXPORT_IMAGE_DIALOG_H_
#define _U2_EXPORT_IMAGE_DIALOG_H_

#include <U2Misc/DialogUtils.h>

#include <QtCore/QList>
#include <QtCore/QString>

#include "ui_ExportImageDialog.h"

namespace U2 {

class BioStruct3DGLWidget;

class ExportImageDialog : public QDialog, private Ui::ImageExportForm
{
    Q_OBJECT

public:
    ExportImageDialog(BioStruct3DGLWidget* widget);

public slots:
    virtual void accept();

private slots:
    void sl_onBrowseButtonClick();
    void sl_onFormatsBoxItemChanged(const QString& text);

private:
    void setupComponents();
    void setSizeControlsEnabled(bool enabled);

    static bool isVectorGraphicFormat(const QString& formatName);
    static int getVectorFormatIdByName(const QString& formatName);

private:
    QList<QString> supportedFormats;
    BioStruct3DGLWidget* glWidget;

    LastOpenDirHelper lod;
}; // class ExportImageDialog

} // namespace

#endif
