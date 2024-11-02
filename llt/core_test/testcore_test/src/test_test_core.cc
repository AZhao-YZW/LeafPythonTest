#include "gtest/gtest.h"
#include "mockcpp/mokc.h"

extern "C" {
#include "test_core.h"
#include "test_data.h"
#include "error_code.h"
#include "leafpy_cfg.h"
int test_core_run_op_proc(test_core_op_info_s *op_info, test_core_s *core);
}

class TestTestCore : public testing::Test {
protected:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
        GlobalMockObject::verify();
    }
};

TEST_F(TestTestCore, core_init)
{
    test_core_s *core = NULL;
    global_obj_s *global_obj = NULL;
    int ret;

    ret = test_core_init(LEAFPY_DEFAULT_CORE_ID);
    ASSERT_EQ(ret, EC_OK);

    core = test_core_get_core(LEAFPY_DEFAULT_CORE_ID);
    ASSERT_NE(core, (void *)NULL);
    ASSERT_NE(core->global_obj, (void *)NULL);
    ASSERT_EQ(core->core_id, LEAFPY_DEFAULT_CORE_ID);

    global_obj = core->global_obj;
    EXPECT_EQ(global_obj->obj_id_cnt, 2);
    EXPECT_EQ(global_obj->obj_attr.obj_type, OBJ_TYPE_GLOBAL);
    EXPECT_EQ(global_obj->obj_attr.free_flag, false);
    EXPECT_EQ(global_obj->obj_attr.layer, GLOBAL_OBJ_LAYER);
    EXPECT_EQ(global_obj->obj_attr.obj_id, GLOBAL_OBJ_ID);
    EXPECT_EQ(global_obj->obj_attr.parent_id, GLOBAL_OBJ_ID);
    EXPECT_EQ(global_obj->obj_attr.child_num, 0);
    EXPECT_STREQ(global_obj->obj_attr.obj_name, "global_obj");
}

TEST_F(TestTestCore, op_test_NEW)
{
    test_core_op_info_s op_info = {0};
    test_core_s *core = NULL;
    int ret;

    ret = test_core_init(LEAFPY_DEFAULT_CORE_ID);
    ASSERT_EQ(ret, EC_OK);

    core = test_core_get_core(LEAFPY_DEFAULT_CORE_ID);
    ASSERT_NE(core, (void *)NULL);

    op_info.op = TEST_CORE_OP_NEW;
    op_info.info.op_new.obj_type = OBJ_TYPE_OBJECT;
    ret = test_core_run_op_proc(&op_info, core);
    EXPECT_EQ(ret, EC_OK);
}