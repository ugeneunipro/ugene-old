#ifndef _U2_CODON_TABLE_H_
#define _U2_CODON_TABLE_H_

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
private:
    void addItemToTable(QTableWidget *table, int row, int column, QString text, QColor backgroundColor = QColor(0, 0, 0, 0),
                        int rowSpan = 1, int columnSpan = 1);
    void addItemToTable(QTableWidget *table, int row, int column, QString text, int rowSpan = 1, int columnSpan = 1);

    void addItemToTable(QTableWidget *table, int row, int column,
                        QString text, QColor backgroundColor,
                        QString link,
                        int rowSpan = 1, int columnSpan = 1);
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
