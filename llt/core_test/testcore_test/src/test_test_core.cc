#include "gtest/gtest.h"
#include "mockcpp/mokc.h"

extern "C" {
#include "test_core.h"
int test_core_run_op_proc(test_core_op_info_s *op_info, test_core_s *core);
}

class TestCore : public testing::Test {
protected:
    virtual void SetUp() {}

    virtual void TearDown()
    {
        GlobalMockObject::verify();
    }
};

TEST_F(TestCore, op_test_NEW)
{
    EXPECT_TRUE(true);
}