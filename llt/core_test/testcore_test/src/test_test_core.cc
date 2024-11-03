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

    void op_NEW_fill_object_obj(void)
    {
        static char test_obj_name[] = "test_obj";

        op_info.op = TEST_CORE_OP_NEW;
        op_info.info.op_new.obj_type = OBJ_TYPE_OBJECT;
        op_info.info.op_new.obj_subtype = NO_OBJ_SUBTYPE;
        op_info.info.op_new.obj_name = test_obj_name;
        op_info.info.op_new.obj_name_len = sizeof(test_obj_name);
        op_info.info.op_new.parent_id = ROOT_OBJ_ID;
    }

    void op_NEW_fill(u8 parent_id, u8 obj_type, u8 obj_subtype, char *obj_name, u8 obj_name_len)
    {
        op_info.op = TEST_CORE_OP_NEW;
        op_info.info.op_new.obj_type = obj_type;
        op_info.info.op_new.obj_subtype = obj_subtype;
        op_info.info.op_new.obj_name = obj_name;
        op_info.info.op_new.obj_name_len = obj_name_len;
        op_info.info.op_new.parent_id = parent_id;
    }

    void op_SET_fill(u8 obj_id, u8 obj_type, u8 obj_subtype, void *set_val)
    {
        op_info.op = TEST_CORE_OP_SET;
        op_info.info.op_set.obj_id = obj_id;
        op_info.info.op_set.obj_type = obj_type;
        op_info.info.op_set.obj_subtype = obj_subtype;
        op_info.info.op_set.obj_val = set_val;
    }

    void op_GET_fill(u8 obj_id, u8 obj_type, u8 obj_subtype, void *get_val)
    {
        op_info.op = TEST_CORE_OP_GET;
        op_info.info.op_get.obj_id = obj_id;
        op_info.info.op_get.obj_type = obj_type;
        op_info.info.op_get.obj_subtype = obj_subtype;
        op_info.result.res_get.obj_val = get_val;
    }

    void op_CACL_ADD_fill(u8 obj1_id, u8 obj2_id, void *res_val, u32 val_len)
    {
        op_info.op = TEST_CORE_OP_CALC;
        op_info.info.op_calc.op = CALC_OP_ADD;
        op_info.info.op_calc.obj1_id = obj1_id;
        op_info.info.op_calc.obj2_id = obj2_id;
        op_info.info.op_calc.val_len = val_len;
        op_info.result.res_calc.obj_val = res_val;
    }

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
    EXPECT_STREQ(global_obj->obj_attr.obj_name, GLOBAL_OBJ_NAME);
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

TEST_F(TestTestCore, core_run_op_NEW_success)
{
    int ret;

    op_NEW_fill_object_obj();
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
        op_info.info.op_new.parent_id++;
    }
}

TEST_F(TestTestCore, core_run_op_NEW_type_invalid)
{
    int ret;

    op_NEW_fill_object_obj();
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

    op_NEW_fill_object_obj();
    op_info.info.op_new.obj_name_len = LEAFPY_MAX_OBJ_NAME_LEN + 1;
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OBJ_NAME_LEN_EXCEED);
}

TEST_F(TestTestCore, core_run_op_NEW_parent_id_unexist)
{
    int ret;

    op_NEW_fill_object_obj();
    op_info.info.op_new.parent_id = ROOT_OBJ_ID + 1;
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OBJ_NOT_FOUND);
}

TEST_F(TestTestCore, core_run_op_NEW_obj_name_exist_under_parent_id)
{
    int ret, i;

    op_NEW_fill_object_obj();
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);

    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OBJ_NAME_INVALID);

    for (i = 0; i < 10; i++) {
        op_info.info.op_new.parent_id++;
        ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
        EXPECT_EQ(ret, EC_OK);
    }

    op_info.info.op_new.parent_id--;
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OBJ_NAME_INVALID);
}

