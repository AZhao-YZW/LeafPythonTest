#include "gtest/gtest.h"
#include "mockcpp/mokc.h"

extern "C" {
#include "error_code.h"
#include "leafpy_cfg.h"
#include "test_vm.h"
}

class TestTestVM : public testing::Test {
protected:
    virtual void SetUp()
    {
        int ret;
        ret = test_vm_init(0, TEST_FRAME_QUEUE_0);
        ASSERT_EQ(ret, EC_OK);
    }

    virtual void TearDown()
    {
        test_vm_free_all();
        test_frame_free_all();
        GlobalMockObject::verify();
    }
};

TEST_F(TestTestVM, vm_add_success)
{
    int ret = test_vm_add(1, TEST_FRAME_QUEUE_1);
    EXPECT_EQ(ret, EC_OK);
}

TEST_F(TestTestVM, vm_add_core_id_exist)
{
    int ret = test_vm_add(0, TEST_FRAME_QUEUE_1);
    EXPECT_EQ(ret, EC_CORE_ID_INVALID);
}

TEST_F(TestTestVM, vm_free_success)
{
    int ret;
    ret = test_vm_add(1, TEST_FRAME_QUEUE_1);
    ASSERT_EQ(ret, EC_OK);
    ret = test_vm_free(1);
    EXPECT_EQ(ret, EC_OK);
    ret = test_vm_free(0);
    EXPECT_EQ(ret, EC_OK);
}