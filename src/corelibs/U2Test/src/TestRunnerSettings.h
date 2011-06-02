#ifndef _U2_TEST_RUNNER_SETTINGS_H_
#define _U2_TEST_RUNNER_SETTINGS_H_


namespace U2 {

class TestRunnerSettings {
public:
    QString getVar(const QString& name) const { return registry.value(name); }
    void setVar(const QString& name, const QString& val) { registry[name] = val; }
    void removeVar(const QString& name) { registry.remove(name); }
private:
    QMap<QString, QString> registry;
};

class APITestData {
public:
    template<class T>
    T getValue(const QString& key) const {
        const QVariant& val = d.value(key);
        return qVariantValue<T>(val);
    }

    template<class T>
    bool addValue(const QString& key, const T& val) {
        assert(!key.isEmpty());
        assert(!d.keys().contains(key));
        const QVariant& var = qVariantFromValue<T>(val);
        d[key] = var;
        return true;
    }

private:
    QMap<QString, QVariant> d;
};

} //namespace

#endif
