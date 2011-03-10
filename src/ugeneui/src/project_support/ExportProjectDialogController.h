#ifndef _U2_EXPORT_PROJECT_DIALOG_CONTROLLER_
#define _U2_EXPORT_PROJECT_DIALOG_CONTROLLER_

#include <ui/ui_ExportProjectDialog.h>

#include <U2Core/AppContext.h>

namespace U2 {

class ExportProjectDialogController : public QDialog, public Ui_ExportProjectDialog {
	Q_OBJECT
public:
	ExportProjectDialogController(QWidget *p, const QString& defaultProjectFileName);

	void accept();

	const QString& getDirToSave() const {return exportDir;}
    const QString& getProjectFile() const {return projectFile;}

	bool useCompression() const {return false;}

private slots:
	void sl_onBrowseButton();

private:
    //canonical representation of the dir
    QString exportDir;
    QString projectFile;
};

} //namespace

#endif