TEST_F(TestTestCore, core_run_op_DEL_success)
{
    int ret;

    op_NEW_fill_object_obj();
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);

    op_info.op = TEST_CORE_OP_DEL;
    op_info.info.op_del.obj_id = 2;
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OK);
}

TEST_F(TestTestCore, core_run_op_DEL_obj_id_unexist)
{
    int ret;

    op_info.op = TEST_CORE_OP_DEL;
    op_info.info.op_del.obj_id = 2;
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OBJ_NOT_FOUND);
}

TEST_F(TestTestCore, core_run_op_DEL_obj_id_not_deletable)
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

TEST_F(TestTestCore, core_run_op_FIND_success)
{
    int ret;

    op_info.op = TEST_CORE_OP_FIND;
    op_info.info.op_find.parent_id = GLOBAL_OBJ_ID;
    op_info.info.op_find.obj_name = GLOBAL_OBJ_NAME;
    op_info.result.res_find.obj_id = 0;
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OK);
    EXPECT_EQ(op_info.result.res_find.obj_id, GLOBAL_OBJ_ID);

    op_info.info.op_find.parent_id = GLOBAL_OBJ_ID;
    op_info.info.op_find.obj_name = ROOT_OBJ_NAME;
    op_info.result.res_find.obj_id = 0;
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OK);
    EXPECT_EQ(op_info.result.res_find.obj_id, ROOT_OBJ_ID);

    op_NEW_fill_object_obj();
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);

    op_info.info.op_new.parent_id = ROOT_OBJ_ID + 1;
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);

    op_info.op = TEST_CORE_OP_FIND;
    op_info.info.op_find.parent_id = ROOT_OBJ_ID;
    op_info.info.op_find.obj_name = "test_obj";
    op_info.result.res_find.obj_id = 0;
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OK);
    EXPECT_EQ(op_info.result.res_find.obj_id, ROOT_OBJ_ID + 1);

    op_info.info.op_find.parent_id = ROOT_OBJ_ID + 1;
    op_info.result.res_find.obj_id = 0;
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OK);
    EXPECT_EQ(op_info.result.res_find.obj_id, ROOT_OBJ_ID + 2);
}

TEST_F(TestTestCore, core_run_op_FIND_obj_name_unexist)
{
    int ret;

    op_info.op = TEST_CORE_OP_FIND;
    op_info.info.op_find.parent_id = ROOT_OBJ_ID;
    op_info.info.op_find.obj_name = "test_obj";
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OBJ_NOT_FOUND);
}

TEST_F(TestTestCore, core_run_op_SET_GET_int_success)
{
    static char int_obj_name[] = "int_obj";
    s64 set_val = 10;
    s64 get_val = 0;
    int ret;

    op_NEW_fill(ROOT_OBJ_ID, OBJ_TYPE_NUMBER, NUM_TYPE_INT, int_obj_name, sizeof(int_obj_name));
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);

    op_SET_fill(ROOT_OBJ_ID + 1, OBJ_TYPE_NUMBER, NUM_TYPE_INT, &set_val);
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OK);

    op_GET_fill(ROOT_OBJ_ID + 1, OBJ_TYPE_NUMBER, NUM_TYPE_INT, &get_val);
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OK);
    EXPECT_EQ(get_val, 10);
}

TEST_F(TestTestCore, core_run_op_SET_GET_float_success)
{
    static char float_obj_name[] = "float_obj";
    f64 set_val = 10.123;
    f64 get_val = 0;
    int ret;

    op_NEW_fill(ROOT_OBJ_ID, OBJ_TYPE_NUMBER, NUM_TYPE_FLOAT, float_obj_name, sizeof(float_obj_name));
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);

    op_SET_fill(ROOT_OBJ_ID + 1, OBJ_TYPE_NUMBER, NUM_TYPE_FLOAT, &set_val);
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OK);

    op_GET_fill(ROOT_OBJ_ID + 1, OBJ_TYPE_NUMBER, NUM_TYPE_FLOAT, &get_val);
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OK);
    EXPECT_EQ(get_val, 10.123);
}

