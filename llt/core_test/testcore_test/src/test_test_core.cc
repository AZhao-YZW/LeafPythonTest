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
    EXPECT_EQ(global_obj->obj_attr.child_num, 1);
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

static void test_core_op_NEW_fill(test_core_op_info_s *op_info)
{
    static char test_obj_name[] = "test_obj";

    op_info->op = TEST_CORE_OP_NEW;
    op_info->info.op_new.obj_type = OBJ_TYPE_OBJECT;
    op_info->info.op_new.obj_subtype = NO_OBJ_SUBTYPE;
    op_info->info.op_new.obj_name = test_obj_name;
    op_info->info.op_new.obj_name_len = sizeof(test_obj_name);
    op_info->info.op_new.parent_id = ROOT_OBJ_ID;
}

TEST_F(TestTestCore, core_run_op_NEW_success)
{
    int ret;

    test_core_op_NEW_fill(&op_info);
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OK);
}

TEST_F(TestTestCore, core_run_op_NEW_success_more_types)
{
    static char test_obj_name[] = "test_obj";
    int ret, i;
    struct {
        u8 obj_type;
        u8 obj_subtype;
    } obj_type_map[] = {
        { OBJ_TYPE_OBJECT, NO_OBJ_SUBTYPE   },
        { OBJ_TYPE_NUMBER, NUM_TYPE_NUMBER  },
        { OBJ_TYPE_NUMBER, NUM_TYPE_INT     },
        { OBJ_TYPE_NUMBER, NUM_TYPE_FLOAT   },
        { OBJ_TYPE_NUMBER, NUM_TYPE_BOOL    },
        { OBJ_TYPE_BOOL,   NO_OBJ_SUBTYPE   },
        { OBJ_TYPE_NUMBER, NUM_TYPE_COMPLEX },
        { OBJ_TYPE_STRING, NO_OBJ_SUBTYPE   },
        { OBJ_TYPE_LIST,   NO_OBJ_SUBTYPE   },
        { OBJ_TYPE_TUPLE,  NO_OBJ_SUBTYPE   },
        { OBJ_TYPE_SET,    NO_OBJ_SUBTYPE   },
        { OBJ_TYPE_DICT,   NO_OBJ_SUBTYPE   },
    };

    op_info.op = TEST_CORE_OP_NEW;
    op_info.info.op_new.obj_name = test_obj_name;
    op_info.info.op_new.obj_name_len = sizeof(test_obj_name);
    op_info.info.op_new.parent_id = ROOT_OBJ_ID;

    for (i = 0; i < ARRAY_SIZE(obj_type_map); i++) {
        op_info.info.op_new.obj_type = obj_type_map[i].obj_type;
        op_info.info.op_new.obj_subtype = obj_type_map[i].obj_subtype;
        ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
        EXPECT_EQ(ret, EC_OK);
    }
}

TEST_F(TestTestCore, core_run_op_NEW_type_invalid)
{
    int ret;

    test_core_op_NEW_fill(&op_info);
    op_info.info.op_new.obj_type = OBJ_TYPE_MAX;
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OBJ_TYPE_INVALID);

    op_info.info.op_new.obj_type = OBJ_TYPE_OBJECT;
    op_info.info.op_new.obj_subtype = NO_OBJ_SUBTYPE + 1;
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OBJ_TYPE_INVALID);
}

TEST_F(TestTestCore, core_run_op_NEW_obj_name_len_exceed)
{
    int ret;

    test_core_op_NEW_fill(&op_info);
    op_info.info.op_new.obj_name_len = LEAFPY_MAX_OBJ_NAME_LEN + 1;
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OBJ_NAME_LEN_EXCEED);
}

TEST_F(TestTestCore, core_run_op_NEW_parent_id_unexist)
{
    int ret;

    test_core_op_NEW_fill(&op_info);
    op_info.info.op_new.parent_id = ROOT_OBJ_ID + 1;
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OBJ_NOT_FOUND);
}

TEST_F(TestTestCore, op_test_DEL_success)
{
    int ret;

    test_core_op_NEW_fill(&op_info);
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);

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
    EXPECT_EQ(ret, EC_OBJ_NOT_FOUND);
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

TEST_F(TestTestCore, op_test_FIND_success)
{
    int ret;

    op_info.op = TEST_CORE_OP_FIND;
    op_info.info.op_find.obj_name = "global_obj";
    op_info.result.res_find.obj_id = 0;
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OK);
    EXPECT_EQ(op_info.result.res_find.obj_id, GLOBAL_OBJ_ID);

    op_info.info.op_find.obj_name = "root_obj";
    op_info.result.res_find.obj_id = 0;
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OK);
    EXPECT_EQ(op_info.result.res_find.obj_id, ROOT_OBJ_ID);

    test_core_op_NEW_fill(&op_info);
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);

    op_info.op = TEST_CORE_OP_FIND;
    op_info.info.op_find.obj_name = "test_obj";
    op_info.result.res_find.obj_id = 0;
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OK);
    EXPECT_EQ(op_info.result.res_find.obj_id, ROOT_OBJ_ID + 1);
}

TEST_F(TestTestCore, op_test_FIND_obj_name_unexist)
{
    int ret;

    op_info.op = TEST_CORE_OP_FIND;
    op_info.info.op_find.obj_name = "test_obj";
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OBJ_NOT_FOUND);
}