/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#include "CrashHandler.h"
#include "TaskSchedulerImpl.h"

#include <string>

#include <U2Core/AppContext.h>
#include <U2Core/Task.h>
#include <U2Core/Log.h>
#include <U2Core/LogCache.h>
#include <U2Core/Timer.h>
#include <U2Core/Version.h>


#define MAX_CRASH_MESSAGES_TO_SEND 70

#if defined(Q_OS_WIN32)
  #ifdef UGENE_X86_64 //see http://social.msdn.microsoft.com/Forums/en-US/vcgeneral/thread/4dc15026-884c-4f8a-8435-09d0111d708d/
    extern "C"
    {
        void rollbackStack();
    }
  #endif
#endif

namespace U2 {

bool CrashHandler::isEnabled() {

    static QString disableEnvString = ENV_USE_CRASHHANDLER+QString("=0");
    static bool disableCrashHandler = QProcess::systemEnvironment().contains(disableEnvString);
    if (disableCrashHandler) {
        return false;
    }

    static QString enableEnvString = ENV_USE_CRASHHANDLER+QString("=1");
    static bool enableCrashHandler = QProcess::systemEnvironment().contains(enableEnvString);
    if (enableCrashHandler) {
        return true;
    }

#ifdef _DEBUG // no crash handler mode in debug build by default
    bool defaultValue = false;
#else
    bool defaultValue = true;
#endif
    return defaultValue;
}

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

    if (pExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_STACK_OVERFLOW) {
#if defined(Q_OS_WIN32)
  #ifdef UGENE_X86 //see http://social.msdn.microsoft.com/Forums/en-US/vcgeneral/thread/4dc15026-884c-4f8a-8435-09d0111d708d/
        _asm add esp, 10240; //roll back stack and current frame pointer
  #else
        rollbackStack();//TODO:need hack for x86_64
  #endif
#endif
        QString anotherError = QString::number(EXCEPTION_STACK_OVERFLOW, 16) + "|Stack overflow"; //previous error was dropped in the stack unwinding 
        st.ShowCallstack(GetCurrentThread(), pExceptionInfo->ContextRecord);
        runMonitorProcess(anotherError);
        return EXCEPTION_EXECUTE_HANDLER;
    }
    else {
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

        st.ShowCallstack(GetCurrentThread(), pExceptionInfo->ContextRecord);

        runMonitorProcess(QString::number(pExceptionInfo->ExceptionRecord->ExceptionCode, 16) + "|" + error + "::" + QString::number((int)pExceptionInfo->ExceptionRecord->ExceptionAddress, 16));
        return EXCEPTION_EXECUTE_HANDLER;
    }
}
#else

    struct sigaction CrashHandler::sa;

    void CrashHandler::signalHandler(int signo, siginfo_t *siginfo, void*) {
        sigprocmask(SIG_UNBLOCK, &sa.sa_mask, NULL);
        std::string exception;

        switch(signo) {
        case SIGBUS:
            exception = "Access to undefined portion of memory object";
            switch (siginfo->si_code) {
            case BUS_ADRALN:
                exception += ": invalid address alignment.";
                break;
            case BUS_ADRERR:
                exception += ": non-existent physical address.";
                break;
            case BUS_OBJERR:
                exception += ": object-specific hardware error.";
                break;
            }
            break;

        case SIGFPE:
            exception = "Erroneous arithmetic operation";
            switch (siginfo->si_code) {
            case FPE_INTDIV:
                exception += ": integer divide-by-zero.";
                break;
            case FPE_INTOVF:
                exception += ": integer overflow.";
                break;
            case FPE_FLTDIV:
                exception += ": floating point divide-by-zero.";
                break;
            case FPE_FLTOVF:
                exception += ": floating point overflow.";
                break;
            case FPE_FLTUND:
                exception += ": floating point underflow.";
                break;
            case FPE_FLTRES:
                exception += ": floating point inexact result.";
                break;
            case FPE_FLTINV:
                exception += ": invalid floating point operation.";
                break;
            case FPE_FLTSUB:
                exception += ": subscript out of range.";
                break;
            }
            break;

        case SIGILL:
            exception = "Illegal instruction";
            switch (siginfo->si_code) {
            case ILL_ILLOPC:
                exception += ": illegal opcode.";
                break;
            case ILL_ILLOPN:
                exception += ": illegal operand.";
                break;
            case ILL_ILLADR:
                exception += ": illegal addressing mode.";
                break;
            case ILL_ILLTRP:
                exception += ": illegal trap.";
                break;
            case ILL_PRVOPC:
                exception += ": privileged opcode.";
                break;
            case ILL_PRVREG:
                exception += ": privileged register.";
                break;
            case ILL_COPROC:
                exception += ": coprocessor error.";
                break;
            case ILL_BADSTK:
                exception += ": internal stack error.";
                break;
            }
            break;

        case SIGSEGV:
            exception = "Segmentation fault";
            switch (siginfo->si_code) {
            case SEGV_MAPERR:
                exception += ": address not mapped.";
                break;
            case SEGV_ACCERR:
                exception += ": invalid permissions.";
                break;
            }
            break;

        case SIGSYS:
            exception = "Bad syscall";
            break;

        case SIGXCPU:
            exception = "CPU time limit exceeded";
            break;

        case SIGXFSZ:
            exception = "File size limit exceeded";
            break;

        case SIGABRT:
            exception = "Program has been aborted";
            break;

        default: return;
        }

        runMonitorProcess(QString::number(signo) + "|" + exception.c_str());
    }
