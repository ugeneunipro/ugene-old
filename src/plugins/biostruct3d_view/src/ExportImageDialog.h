#ifndef _U2_EXPORT_IMAGE_DIALOG_H_
#define _U2_EXPORT_IMAGE_DIALOG_H_

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
}; // class ExportImageDialog

} // namespace

#endif
