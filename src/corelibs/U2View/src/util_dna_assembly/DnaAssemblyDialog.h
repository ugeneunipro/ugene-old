#ifndef _U2_DNA_ASSEMBLEY_DIALOG_H_
#define _U2_DNA_ASSEMBLEY_DIALOG_H_

#include <QtCore/QVariant>

#include <U2Core/GUrl.h>
#include <ui/ui_AssemblyToRefDialog.h>

namespace U2 {

class DnaAssemblyAlgRegistry;
class DnaAssemblyAlgorithmMainWidget;

class DnaAssemblyDialog : public QDialog, private Ui::AssemblyToRefDialog    {
    Q_OBJECT

public:
    DnaAssemblyDialog(const DnaAssemblyAlgRegistry* registry, QWidget* p = NULL);
    const GUrl getRefSeqUrl();
    const QList<GUrl> getShortReadUrls();
    const QString getAlgorithmName();
    const QString getResultFileName();
    QMap<QString,QVariant> getCustomSettings();

protected:
    bool eventFilter(QObject *, QEvent *);

private slots:
    void sl_onAddRefButtonClicked();
    void sl_onAddShortReadsButtonClicked();
    void sl_onRemoveShortReadsButtonClicked();
    void sl_onSetResultFileNameButtonClicked();
    void sl_onAlgorithmChanged(const QString &text);

private:
    const DnaAssemblyAlgRegistry* assemblyRegistry;
    DnaAssemblyAlgorithmMainWidget* customGUI;
    static QString genomePath;
    static QList<QString> shortReads;
    void updateState();
    void addGuiExtension();
    void buildResultUrl(const GUrl& url);
    void accept();
};

} // namespace

#endif //  _U2_DNA_ASSEMBLEY_DIALOG_H_
