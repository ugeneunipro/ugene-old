#ifndef _STACK_WALKER_H_
#define _STACK_WALKER_H_

#include <QtCore/qglobal.h>

#if defined(USE_CRASHHANDLER)
#if defined (Q_OS_WIN)

#include <windows.h>
#include <wtypes.h>

#include <QtCore/QString>
#include <QtCore/QMap>

namespace U2 {


class StackWalkerInternal;  // forward
class StackWalker {
public:
    enum StackWalkOptions {
        // No addition info will be retrived 
        // (only the address is available)
        RetrieveNone = 0,
        
        // Try to get the symbol-name
        RetrieveSymbol = 1,
        
        // Try to get the line for this symbol
        RetrieveLine = 2,
        
        // Try to retrieve the module-infos
        RetrieveModuleInfo = 4,
        
        // Also retrieve the version for the DLL/EXE
        RetrieveFileVersion = 8,
        
        // Contains all the above
        RetrieveVerbose = 0xF,
        
        // Generate a "good" symbol-search-path
        SymBuildPath = 0x10,
        
        // Also use the public Microsoft-Symbol-Server
        SymUseSymSrv = 0x20,
        
        // Contains all the above "Sym"-options
        SymAll = 0x30,
        
        // Contains all options (default)
        OptionsAll = 0x3F
    };

    StackWalker(
    int options = OptionsAll, // 'int' is by design, to combine the enum-flags
    LPCSTR szSymPath = NULL, 
    DWORD dwProcessId = GetCurrentProcessId(), 
    HANDLE hProcess = GetCurrentProcess()
    );
    StackWalker(DWORD dwProcessId, HANDLE hProcess);
    virtual ~StackWalker();

    typedef bool (__stdcall *PReadProcessMemoryRoutine)(
    HANDLE      hProcess,
    DWORD64     qwBaseAddress,
    PVOID       lpBuffer,
    DWORD       nSize,
    LPDWORD     lpNumberOfBytesRead,
    LPVOID      pUserData  // optional data, which was passed in "ShowCallstack"
    );

    BOOL LoadModules();

    bool ShowCallstack(
    HANDLE hThread = GetCurrentThread(), 
    const CONTEXT *context = NULL, 
    PReadProcessMemoryRoutine readMemoryFunction = NULL,
    LPVOID pUserData = NULL  // optional to identify some data in the 'readMemoryFunction'-callback
    );

    QString getBuffer() const {return buffer;}

protected:
	enum { STACKWALK_MAX_NAMELEN = 1024 }; // max name length for found symbols

protected:
  // Entry for each Callstack-Entry
    typedef struct CallstackEntry
    {
        DWORD64 offset;  // if 0, we have no valid entry
        CHAR name[STACKWALK_MAX_NAMELEN];
        CHAR undName[STACKWALK_MAX_NAMELEN];
        CHAR undFullName[STACKWALK_MAX_NAMELEN];
        DWORD64 offsetFromSmybol;
        DWORD offsetFromLine;
        DWORD lineNumber;
        CHAR lineFileName[STACKWALK_MAX_NAMELEN];
        DWORD symType;
        LPCSTR symTypeString;
        CHAR moduleName[STACKWALK_MAX_NAMELEN];
        DWORD64 baseOfImage;
        CHAR loadedImageName[STACKWALK_MAX_NAMELEN];
    } CallstackEntry;

    typedef enum CallstackEntryType {firstEntry, nextEntry, lastEntry};

    virtual void OnSymInit(LPCSTR szSearchPath, DWORD symOptions, LPCSTR szUserName);
    virtual void OnLoadModule(LPCSTR img, LPCSTR mod, DWORD64 baseAddr, DWORD size, DWORD result, LPCSTR symType, LPCSTR pdbName, ULONGLONG fileVersion);
    virtual void OnCallstackEntry(CallstackEntryType eType, CallstackEntry &entry);
    virtual void OnDbgHelpErr(LPCSTR szFuncName, DWORD gle, DWORD64 addr);
    virtual void OnOutput(LPCSTR szText);

    StackWalkerInternal *m_sw;
    HANDLE m_hProcess;
    DWORD m_dwProcessId;
    BOOL m_modulesLoaded;
    LPSTR m_szSymPath;

    int m_options;

    static BOOL __stdcall myReadProcMem(HANDLE hProcess, DWORD64 qwBaseAddress, PVOID lpBuffer, DWORD nSize, LPDWORD lpNumberOfBytesRead);

    QString buffer;
    QMap<LPVOID, QString> libraries;
    friend StackWalkerInternal;
    QString stackTrace;
};

}

#endif
#endif

#endif
