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

#ifndef _U2_DNA_ASSEMBLEY_DIALOG_H_
#define _U2_DNA_ASSEMBLEY_DIALOG_H_

#include <U2Core/GUrl.h>

#include <ui/ui_AssemblyToRefDialog.h>

#include <QtCore/QVariant>
#include <QtCore/QStringList>


namespace U2 {

class DnaAssemblyAlgRegistry;
class DnaAssemblyAlgorithmMainWidget;

class DnaAssemblyDialog : public QDialog, private Ui::AssemblyToRefDialog {
    Q_OBJECT

public:
    DnaAssemblyDialog(QWidget* p = NULL, const QStringList& shortReadsUrls = QStringList(), const QString& refSeqUrl = QString());
    const GUrl getRefSeqUrl();
    const QList<GUrl> getShortReadUrls();
    const QString getAlgorithmName();
    const QString getResultFileName();
    bool isPrebuiltIndex() const;
    bool isSamOutput() const;
    QMap<QString,QVariant> getCustomSettings();

protected:
    bool eventFilter(QObject *, QEvent *);

private slots:
    void sl_onAddRefButtonClicked();
    void sl_onAddShortReadsButtonClicked();
    void sl_onRemoveShortReadsButtonClicked();
    void sl_onSetResultFileNameButtonClicked();
    void sl_onAlgorithmChanged(const QString &text);
    void sl_onPrebuiltIndexBoxClicked();
    void sl_onSamBoxClicked();

private:
    void updateState();
    void addGuiExtension();
    void buildResultUrl(const GUrl& url);
    void accept();


    const DnaAssemblyAlgRegistry*   assemblyRegistry;
    DnaAssemblyAlgorithmMainWidget* customGUI;
    static QString                  lastRefSeqUrl;
    static QStringList              lastShortReadsUrls;
    static QString                  methodName;
    static bool                     prebuiltIndex;
    static bool                     samOutput;
};

} // namespace

#endif //  _U2_DNA_ASSEMBLEY_DIALOG_H_
