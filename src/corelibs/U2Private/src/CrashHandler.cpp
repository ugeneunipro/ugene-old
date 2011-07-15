/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#if defined(USE_CRASHHANDLER)

#include "CrashHandler.h"

#include "TaskSchedulerImpl.h"
#include <U2Core/Version.h>

#include <U2Core/Timer.h>

namespace U2 {


#if defined( Q_OS_WIN )

#include <intrin.h>
#include <dbghelp.h>

PVOID CrashHandler::handler = NULL;
PVOID CrashHandler::handler2 = NULL;

StackWalker CrashHandler::st;

addExceptionHandler CrashHandler::addHandlerFunc = NULL;
removeExceptionHandler CrashHandler::removeHandlerFunc = NULL;


LONG CrashHandler::CrashHandlerFuncThird(PEXCEPTION_POINTERS pExceptionInfo ) {
    exit(1);
}

LONG CrashHandler::CrashHandlerFuncSecond(PEXCEPTION_POINTERS pExceptionInfo ) {
    if(removeHandlerFunc != NULL) {
        removeHandlerFunc(handler2);
    }
    if(addHandlerFunc != NULL) {
        addHandlerFunc(1, CrashHandlerFuncThird);
    }
    QString path = QCoreApplication::applicationDirPath() + "/ugenem.exe";
    static QMutex mutex;
    QMutexLocker lock(&mutex);
    QProcess::startDetached(path, QStringList());
    exit(1);
}

LONG CrashHandler::CrashHandlerFunc(PEXCEPTION_POINTERS pExceptionInfo ) {
    QString error;
    switch(pExceptionInfo->ExceptionRecord->ExceptionCode) {
        case EXCEPTION_ACCESS_VIOLATION: error = "Access violation";
            break;
        case EXCEPTION_DATATYPE_MISALIGNMENT: error = "Data type misalignment";
            break;
        case EXCEPTION_BREAKPOINT: error = "Break point";
            break;
        case EXCEPTION_SINGLE_STEP: error = "Single step";
            break;
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: error = "Array bounds exceeded";
            break;
        case EXCEPTION_FLT_DENORMAL_OPERAND: error = "Float denormal operand";
            break;
        case EXCEPTION_FLT_DIVIDE_BY_ZERO: error = "Float divide by zero";
            break;
        case EXCEPTION_FLT_INEXACT_RESULT: error = "Float inexact result";
            break;
        case EXCEPTION_FLT_INVALID_OPERATION: error = "Float invalid operation";
            break;
        case EXCEPTION_FLT_OVERFLOW: error = "Float overflow";
            break;
        case EXCEPTION_FLT_STACK_CHECK: error = "Float check stack";
            break;
        case EXCEPTION_FLT_UNDERFLOW: error = "Float underflow";
            break;
        case EXCEPTION_INT_DIVIDE_BY_ZERO: error = "Int divide by zero";
            break;
        case EXCEPTION_INT_OVERFLOW: error = "Int overflow";
            break;
        case EXCEPTION_PRIV_INSTRUCTION: error = "Privileged instruction";
            break;
        case EXCEPTION_IN_PAGE_ERROR: error = "In page error";
            break;
        case EXCEPTION_ILLEGAL_INSTRUCTION: error = "Illegal instruction";
            break;
        case EXCEPTION_NONCONTINUABLE_EXCEPTION: error = "Noncontinuable exception";
            break;
        case EXCEPTION_STACK_OVERFLOW:  break;
        case EXCEPTION_INVALID_DISPOSITION: error = "Invalid disposition";
            break;
        case EXCEPTION_GUARD_PAGE: error = "Guard page violation";
            break;
        case EXCEPTION_INVALID_HANDLE: error = "Invalid handle";
            break;
        /*case EXCEPTION_POSSIBLE_DEADLOCK: error = "Possible deadlock";
            break;*/
        case CONTROL_C_EXIT: error = "Control C exit";
            break;
        default: /*error = "Unknown exception";*/ return EXCEPTION_EXECUTE_HANDLER;
    }
    if(removeHandlerFunc != NULL) {
        removeHandlerFunc(handler);
    }    
    //RemoveVectoredExceptionHandler(handler);
    //handler2 = AddVectoredExceptionHandler(1, CrashHandlerFuncSecond);
    if(pExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_STACK_OVERFLOW) {
#if defined(Q_OS_WIN32)
        _asm add esp, 10240; //roll back stack and current frame pointer
#endif
        /*WORD *sp = (WORD*)_AddressOfReturnAddress();
        WORD newSp = *sp + 10240;
        __movsw(sp, &newSp, 1);*/

        QString anotherError = QString::number(EXCEPTION_STACK_OVERFLOW, 16) + "|Stack overflow"; //previous error was dropped in the stack unwinding 
        st.ShowCallstack(GetCurrentThread(), pExceptionInfo->ContextRecord);
        runMonitorProcess(anotherError);
    }

    st.ShowCallstack(GetCurrentThread(), pExceptionInfo->ContextRecord);

    runMonitorProcess(QString::number(pExceptionInfo->ExceptionRecord->ExceptionCode, 16) + "|" + error + "::" + QString::number((int)pExceptionInfo->ExceptionRecord->ExceptionAddress, 16));
    return EXCEPTION_EXECUTE_HANDLER;
}

#else
    struct sigaction CrashHandler::sa;

