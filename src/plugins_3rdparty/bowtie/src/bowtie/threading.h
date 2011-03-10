#ifndef THREADING_H_
#define THREADING_H_

#include <iostream>
#include "spinlock.h"
#include <QtCore/QMutex>

// Note that USE_SPINLOCK trumps BOWTIE_PTHREADS
#define BOWTIE_PTHREADS 1

#ifdef BOWTIE_PTHREADS
#define pthread_t int
#endif

#ifdef USE_SPINLOCK
#  include "spinlock.h"
#  define MUTEX_T SpinLock
#  define MUTEX_INIT(l)
#  define MUTEX_LOCK(l) (l).Enter()
#  define MUTEX_UNLOCK(l) (l).Leave()
#else
#  ifdef BOWTIE_PTHREADS
	struct QMutextContainer {
		struct Data { Data() {count = 0; m = new QMutex(); } QMutex* m; int count; };
		QMutextContainer(): d(NULL) {}
		QMutextContainer(const QMutextContainer &c) { if(c.d==NULL) { d=NULL; return; } d = c.d; d->count++; }
		QMutextContainer &operator = (const QMutextContainer &c) { if(c.d==NULL) { d=NULL; return *this; } d = c.d; d->count++; return *this; }
		~QMutextContainer() { if(d==NULL) return; d->count--; if(d->count == 0) delete d; d = NULL; }
		Data* d;
		void init() {d = new Data(); d->count++;}
		void lock() {d->m->lock();}
		void unlock() {d->m->unlock();}
	};
#    define MUTEX_T QMutextContainer
#    define MUTEX_INIT(l) (l).init()
#    define MUTEX_LOCK(l) (l).lock()
#    define MUTEX_UNLOCK(l) (l).unlock()
#  else
#    define MUTEX_T int
#    define MUTEX_INIT(l) l = 0
#    define MUTEX_LOCK(l) l = 1
#    define MUTEX_UNLOCK(l) l = 0
#  endif /* UGENE_THREADS */
#endif /* USE_SPINLOCK */

#ifdef BOWTIE_PTHREADS
static inline void exitThread(void* v) {
	throw 2;
}
extern void joinThread(pthread_t th);

extern void createThread(pthread_t* th,
								void *(*start_routine) (void *),
								void *arg);
#endif

/**
 * Wrap a lock; obtain lock upon construction, release upon destruction.
 */
class ThreadSafe {
public:
	ThreadSafe(MUTEX_T* lock) {
		lock_ = lock;
		MUTEX_LOCK(*lock_);
	}
	~ThreadSafe() { MUTEX_UNLOCK(*lock_); }
private:
	MUTEX_T *lock_;
};

#endif
