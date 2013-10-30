#ifndef _U2_CODON_TABLE_H_
#define _U2_CODON_TABLE_H_

#include <U2Core/DNATranslation.h>

#include <U2View/GSequenceLineViewAnnotated.h>
#include <U2View/ADVSplitWidget.h>
#include <U2View/ADVSequenceWidget.h>

#include <QtGui/QLabel>
#include <QtGui/QVBoxLayout>
#include <QtGui/QTableWidget>

namespace U2 {

class U2VIEW_EXPORT CodonTableView : public ADVSplitWidget {
    Q_OBJECT
public:
    CodonTableView(AnnotatedDNAView *view);

    virtual bool acceptsGObject(GObject*) {return false;}
    virtual void updateState(const QVariantMap& m) {}
    virtual void saveState(QVariantMap& m){}

    static const QColor NONPOLAR_COLOR;
    static const QColor POLAR_COLOR;
    static const QColor BASIC_COLOR;
    static const QColor ACIDIC_COLOR;
    static const QColor STOP_CODON_COLOR;

public slots:
    void sl_setVisible();
    void sl_setAminoTranslation();
    void sl_onSequenceFocusChanged(ADVSequenceWidget* from, ADVSequenceWidget* to);
private:
    QTableWidget *table;

    void addItemToTable(int row, int column, QString text, QColor backgroundColor = QColor(0, 0, 0, 0),
                        int rowSpan = 1, int columnSpan = 1);
    void addItemToTable(int row, int column, QString text, int rowSpan = 1, int columnSpan = 1);
    void addItemToTable(int row, int column,
                        QString text, QColor backgroundColor,
                        QString link,
                        int rowSpan = 1, int columnSpan = 1);
    void addItemToTable(int row, int column, DNACodon *codon);

    void setAminoTranslation(const QString &trId);
    void spanEqualCells();
    QColor getColor(DNACodonGroup gr);
};

class CodonTableAction : public ADVSequenceWidgetAction {
    Q_OBJECT
public:
    CodonTableAction(CodonTableView *_view);
    ~CodonTableAction() {}
    CodonTableView *view;
};

} // namespace

#endif // _U2_CODON_TABLE_H_
