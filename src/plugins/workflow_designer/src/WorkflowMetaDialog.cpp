#include "WorkflowMetaDialog.h"

#include <U2Lang/WorkflowUtils.h>
#include <U2Core/AppContext.h>
#include <U2Core/Settings.h>
#include <U2Designer/DesignerUtils.h>
#include <QtGui/QFileDialog>

namespace U2 {

WorkflowMetaDialog::WorkflowMetaDialog(const Metadata& meta): meta(meta) {
    setupUi(this);

    connect(browseButton, SIGNAL(clicked()), SLOT(sl_onBrowse()));
    connect(cancelButton, SIGNAL(clicked()), SLOT(reject()));
    connect(okButton, SIGNAL(clicked()), SLOT(sl_onSave()));
    connect(urlEdit, SIGNAL(textChanged(const QString & )), SLOT(sl_onURLChanged(const QString& )));
    connect(urlEdit, SIGNAL(textEdited (const QString & )), SLOT(sl_onURLChanged(const QString& )));

    urlEdit->setText(meta.url);
    okButton->setDisabled(meta.url.isEmpty());
    nameEdit->setText(meta.name);
    commentEdit->setText(meta.comment);
}

void WorkflowMetaDialog::sl_onSave() {
    assert(!WorkflowUtils::WD_FILE_EXTENSIONS.isEmpty());
    QString url = urlEdit->text();
    bool endsWithWDExt = false;
    foreach( const QString & ext, WorkflowUtils::WD_FILE_EXTENSIONS ) {
        assert(!ext.isEmpty());
        if(url.endsWith(ext)) {
            endsWithWDExt = true;
        }
    }
    if(!endsWithWDExt) {
        url.append("." + WorkflowUtils::WD_FILE_EXTENSIONS.first());
    }
    meta.url = url;
    meta.comment = commentEdit->toPlainText();
    meta.name = nameEdit->text();
    accept();
}

void WorkflowMetaDialog::sl_onURLChanged(const QString & text) {
    okButton->setDisabled(text.isEmpty());
}

#define LAST_DIR QString("workflowview/lastdir")

void WorkflowMetaDialog::sl_onBrowse() {
    QString url = urlEdit->text();
    if (url.isEmpty()) {
        url = AppContext::getSettings()->getValue(LAST_DIR, QString("")).toString();
    }
    QString filter = DesignerUtils::getSchemaFileFilter(false);
    url = QFileDialog::getSaveFileName(0, tr("Save workflow schema to file"), url, filter);
    if (!url.isEmpty()) {
        AppContext::getSettings()->setValue(LAST_DIR, QFileInfo(url).absoluteDir().absolutePath());
        urlEdit->setText(url);
    }
}


}//namespace
