#ifndef _U2_BUILD_INDEX_DIALOG_H_
#define _U2_BUILD_INDEX_DIALOG_H_

#include <U2Core/GUrl.h>
#include <ui/ui_BuildIndexFromRefDialog.h>

#include <QtCore/QVariant>

namespace U2 {

class DnaAssemblyAlgRegistry;
class DnaAssemblyAlgorithmBuildIndexWidget;

class BuildIndexDialog : public QDialog, private Ui::BuildIndexFromRefDialog    {
	Q_OBJECT

public:
	BuildIndexDialog(const DnaAssemblyAlgRegistry* registry, QWidget* p = NULL);
	const GUrl getRefSeqUrl();
	const QString getAlgorithmName();
	const QString getIndexFileName();
	QMap<QString,QVariant> getCustomSettings();

private:
	const DnaAssemblyAlgRegistry *assemblyRegistry;
	DnaAssemblyAlgorithmBuildIndexWidget *customGUI;
	static QString genomePath;
	void buildIndexUrl(const GUrl& url);
	void updateState();
	void addGuiExtension();
	void accept();

private slots:
	void sl_onAddRefButtonClicked();
	void sl_onSetIndexFileNameButtonClicked();
	void sl_onAlgorithmChanged(const QString &text);
};

} // namespace

#endif //  _U2_BUILD_INDEX_DIALOG_H_
