#ifndef _TLS_TASK_H_
#define _TLS_TASK_H_

#include <U2Core/Task.h>
#include <QtCore/QThreadStorage>

namespace U2 {
/************************************************************************/
/* Task with task local storage                                         */
/************************************************************************/

class U2CORE_EXPORT TLSContext {
friend class TLSUtils;
public:
    TLSContext(QString _id) : id(_id) {}
private:
    // class unique id
    const QString id;
};

class TLSContextRef {
public:
  TLSContextRef(TLSContext* _ctx) : ctx(_ctx){}
  TLSContext* ctx;
 };

class U2CORE_EXPORT TLSUtils {
public:
  // Gets task local context, assigned to current thread
  static TLSContext* current(QString contextId);

  // Creates TLSContextRef for current thread
  static void bindToTLSContext(TLSContext *ctx);

  // Deletes TLSContextRef for current thread
  static void detachTLSContext();

private:
  static QThreadStorage<TLSContextRef*> tls;
};

class U2CORE_EXPORT TLSTask : public Task {
Q_OBJECT
public:
  TLSTask(const QString& _name, TaskFlags _flags = TaskFlags_FOSCOE, bool deleteContext = true);
  ~TLSTask(); // tls context removed here
  void run();
  void prepare();
protected:

  // Unsafe run. Use this method in derivate classes
  virtual void _run() {}

  // Creates instance of TLSContext. By default invokes in prepare()
  virtual TLSContext* createContextInstance()=0;
  
  TLSContext* taskContext;
  bool deleteContext;
};

} //namespace


#endif // _TLS_TASK_H_