TEST_F(TestTestCore, core_run_op_SET_GET_bool_success)
{
    static char bool_obj_name[] = "bool_obj";
    u8 set_val = 1;
    u8 get_val = 0;
    int ret;

    op_NEW_fill(ROOT_OBJ_ID, OBJ_TYPE_NUMBER, NUM_TYPE_BOOL, bool_obj_name, sizeof(bool_obj_name));
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);

    op_SET_fill(ROOT_OBJ_ID + 1, OBJ_TYPE_NUMBER, NUM_TYPE_BOOL, &set_val);
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OK);

    op_GET_fill(ROOT_OBJ_ID + 1, OBJ_TYPE_NUMBER, NUM_TYPE_BOOL, &get_val);
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OK);
    EXPECT_EQ(get_val, 1);
}

TEST_F(TestTestCore, core_run_op_SET_GET_String_success)
{
    static char String_obj_name[] = "String_obj";
    char set_val[] = "hello";
    char *set_val_p = set_val;
    char *get_val = NULL;
    int ret;

    op_NEW_fill(ROOT_OBJ_ID, OBJ_TYPE_STRING, NO_OBJ_SUBTYPE, String_obj_name, sizeof(String_obj_name));
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);

    op_SET_fill(ROOT_OBJ_ID + 1, OBJ_TYPE_STRING, NO_OBJ_SUBTYPE, &set_val_p);
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OK);

    op_GET_fill(ROOT_OBJ_ID + 1, OBJ_TYPE_STRING, NO_OBJ_SUBTYPE, &get_val);
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OK);
    EXPECT_EQ(get_val, set_val);
    EXPECT_STREQ(get_val, "hello");

    set_val[0] = 'H';
    op_SET_fill(ROOT_OBJ_ID + 1, OBJ_TYPE_STRING, NO_OBJ_SUBTYPE, &set_val_p);
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OK);

    op_GET_fill(ROOT_OBJ_ID + 1, OBJ_TYPE_STRING, NO_OBJ_SUBTYPE, &get_val);
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OK);
    EXPECT_EQ(get_val, set_val);
    EXPECT_STREQ(get_val, "Hello");
}

TEST_F(TestTestCore, core_run_obj_op_proc_obj_id_unexist)
{
    static char int_obj_name[] = "int_obj";
    s64 set_val = 10;
    s64 get_val = 0;
    int ret;

    op_NEW_fill(ROOT_OBJ_ID, OBJ_TYPE_NUMBER, NUM_TYPE_INT, int_obj_name, sizeof(int_obj_name));
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);

    op_SET_fill(ROOT_OBJ_ID + 2, OBJ_TYPE_NUMBER, NUM_TYPE_INT, &set_val);
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OBJ_NOT_FOUND);
}

TEST_F(TestTestCore, core_run_op_CALC_ADD_int_int_success)
{
    static char int_obj1_name[] = "int_obj1";
    static char int_obj2_name[] = "int_obj2";
    s64 set_val1 = 123;
    s64 set_val2 = 456;
    s64 res_val = 0;
    int ret;

    op_NEW_fill(ROOT_OBJ_ID, OBJ_TYPE_NUMBER, NUM_TYPE_INT, int_obj1_name, sizeof(int_obj1_name));
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);
    op_SET_fill(ROOT_OBJ_ID + 1, OBJ_TYPE_NUMBER, NUM_TYPE_INT, &set_val1);
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);

    op_NEW_fill(ROOT_OBJ_ID, OBJ_TYPE_NUMBER, NUM_TYPE_INT, int_obj2_name, sizeof(int_obj2_name));
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);
    op_SET_fill(ROOT_OBJ_ID + 2, OBJ_TYPE_NUMBER, NUM_TYPE_INT, &set_val2);
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);

    op_CACL_ADD_fill(ROOT_OBJ_ID + 1, ROOT_OBJ_ID + 2, &res_val, sizeof(res_val));
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OK);
    EXPECT_EQ(res_val, 123 + 456);
}