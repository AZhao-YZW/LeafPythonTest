#include "gtest/gtest.h"
#include "mockcpp/mokc.h"
#include <cstring>

extern "C" {
#include "error_code.h"
#include "mm.h"
}

class TestMM : public testing::Test {
protected:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
        GlobalMockObject::verify();
    }
};

TEST_F(TestMM, test_mm_malloc_free)
{
    void *arr;

    arr = mm_malloc(0);
    EXPECT_NE(arr, (void *)NULL);
    mm_free(arr);

    arr = mm_malloc(1);
    EXPECT_NE(arr, (void *)NULL);
    mm_free(arr);

    arr = mm_malloc(10);
    EXPECT_NE(arr, (void *)NULL);
    mm_free(arr);

    arr = mm_malloc(100);
    EXPECT_NE(arr, (void *)NULL);
    mm_free(arr);

    arr = mm_malloc(1000);
    EXPECT_NE(arr, (void *)NULL);
    mm_free(arr);

    arr = mm_malloc(0xFFFF);
    EXPECT_NE(arr, (void *)NULL);
    mm_free(arr);
}

TEST_F(TestMM, test_mm_memset_s)
{
    char *exp_arr = (char *)mm_malloc(sizeof(char) * 101);
    char *arr = (char *)mm_malloc(sizeof(char) * 101);

    std::memset(exp_arr, 'A', 100);
    exp_arr[100] = '\0';
    mm_memset_s(arr, 100, 'A', 100);
    arr[100] = '\0';

    EXPECT_STREQ(exp_arr, arr);
}