    void CrashHandler::signalHandler(int signo, siginfo_t*, void*) {
        sigprocmask(SIG_UNBLOCK, &sa.sa_mask, NULL);
        QString exception;
        switch(signo) {
        case SIGBUS: exception = "Access to undefined portion of memory object";
            break;
        case SIGFPE: exception = "Erroneous arithmetic operation";
            break;
        case SIGILL: exception = "Illegal instruction";
            break;
        case SIGSEGV: exception = "Segmentation fault";
            break;
        case SIGSYS: exception = "Bad syscall";
            break;
        case SIGXCPU: exception = "CPU time limit exceeded";
            break;
        case SIGXFSZ: exception = "File size limit exceeded";
            break;
        case SIGABRT: exception = "Program has been aborted";
            break;
        default: return;
        }
        runMonitorProcess(QString::number(signo) + "|" + exception);
    }
#endif

char  *CrashHandler::buffer = NULL;

void CrashHandler::allocateReserve() {
    buffer = new char[1024*1024];
}

void CrashHandler::releaseReserve() {
    delete []buffer;
}

void CrashHandler::setupHandler() {
#if defined( Q_OS_WIN )
    addHandlerFunc = NULL;
    removeHandlerFunc = NULL;
    HMODULE h = LoadLibrary(TEXT("kernel32.dll"));
    if(h != NULL) {
        FARPROC func = GetProcAddress(h, "AddVectoredExceptionHandler");
        if(func != NULL) {
            addHandlerFunc = (addExceptionHandler)func;
        }

        func = GetProcAddress(h, "RemoveVectoredExceptionHandler");
        if(func != NULL) {
            removeHandlerFunc = (removeExceptionHandler)func;
        }
    }
    if(addHandlerFunc != NULL) {
        addHandlerFunc(1, CrashHandlerFunc);
    }
    //handler = AddVectoredExceptionHandler(1, CrashHandlerFunc);
    
#elif defined( Q_OS_MAC)
    return; //TODO: implement crash hander for MAC OS
#else
    stack_t sigstk;
    sigstk.ss_sp = malloc(SIGSTKSZ);
    sigstk.ss_size = SIGSTKSZ;
    sigstk.ss_flags = 0;
    if (sigaltstack(&sigstk,0) < 0) {
        perror("sigaltstack");
    }

    //struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sigemptyset(&sa.sa_mask);
    int kExceptionSignals[9] = {SIGBUS, SIGFPE, SIGILL, SIGSEGV, SIGSYS, SIGXCPU, SIGXFSZ, SIGABRT, -1};
    for (unsigned i = 0; kExceptionSignals[i] != -1; ++i) {
        sigaddset(&sa.sa_mask, kExceptionSignals[i]);
    }

    sa.sa_sigaction = signalHandler;
    sa.sa_flags = SA_ONSTACK | SA_SIGINFO;
    for (unsigned i = 0; kExceptionSignals[i] != -1; ++i) {
        sigaction(kExceptionSignals[i], &sa, NULL);
    }

#endif
}


void CrashHandler::runMonitorProcess(const QString &exceptionType) {
    QString path = QCoreApplication::applicationDirPath() + "/ugenem";

#ifndef Q_OS_WIN
    char pid_buf[30];
    sprintf(pid_buf, "%d", getpid());
    char name_buf[512];
    name_buf[readlink(path.toAscii().data(), name_buf, 511)]=0;
    FILE *fp;
    fp = freopen ("/tmp/UGENEstacktrace.txt","w+",stdout);
    void * stackTrace[1024];
    int frames = backtrace(stackTrace, 1024);
    backtrace_symbols_fd(stackTrace, frames, STDOUT_FILENO);
    fclose(fp);
#endif

    QString message = exceptionType + "|";

    message += Version::ugeneVersion().text;
    message += "|";

    QString activeWindow = AppContext::getActiveWindowName();
    if(activeWindow.isEmpty()) {
        message += "None|";
    } else {
        message += activeWindow + "|";
    }

    TaskScheduler *ts = AppContext::getTaskScheduler();
    if (ts) {
        QList<LogMessage*> ms = ts->getMessages();
        QString taskLog;
        QList<LogMessage*>::iterator it;
        int i;
        for(i = 0, it = --ms.end(); i <= 70 && it!= ms.begin(); i++, it--) {
            LogMessage* msg = *it;
            taskLog.prepend("[" + GTimer::createDateTime(msg->time).toString("hh:mm:ss.zzz") + "] " + "[" + msg->categories.first() + "] " + msg->text + "\n");
        }
        taskLog += "|";
        message += taskLog;
        QString taskList;
        QList<Task* > topTasks = AppContext::getTaskScheduler()->getTopLevelTasks();
        foreach(Task *t, topTasks) {
            if(t->getState() != Task::State_Finished) {
                QString state;
                if(t->getState() == Task::State_Running) {
                    state = "(Running)";
                } else if(t->getState() == Task::State_New) {
                    state = "(New)";
                } else if(t->getState() == Task::State_Prepared) {
                    state = "(Preparing)";
                }
                QString progress = QString::number(t->getStateInfo().progress);
                taskList.append(t->getTaskName() + "\t" + state + "\t" + progress  + "\n");
                foreach(Task *tt, t->getSubtasks()) {
                    getSubTasks(tt, taskList, 1);
                }
            }
        }
        message += taskList;
        if(taskList.isEmpty()) {
            message += "None";
        }
    } else {
        message += "None|None";
    }

#if defined (Q_OS_WIN)
    message += "|" + st.getBuffer();
#else
    message += "|None";
#endif

    static QMutex mutex;
    QMutexLocker lock(&mutex);

    QProcess::startDetached(path, QStringList() << message.toUtf8().toBase64());
    exit(1);
}

void CrashHandler::getSubTasks(Task *t, QString& list, int lvl) {
    if(t->getState() != Task::State_Finished) {
        QString prefix;
        QString state;
        prefix.fill('-', lvl);
        if(t->getState() == Task::State_Running) {
            state = "(Running)";
        } else if(t->getState() == Task::State_New) {
            state = "(New)";
        } else if(t->getState() == Task::State_Prepared) {
            state = "(Preparing)";
        }
        QString progress = QString::number(t->getStateInfo().progress);
        list.append(prefix + t->getTaskName() + "\t" + state + "\t" + progress  + "\n");
        foreach(Task *tt, t->getSubtasks()) {
            getSubTasks(tt, list, lvl + 1);
        }
    }

}


}

#endif
