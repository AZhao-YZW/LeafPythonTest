#include "gtest/gtest.h"
#include "mockcpp/mokc.h"

extern "C" {
#include "test_core.h"
#include "test_data.h"
#include "error_code.h"
#include "leafpy_cfg.h"
}

class TestTestCore : public testing::Test {
protected:
    test_core_op_info_s op_info = {0};

    virtual void SetUp()
    {
        test_core_op_info_s tmp_op_info = {0};
        int ret;
        ret = test_core_init(LEAFPY_DEFAULT_CORE_ID);
        ASSERT_EQ(ret, EC_OK);
        op_info = tmp_op_info;
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

TEST_F(TestTestCore, core_run_core_id_unexist)
{
    int ret = test_core_run(LEAFPY_DEFAULT_CORE_ID + 1, &op_info);
    EXPECT_EQ(ret, EC_CORE_ID_INVALID);
}

TEST_F(TestTestCore, core_run_unsupport_op)
{
    op_info.op = TEST_CORE_OP_MAX;
    int ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_UNSUPPORT_OP);
}

static void test_core_run_op_NEW_success(test_core_op_info_s *op_info)
{
    char test_obj_name[] = "test_obj";
    int ret;

    op_info->op = TEST_CORE_OP_NEW;
    op_info->info.op_new.obj_type = OBJ_TYPE_OBJECT;
    op_info->info.op_new.obj_name = test_obj_name;
    op_info->info.op_new.obj_name_len = sizeof(test_obj_name);
    op_info->info.op_new.parent_id = ROOT_OBJ_ID;
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, op_info);
    EXPECT_EQ(ret, EC_OK);
}

TEST_F(TestTestCore, core_run_op_NEW_success)
{
    test_core_run_op_NEW_success(&op_info);
}

TEST_F(TestTestCore, core_run_op_NEW_obj_name_len_exceed)
{
    char test_obj_name[] = "test_obj";
    int ret;

    op_info.op = TEST_CORE_OP_NEW;
    op_info.info.op_new.obj_type = OBJ_TYPE_OBJECT;
    op_info.info.op_new.obj_name = test_obj_name;
    op_info.info.op_new.obj_name_len = LEAFPY_MAX_OBJ_NAME_LEN + 1;
    op_info.info.op_new.parent_id = ROOT_OBJ_ID;
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OBJ_NAME_LEN_EXCEED);
}

TEST_F(TestTestCore, op_test_DEL_success)
{
    int ret;

    test_core_run_op_NEW_success(&op_info);
    op_info.op = TEST_CORE_OP_DEL;
    op_info.info.op_del.obj_id = 2;
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OK);
}

TEST_F(TestTestCore, op_test_DEL_obj_id_unexist)
{
    int ret;

    op_info.op = TEST_CORE_OP_DEL;
    op_info.info.op_del.obj_id = 2;
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OBJ_ID_INVALID);
}

TEST_F(TestTestCore, op_test_DEL_obj_id_not_deletable)
{
    int ret;

    op_info.op = TEST_CORE_OP_DEL;
    op_info.info.op_del.obj_id = 0;
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OBJ_NOT_DELETABLE);

    op_info.info.op_del.obj_id = 1;
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OBJ_NOT_DELETABLE);
}