#endif

char* CrashHandler::buffer = NULL;
LogCache* CrashHandler::crashLogCache = NULL;

void CrashHandler::preallocateReservedSpace() {
    assert(buffer == NULL);
    buffer = new char[1024*1024];
}

void CrashHandler::releaseReserve() {
    delete[] buffer;
    buffer = NULL;
    delete crashLogCache;
    crashLogCache = NULL;
}

void CrashHandler::setupHandler() {
    // setup cached messages first
    assert(crashLogCache == NULL);
    crashLogCache = new LogCache();
    crashLogCache->filter.filters.append(LogFilterItem(ULOG_CAT_TASKS, LogLevel_TRACE));
    crashLogCache->filter.filters.append(LogFilterItem(ULOG_CAT_CORE_SERVICES, LogLevel_TRACE));
    crashLogCache->filter.filters.append(LogFilterItem(ULOG_CAT_IO, LogLevel_TRACE));
    crashLogCache->filter.filters.append(LogFilterItem(ULOG_CAT_USER_INTERFACE, LogLevel_ERROR));
    crashLogCache->filter.filters.append(LogFilterItem(ULOG_CAT_ALGORITHM, LogLevel_TRACE));
    crashLogCache->filter.filters.append(LogFilterItem(ULOG_CAT_CONSOLE, LogLevel_ERROR));
    crashLogCache->filter.filters.append(LogFilterItem(ULOG_CAT_CORE_SERVICES, LogLevel_DETAILS));
    crashLogCache->filter.filters.append(LogFilterItem(ULOG_CAT_TASKS, LogLevel_DETAILS));

#if defined( Q_OS_WIN )
    addHandlerFunc = NULL;
    removeHandlerFunc = NULL;
    HMODULE h = LoadLibrary(TEXT("kernel32.dll"));
    if (h != NULL) {
        FARPROC func = GetProcAddress(h, "AddVectoredExceptionHandler");
        if(func != NULL) {
            addHandlerFunc = (addExceptionHandler)func;
        }

        func = GetProcAddress(h, "RemoveVectoredExceptionHandler");
        if(func != NULL) {
            removeHandlerFunc = (removeExceptionHandler)func;
        }
    }
    if (addHandlerFunc != NULL) {
        addHandlerFunc(1, CrashHandlerFunc);
    }
    //handler = AddVectoredExceptionHandler(1, CrashHandlerFunc);

#else
#ifndef Q_OS_MAC // if separate stack has been used in MAC OS as under Linux, then backtrace() will not work
#define SA_FLAGS (SA_ONSTACK | SA_SIGINFO)

    stack_t sigstk;
    sigstk.ss_sp = malloc(SIGSTKSZ * 2);
    sigstk.ss_size = SIGSTKSZ * 2;
    sigstk.ss_flags = 0;
    if (sigaltstack(&sigstk,0) < 0) {
        perror("sigaltstack");
    }

#else
#define SA_FLAGS SA_SIGINFO
#endif // Q_OS_MAK

    //struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sigemptyset(&sa.sa_mask);
    int kExceptionSignals[9] = {SIGBUS, SIGFPE, SIGILL, SIGSEGV, SIGSYS, SIGXCPU, SIGXFSZ, SIGABRT, -1};
    for (unsigned i = 0; kExceptionSignals[i] != -1; ++i) {
        sigaddset(&sa.sa_mask, kExceptionSignals[i]);
    }

    sa.sa_sigaction = signalHandler;
    sa.sa_flags = (SA_FLAGS);
    for (unsigned i = 0; kExceptionSignals[i] != -1; ++i) {
        sigaction(kExceptionSignals[i], &sa, NULL);
    }

#endif
}

void CrashHandler::runMonitorProcess(const QString &exceptionType)
{
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

    QString reportText = exceptionType + "|";

    reportText += Version::appVersion().text;
    reportText += "|";

    QString activeWindow = AppContext::getActiveWindowName();
    if(activeWindow.isEmpty()) {
        reportText += "None|";
    } else {
        reportText += activeWindow + "|";
    }

    QList<LogMessage*> logMessages = crashLogCache == NULL ? QList<LogMessage*>() : crashLogCache->messages;
    QString messageLog;
    if (!logMessages.isEmpty()) {
        QList<LogMessage*>::iterator it;
        int i;
        for(i = 0, it = --logMessages.end(); i <= MAX_CRASH_MESSAGES_TO_SEND && it!= logMessages.begin(); i++, it--) {
            LogMessage* msg = *it;
            messageLog.prepend("[" + GTimer::createDateTime(msg->time).toString("hh:mm:ss.zzz") + "] " + "[" + msg->categories.first() + "] " + msg->text + "\n");
        }
    } else {
        messageLog += "None";
    }
    reportText += messageLog + " | ";

    QString taskList;
    TaskScheduler *ts = AppContext::getTaskScheduler();
    QList<Task* > topTasks = ts != NULL ? ts->getTopLevelTasks() : QList<Task*>();
    foreach(Task *t, topTasks) {
        if(t->getState() != Task::State_Finished) {
            QString state;
            if (t->getState() == Task::State_Running) {
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
    reportText += taskList;
    if (taskList.isEmpty()) {
        reportText += "None";
    }
    
#if defined (Q_OS_WIN)
    reportText += "|" + st.getBuffer();
#else
    reportText += "|None";
#endif

    static QMutex mutex;
    QMutexLocker lock(&mutex);

    QProcess::startDetached(path, QStringList() << reportText.toUtf8().toBase64());
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
