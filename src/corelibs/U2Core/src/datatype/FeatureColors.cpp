#include "FeatureColors.h"

namespace U2 {

static QList<QColor> prepareColors() {
    QList<QColor> colors;
    QString mustHave = "FF";
    QStringList optional;
    optional<<"FF"<<"CC"<<"99"<<"66"<<"33"<<"00";
    for(int i = 0; i < 3; i++) { 
        for(int j=0; j < optional.size(); j++) {
            for(int k=1; k < optional.size(); k++) { //todo: avoid duplicates
                QString colorName = 
                    i == 0 ? mustHave + optional[j] + optional[k] : 
                    i == 1 ? optional[j] + mustHave + optional[k] :
                    optional[j] + optional[k] + mustHave;
                QColor c("#"+colorName);
                assert(c.isValid());
                colors.append(c);
            }
        }
    }
    return colors;
}

QColor FeatureColors::genLightColor(const QString& name) {
    static QList<QColor> colors = prepareColors();

    int hash = 0;
    QByteArray a = name.toAscii();
    for (int i=0;i<a.size(); i++) {
        hash+=a[i];
    }
    QColor c = colors.at((hash*hash)%colors.size());
    return c;
}

} //endif
