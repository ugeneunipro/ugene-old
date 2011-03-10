#ifndef _U2_DOT_PLOT_FILES_DIALOG_H_
#define _U2_DOT_PLOT_FILES_DIALOG_H_

#include <ui/ui_DotPlotFilesDialog.h>

namespace U2 {
class ADVSequenceObjectContext;

class DotPlotFilesDialog : public QDialog, public Ui_DotPlotFilesDialog{
    Q_OBJECT
public:
    DotPlotFilesDialog(QWidget *parent);

    virtual void accept();

    QString getFirstFileName() const {return firstFileName;}
    QString getSecondFileName() const {return secondFileName;}

    int getFirstGap() const;
    int getSecondGap() const;

protected slots:
    void sl_openFirstFile();
    void sl_openSecondFile();

    void sl_oneSequence();

    void sl_mergeFirst();
    void sl_mergeSecond();
private:
    QString firstFileName;
    QString secondFileName;

    QString filter;
};

} //namespace

#endif
