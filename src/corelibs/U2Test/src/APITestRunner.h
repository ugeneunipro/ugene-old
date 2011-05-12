#ifndef _U2_API_TEST_RUNNER_H_
#define _U2_API_TEST_RUNNER_H_


namespace U2 {

class APITestRunner {
public:
    virtual void launchTests(const QStringList& suiteUrls)=0;
};

class APITestBase {
public:
    APITestBase() : runner(NULL) {}
    //~APITestBase() { delete runner; }

    void setRunner(APITestRunner* _runner) {
        assert(runner==NULL || _runner==NULL);
        runner = _runner;
    }

    APITestRunner* getRunner() const {
        assert(runner!=NULL);
        return runner;
    }

private:
    APITestRunner* runner;
};

} //namespace

#endif
