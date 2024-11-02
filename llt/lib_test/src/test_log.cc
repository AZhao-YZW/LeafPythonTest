#include "gtest/gtest.h"
#include "mockcpp/mokc.h"

extern "C" {
#include "error_code.h"
#include "leafpy_cfg.h"
#include "log.h"
#include "type.h"
}

class TestLog : public testing::Test {
protected:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
        GlobalMockObject::verify();
    }
};

TEST_F(TestLog, core_printf_const_string)
{
    testing::internal::CaptureStdout();
    core_printf("test: constant string");
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, "[core] test: constant string");
}

TEST_F(TestLog, core_printf_format_string)
{
    testing::internal::CaptureStdout();
    core_printf("test: format string[%s]", "this is format string");
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, "[core] test: format string[this is format string]");
}

TEST_F(TestLog, core_printf_format_number_u)
{
    testing::internal::CaptureStdout();
    core_printf("test: format number[%u, %u, %u, %u, %u, %u, %u]",
        0, 1, 255, 256, 65535, 65536, 4294967295);
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, "[core] test: format number"
        "[0, 1, 255, 256, 65535, 65536, 4294967295]");
}

TEST_F(TestLog, core_printf_format_number_lu)
{
    testing::internal::CaptureStdout();
    core_printf("test: format number[%lu, %lu, %lu, %lu, %lu, %lu, %lu]",
        0, 1, 255, 256, 65535, 65536, 4294967295);
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, "[core] test: format number"
        "[0, 1, 255, 256, 65535, 65536, 4294967295]");
}

TEST_F(TestLog, core_printf_format_number_llu)
{
    testing::internal::CaptureStdout();
    core_printf("test: format number[%llu, %llu, %llu, %llu, %llu, %llu, %llu, %llu, %llu]",
        0ULL, 1ULL, 255ULL, 256ULL, 65535ULL, 65536ULL, 4294967295ULL, 4294967296ULL,
        0xFFFFFFFFFFFFFFFF);
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, "[core] test: format number"
        "[0, 1, 255, 256, 65535, 65536, 4294967295, 4294967296, 18446744073709551615]");
}

TEST_F(TestLog, core_printf_format_number_d)
{
    testing::internal::CaptureStdout();
    core_printf("test: format number[%d, %d, %d, %d, %d, %d, %d, %d, %d]",
        0, 1, -1, 127, -128, 32767, -32768, 2147483647, -2147483648);
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, "[core] test: format number"
        "[0, 1, -1, 127, -128, 32767, -32768, 2147483647, -2147483648]");
}

TEST_F(TestLog, core_printf_format_number_ld)
{
    testing::internal::CaptureStdout();
    core_printf("test: format number[%ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld]",
        0, 1, -1, 127, -128, 32767, -32768, 2147483647, -2147483648);
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, "[core] test: format number"
        "[0, 1, -1, 127, -128, 32767, -32768, 2147483647, -2147483648]");
}

TEST_F(TestLog, core_printf_format_number_lld)
{
    testing::internal::CaptureStdout();
    core_printf("test: format number[%lld, %lld, %lld, %lld, %lld, %lld, %lld, %lld, %lld, %lld]",
        0LL, 1LL, -1LL, 127LL, -128LL, 32767LL, -32768LL, 2147483647LL, -2147483648LL,
        9223372036854775807LL);
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, "[core] test: format number"
        "[0, 1, -1, 127, -128, 32767, -32768, 2147483647, -2147483648, 9223372036854775807]");
}

TEST_F(TestLog, corename_printf_name_test)
{
    testing::internal::CaptureStdout();
    corename_printf(LEAFPY_CORE_NAME_LEAFPY, "test: core name test");
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, "[leafpy] test: core name test");

    testing::internal::CaptureStdout();
    corename_printf(LEAFPY_CORE_NAME_TESTCORE, "test: core name test");
    output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, "[testcore] test: core name test");
}