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

} //namespace

#endif
