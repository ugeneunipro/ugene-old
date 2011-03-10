#ifndef PRIMER3DIALOG_H
#define PRIMER3DIALOG_H

#include <QtGui/QDialog>
#include <U2View/ADVSequenceObjectContext.h>
#include <U2Gui/CreateAnnotationWidgetController.h>
#include "ui_Primer3Dialog.h"
#include "Primer3Task.h"

namespace U2 {

class Primer3Dialog : public QDialog
{
    Q_OBJECT
public:
    Primer3Dialog(const Primer3TaskSettings &defaultSettings, ADVSequenceObjectContext *context);

    Primer3TaskSettings getSettings()const;
    const CreateAnnotationModel &getCreateAnnotationModel()const;
    int getRangeStart()const;
    int getRangeEnd()const;

    void prepareAnnotationObject();
public:
    static bool parseIntervalList(QString inputString, QString delimiter, QList<QPair<int, int> > *outputList);
    static QString intervalListToString(QList<QPair<int, int> > intervalList, QString delimiter);
private:
    void reset();
    bool doDataExchange();

    void showInvalidInputMessage(QWidget *field, QString fieldLabel);
private:
    Ui::Primer3Dialog ui;

    CreateAnnotationWidgetController *createAnnotationWidgetController;
    U2Region selection;
    int sequenceLength;

    QList<QPair<QString, QByteArray> > repeatLibraries;

    Primer3TaskSettings defaultSettings;
    Primer3TaskSettings settings;
private slots:
    void on_sbRangeEnd_editingFinished();
    void on_sbRangeStart_editingFinished();
    void on_pbSequenceRange_clicked();
    void on_pbSelectionRange_clicked();
    void on_pbReset_clicked();
    void on_pbPick_clicked();
};

} // namespace U2

#endif // PRIMER3DIALOG_H
