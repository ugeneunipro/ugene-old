include (gtest.pri)
# Input
HEADERS += src/gtest/gtest-death-test.h \
           src/gtest/gtest-message.h \
           src/gtest/gtest-param-test.h \
           src/gtest/gtest-printers.h \
           src/gtest/gtest-spi.h \
           src/gtest/gtest-test-part.h \
           src/gtest/gtest-typed-test.h \
           src/gtest/gtest.h \
           src/gtest/gtest_pred_impl.h \
           src/gtest/gtest_prod.h \
           src/src/gtest-internal-inl.h \
           src/test/production.h \
           src/gtest/internal/gtest-death-test-internal.h \
           src/gtest/internal/gtest-filepath.h \
           src/gtest/internal/gtest-internal.h \
           src/gtest/internal/gtest-linked_ptr.h \
           src/gtest/internal/gtest-param-util-generated.h \
           src/gtest/internal/gtest-param-util.h \
           src/gtest/internal/gtest-port.h \
           src/gtest/internal/gtest-string.h \
           src/gtest/internal/gtest-tuple.h \
           src/gtest/internal/gtest-type-util.h

SOURCES += src/gtest-all.cc \
           src/gtest_main.cc \
           src/gtest_prod_test.cc \
           src/gtest_unittest.cc \
           src/src/gtest-death-test.cc \
           src/src/gtest-filepath.cc \
           src/src/gtest-port.cc \
           src/src/gtest-printers.cc \
           src/src/gtest-test-part.cc \
           src/src/gtest-typed-test.cc \
           src/src/gtest.cc \
           src/test/production.cc
