#ifndef _U2_API_TEST_RUNNER_H_
#define _U2_API_TEST_RUNNER_H_


namespace U2 {

class GTestEnvironment;

class APITestEnvRegistry {
public:
    const GTestEnvironment* getEnvironment(const QString& name) const { return registry.value(name); }
    void setEnvironment(const QString& name, const GTestEnvironment* env) { registry[name] = env; }
    void removeEnvironment(const QString& name) { registry.remove(name); }
private:
    QMap<QString, const GTestEnvironment*> registry;
};

} //namespace

#endif
