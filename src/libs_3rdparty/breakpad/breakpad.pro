include (breakpad.pri)

HEADERS += src/client/minidump_file_writer.h \
           src/client/minidump_file_writer-inl.h \
           src/common/convert_UTF.h \
           src/common/md5.h \
           src/common/memory.h \
           src/common/using_std_string.h \
           src/common/scoped_ptr.h \
           src/common/string_conversion.h \
           src/google_breakpad/common/breakpad_types.h \
           src/google_breakpad/common/minidump_cpu_amd64.h \
           src/google_breakpad/common/minidump_cpu_arm.h \
           src/google_breakpad/common/minidump_cpu_arm64.h \
           src/google_breakpad/common/minidump_cpu_mips.h \
           src/google_breakpad/common/minidump_cpu_ppc.h \
           src/google_breakpad/common/minidump_cpu_ppc64.h \
           src/google_breakpad/common/minidump_cpu_sparc.h \
           src/google_breakpad/common/minidump_cpu_x86.h \
           src/google_breakpad/common/minidump_exception_linux.h \
           src/google_breakpad/common/minidump_exception_mac.h \
           src/google_breakpad/common/minidump_exception_ps3.h \
           src/google_breakpad/common/minidump_exception_solaris.h \
           src/google_breakpad/common/minidump_exception_win32.h \
           src/google_breakpad/common/minidump_format.h \
           src/google_breakpad/common/minidump_size.h

SOURCES += src/client/minidump_file_writer.cc \
           src/common/convert_UTF.c \
           src/common/md5.cc \
           src/common/string_conversion.cc

macx {
HEADERS += src/client/mac/crash_generation/client_info.h \
           src/client/mac/crash_generation/crash_generation_client.h \
           src/client/mac/crash_generation/crash_generation_server.h \
           src/client/mac/handler/breakpad_nlist_64.h \
           src/client/mac/handler/dynamic_images.h \
           src/client/mac/handler/exception_handler.h \
           src/client/mac/handler/mach_vm_compat.h \
           src/client/mac/handler/minidump_generator.h \
           src/client/mac/handler/protected_memory_allocator.h \
           src/client/mac/handler/ucontext_compat.h \
           src/common/linux/linux_libc_support.h \
           src/common/mac/bootstrap_compat.h \
           src/common/mac/byteswap.h \
           src/common/mac/file_id.h \
           src/common/mac/macho_id.h \
           src/common/mac/macho_utilities.h \
           src/common/mac/macho_walker.h \
           src/common/mac/scoped_task_suspend-inl.h \
           src/common/mac/string_utilities.h

SOURCES += src/client/mac/crash_generation/crash_generation_client.cc \
           src/client/mac/crash_generation/crash_generation_server.cc \
           src/client/mac/handler/breakpad_nlist_64.cc \
           src/client/mac/handler/dynamic_images.cc \
           src/client/mac/handler/exception_handler.cc \
           src/client/mac/handler/protected_memory_allocator.cc \
           src/common/linux/linux_libc_support.cc \
           src/common/mac/bootstrap_compat.cc \
           src/common/mac/file_id.cc \
           src/common/mac/macho_id.cc \
           src/common/mac/macho_utilities.cc \
           src/common/mac/macho_walker.cc \
           src/common/mac/string_utilities.cc

OBJECTIVE_HEADERS += src/common/mac/MachIPC.h

OBJECTIVE_SOURCES += src/client/mac/handler/minidump_generator.cc \
                     src/common/mac/MachIPC.mm
}
