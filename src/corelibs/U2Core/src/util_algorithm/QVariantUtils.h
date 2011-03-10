#ifndef _U2_QVARUTILS_H_
#define _U2_QVARUTILS_H_

#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <assert.h>
#include <U2Core/global.h>
#include <U2Core/AnnotationData.h>

namespace U2 {

class U2CORE_EXPORT QVariantUtils {
public:
    static QVariant String2Var(const QString& string) {
        QDataStream s(QByteArray::fromBase64(string.toAscii()));
        return QVariant(s);
    }

    static QVariantMap string2Map(const QString& string, bool emptyMapIfError) {
        QDataStream s(QByteArray::fromBase64(string.toAscii()));
        QVariant res(QVariant::Map);
        s >> res;
        if (res.type() == QVariant::Map) {
            return res.toMap();
        }
        assert(emptyMapIfError); Q_UNUSED(emptyMapIfError);
        return QVariantMap();
    }

    static QString var2String(const QVariant& v) {
        QByteArray a;
        QDataStream s(&a, QIODevice::WriteOnly);
        s << v;
        QString res(a.toBase64());
        return res;
    }

    static QString map2String(const QVariantMap& map) {
        return var2String(QVariant(map));
    }

    static QList<SharedAnnotationData> var2ftl(const QVariantList& lst) {
        QList<SharedAnnotationData> atl;
        foreach(QVariant v, lst) {
            atl += qVariantValue<QList<SharedAnnotationData> >(v);
        }
        return atl;
    }

    static QVariant addStr2List(const QVariant& v, const QString& s) {
        if (v.canConvert(QVariant::StringList)) {
            QStringList l = v.toStringList();
            return (l << s);
        } else {
            assert(v.type() == QVariant::Invalid);
        }
        return QVariant(s);
    }

    static QVariant addStr2List(const QVariant& v, const QStringList& sl) {
        if (v.canConvert(QVariant::StringList)) {
            QStringList l = v.toStringList();
            return QVariant(l += sl);
        } else {
            assert(v.type() == QVariant::Invalid);
        }
        return QVariant(sl);
    }
};

} //namespace
#endif
