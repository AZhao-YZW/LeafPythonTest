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
        op_info.result.res_new.obj_id = 0;
    }

    void op_NEW_fill(u8 parent_id, u8 obj_type, u8 obj_subtype, char *obj_name, u8 obj_name_len)
    {
        op_info.op = TEST_CORE_OP_NEW;
        op_info.info.op_new.obj_type = obj_type;
        op_info.info.op_new.obj_subtype = obj_subtype;
        op_info.info.op_new.obj_name = obj_name;
        op_info.info.op_new.obj_name_len = obj_name_len;
        op_info.info.op_new.parent_id = parent_id;
        op_info.result.res_new.obj_id = 0;
    }

    void op_SET_fill(u8 obj_id, void *set_val)
    {
        op_info.op = TEST_CORE_OP_SET;
        op_info.info.op_set.obj_id = obj_id;
        op_info.info.op_set.obj_val = set_val;
    }

    void op_GET_fill(u8 obj_id, void *get_val)
    {
        op_info.op = TEST_CORE_OP_GET;
        op_info.info.op_get.obj_id = obj_id;
        op_info.result.res_get.obj_val = get_val;
    }

    void op_CACL_fill(enum test_core_calc_op_e calc_op,
        u8 t1, u8 st1, u8 id1, u8 t2, u8 st2, u8 id2, void *res_val, u32 val_len)
    {
        op_info.op = TEST_CORE_OP_CALC;
        op_info.info.op_calc.op = calc_op;
        op_info.info.op_calc.obj1_type = t1;
        op_info.info.op_calc.obj1_subtype = st1;
        op_info.info.op_calc.obj1_id = id1;
        op_info.info.op_calc.obj2_type = t2;
        op_info.info.op_calc.obj2_subtype = st2;
        op_info.info.op_calc.obj2_id = id2;
        op_info.info.op_calc.val_len = val_len;
        op_info.result.res_calc.obj_val = res_val;
    }

    void op_LOGIC_fill(enum test_core_logic_op_e logic_op,
        u8 t1, u8 st1, u8 id1, u8 t2, u8 st2, u8 id2, void *res_val, u32 val_len)
    {
        op_info.op = TEST_CORE_OP_LOGIC;
        op_info.info.op_logic.op = logic_op;
        op_info.info.op_logic.obj1_type = t1;
        op_info.info.op_logic.obj1_subtype = st1;
        op_info.info.op_logic.obj1_id = id1;
        op_info.info.op_logic.obj2_type = t2;
        op_info.info.op_logic.obj2_subtype = st2;
        op_info.info.op_logic.obj2_id = id2;
        op_info.info.op_logic.val_len = val_len;
        op_info.result.res_logic.obj_val = res_val;
    }

    void op_SET(u8 obj_id, void *set_val)
    {
        int ret;
        op_SET_fill(obj_id, set_val);
        ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
        ASSERT_EQ(ret, EC_OK); 
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

TEST_F(TestTestCore, core_run_op_SET_GET_int_success)
{
    static char int_obj_name[] = "int_obj";
    s64 set_val = 10;
    s64 get_val = 0;
    int ret;

    op_NEW_fill(ROOT_OBJ_ID, OBJ_TYPE_NUMBER, NUM_TYPE_INT, int_obj_name, sizeof(int_obj_name));
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);

    op_SET_fill(ROOT_OBJ_ID + 1, &set_val);
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OK);

    op_GET_fill(ROOT_OBJ_ID + 1, &get_val);
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

    op_SET_fill(ROOT_OBJ_ID + 1, &set_val);
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OK);

    op_GET_fill(ROOT_OBJ_ID + 1, &get_val);
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

    op_SET_fill(ROOT_OBJ_ID + 1, &set_val);
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OK);

    op_GET_fill(ROOT_OBJ_ID + 1, &get_val);
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

    op_SET_fill(ROOT_OBJ_ID + 1, &set_val_p);
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OK);

    op_GET_fill(ROOT_OBJ_ID + 1, &get_val);
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OK);
    EXPECT_EQ(get_val, set_val);
    EXPECT_STREQ(get_val, "hello");

    set_val[0] = 'H';
    op_SET_fill(ROOT_OBJ_ID + 1, &set_val_p);
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OK);

    op_GET_fill(ROOT_OBJ_ID + 1, &get_val);
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OK);
    EXPECT_EQ(get_val, set_val);
    EXPECT_STREQ(get_val, "Hello");
}

TEST_F(TestTestCore, core_run_op_SET_GET_proc_obj_id_unexist)
{
    static char int_obj_name[] = "int_obj";
    s64 set_val = 10;
    s64 get_val = 0;
    int ret;

    op_NEW_fill(ROOT_OBJ_ID, OBJ_TYPE_NUMBER, NUM_TYPE_INT, int_obj_name, sizeof(int_obj_name));
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);

    op_SET_fill(ROOT_OBJ_ID + 2, &set_val);
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
    op_SET_fill(ROOT_OBJ_ID + 1, &set_val1);
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);

    op_NEW_fill(ROOT_OBJ_ID, OBJ_TYPE_NUMBER, NUM_TYPE_INT, int_obj2_name, sizeof(int_obj2_name));
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);
    op_SET_fill(ROOT_OBJ_ID + 2, &set_val2);
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);

    op_CACL_fill(CALC_OP_ADD,
                 OBJ_TYPE_NUMBER, NUM_TYPE_INT, ROOT_OBJ_ID + 1,
                 OBJ_TYPE_NUMBER, NUM_TYPE_INT, ROOT_OBJ_ID + 2,
                 &res_val, sizeof(res_val));
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OK);
    EXPECT_EQ(res_val, set_val1 + set_val2);
}

TEST_F(TestTestCore, core_run_op_CALC_ADD_float_float_success)
{
    static char float_obj1_name[] = "float_obj1";
    static char float_obj2_name[] = "float_obj2";
    f64 set_val1 = 123.123;
    f64 set_val2 = 456.456;
    f64 res_val = 0;
    int ret;

    op_NEW_fill(ROOT_OBJ_ID, OBJ_TYPE_NUMBER, NUM_TYPE_FLOAT, float_obj1_name, sizeof(float_obj1_name));
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);
    op_SET_fill(ROOT_OBJ_ID + 1, &set_val1);
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);

    op_NEW_fill(ROOT_OBJ_ID, OBJ_TYPE_NUMBER, NUM_TYPE_FLOAT, float_obj2_name, sizeof(float_obj2_name));
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);
    op_SET_fill(ROOT_OBJ_ID + 2, &set_val2);
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);

    op_CACL_fill(CALC_OP_ADD,
                 OBJ_TYPE_NUMBER, NUM_TYPE_FLOAT, ROOT_OBJ_ID + 1,
                 OBJ_TYPE_NUMBER, NUM_TYPE_FLOAT, ROOT_OBJ_ID + 2,
                 &res_val, sizeof(res_val));
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OK);
    EXPECT_EQ(res_val, set_val1 + set_val2);
}

TEST_F(TestTestCore, core_run_op_CALC_ADD_String_String_success)
{
    static char String_obj1_name[] = "String_obj1";
    static char String_obj2_name[] = "String_obj2";
    char set_val1[] = "hello";
    char *set_val1_p = set_val1;
    char set_val2[] = " world";
    char *set_val2_p = set_val2;
    char res_val[] = "aaaaa bbbbb";
    char *res_val_p = res_val;
    int ret;

    op_NEW_fill(ROOT_OBJ_ID, OBJ_TYPE_STRING, NO_OBJ_SUBTYPE, String_obj1_name, sizeof(String_obj1_name));
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);
    op_SET_fill(ROOT_OBJ_ID + 1, &set_val1_p);
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);

    op_NEW_fill(ROOT_OBJ_ID, OBJ_TYPE_STRING, NO_OBJ_SUBTYPE, String_obj2_name, sizeof(String_obj2_name));
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);
    op_SET_fill(ROOT_OBJ_ID + 2, &set_val2_p);
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);

    op_CACL_fill(CALC_OP_ADD,
                 OBJ_TYPE_STRING, NO_OBJ_SUBTYPE, ROOT_OBJ_ID + 1,
                 OBJ_TYPE_STRING, NO_OBJ_SUBTYPE, ROOT_OBJ_ID + 2,
                 &res_val_p, sizeof(res_val));
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OK);
    EXPECT_STREQ(res_val, "hello world");
}

TEST_F(TestTestCore, core_run_op_CALC_SUB_int_int_success)
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
    op_SET_fill(ROOT_OBJ_ID + 1, &set_val1);
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);

    op_NEW_fill(ROOT_OBJ_ID, OBJ_TYPE_NUMBER, NUM_TYPE_INT, int_obj2_name, sizeof(int_obj2_name));
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);
    op_SET_fill(ROOT_OBJ_ID + 2, &set_val2);
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);

    op_CACL_fill(CALC_OP_SUB,
                 OBJ_TYPE_NUMBER, NUM_TYPE_INT, ROOT_OBJ_ID + 1,
                 OBJ_TYPE_NUMBER, NUM_TYPE_INT, ROOT_OBJ_ID + 2,
                 &res_val, sizeof(res_val));
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OK);
    EXPECT_EQ(res_val, set_val1 - set_val2);
}

TEST_F(TestTestCore, core_run_op_CALC_SUB_float_float_success)
{
    static char float_obj1_name[] = "float_obj1";
    static char float_obj2_name[] = "float_obj2";
    f64 set_val1 = 123.123;
    f64 set_val2 = 456.456;
    f64 res_val = 0;
    int ret;

    op_NEW_fill(ROOT_OBJ_ID, OBJ_TYPE_NUMBER, NUM_TYPE_FLOAT, float_obj1_name, sizeof(float_obj1_name));
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);
    op_SET_fill(ROOT_OBJ_ID + 1, &set_val1);
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);

    op_NEW_fill(ROOT_OBJ_ID, OBJ_TYPE_NUMBER, NUM_TYPE_FLOAT, float_obj2_name, sizeof(float_obj2_name));
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);
    op_SET_fill(ROOT_OBJ_ID + 2, &set_val2);
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);

    op_CACL_fill(CALC_OP_SUB,
                 OBJ_TYPE_NUMBER, NUM_TYPE_FLOAT, ROOT_OBJ_ID + 1,
                 OBJ_TYPE_NUMBER, NUM_TYPE_FLOAT, ROOT_OBJ_ID + 2,
                 &res_val, sizeof(res_val));
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OK);
    EXPECT_EQ(res_val, set_val1 - set_val2);
}

TEST_F(TestTestCore, core_run_op_CALC_SUB_String_String_invalid)
{
    static char String_obj1_name[] = "String_obj1";
    static char String_obj2_name[] = "String_obj2";
    char set_val1[] = "hello";
    char *set_val1_p = set_val1;
    char set_val2[] = " world";
    char *set_val2_p = set_val2;
    char res_val[] = "aaaaa bbbbb";
    char *res_val_p = res_val;
    int ret;

    op_NEW_fill(ROOT_OBJ_ID, OBJ_TYPE_STRING, NO_OBJ_SUBTYPE, String_obj1_name, sizeof(String_obj1_name));
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);
    op_SET_fill(ROOT_OBJ_ID + 1, &set_val1_p);
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);

    op_NEW_fill(ROOT_OBJ_ID, OBJ_TYPE_STRING, NO_OBJ_SUBTYPE, String_obj2_name, sizeof(String_obj2_name));
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);
    op_SET_fill(ROOT_OBJ_ID + 2, &set_val2_p);
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);

    op_CACL_fill(CALC_OP_SUB,
                 OBJ_TYPE_STRING, NO_OBJ_SUBTYPE, ROOT_OBJ_ID + 1,
                 OBJ_TYPE_STRING, NO_OBJ_SUBTYPE, ROOT_OBJ_ID + 2,
                 &res_val_p, sizeof(res_val));
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OBJ_TYPE_INVALID);
}

TEST_F(TestTestCore, core_run_op_CALC_MUL_int_int_success)
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
    op_SET_fill(ROOT_OBJ_ID + 1, &set_val1);
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);

    op_NEW_fill(ROOT_OBJ_ID, OBJ_TYPE_NUMBER, NUM_TYPE_INT, int_obj2_name, sizeof(int_obj2_name));
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);
    op_SET_fill(ROOT_OBJ_ID + 2, &set_val2);
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);

    op_CACL_fill(CALC_OP_MUL,
                 OBJ_TYPE_NUMBER, NUM_TYPE_INT, ROOT_OBJ_ID + 1,
                 OBJ_TYPE_NUMBER, NUM_TYPE_INT, ROOT_OBJ_ID + 2,
                 &res_val, sizeof(res_val));
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OK);
    EXPECT_EQ(res_val, set_val1 * set_val2);
}

TEST_F(TestTestCore, core_run_op_CALC_MUL_float_float_success)
{
    static char float_obj1_name[] = "float_obj1";
    static char float_obj2_name[] = "float_obj2";
    f64 set_val1 = 123.123;
    f64 set_val2 = 456.456;
    f64 res_val = 0;
    int ret;

    op_NEW_fill(ROOT_OBJ_ID, OBJ_TYPE_NUMBER, NUM_TYPE_FLOAT, float_obj1_name, sizeof(float_obj1_name));
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);
    op_SET_fill(ROOT_OBJ_ID + 1, &set_val1);
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);

    op_NEW_fill(ROOT_OBJ_ID, OBJ_TYPE_NUMBER, NUM_TYPE_FLOAT, float_obj2_name, sizeof(float_obj2_name));
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);
    op_SET_fill(ROOT_OBJ_ID + 2, &set_val2);
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);

    op_CACL_fill(CALC_OP_MUL,
                 OBJ_TYPE_NUMBER, NUM_TYPE_FLOAT, ROOT_OBJ_ID + 1,
                 OBJ_TYPE_NUMBER, NUM_TYPE_FLOAT, ROOT_OBJ_ID + 2,
                 &res_val, sizeof(res_val));
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OK);
    EXPECT_EQ(res_val, set_val1 * set_val2);
}

TEST_F(TestTestCore, core_run_op_CALC_MUL_String_String_invalid)
{
    static char String_obj1_name[] = "String_obj1";
    static char String_obj2_name[] = "String_obj2";
    char set_val1[] = "hello";
    char *set_val1_p = set_val1;
    char set_val2[] = " world";
    char *set_val2_p = set_val2;
    char res_val[] = "aaaaa bbbbb";
    char *res_val_p = res_val;
    int ret;

    op_NEW_fill(ROOT_OBJ_ID, OBJ_TYPE_STRING, NO_OBJ_SUBTYPE, String_obj1_name, sizeof(String_obj1_name));
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);
    op_SET_fill(ROOT_OBJ_ID + 1, &set_val1_p);
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);

    op_NEW_fill(ROOT_OBJ_ID, OBJ_TYPE_STRING, NO_OBJ_SUBTYPE, String_obj2_name, sizeof(String_obj2_name));
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);
    op_SET_fill(ROOT_OBJ_ID + 2, &set_val2_p);
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);

    op_CACL_fill(CALC_OP_MUL,
                 OBJ_TYPE_STRING, NO_OBJ_SUBTYPE, ROOT_OBJ_ID + 1,
                 OBJ_TYPE_STRING, NO_OBJ_SUBTYPE, ROOT_OBJ_ID + 2,
                 &res_val_p, sizeof(res_val));
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OBJ_TYPE_INVALID);
}

TEST_F(TestTestCore, core_run_op_CALC_DIV_int_int_success)
{
    static char int_obj1_name[] = "int_obj1";
    static char int_obj2_name[] = "int_obj2";
    s64 set_val1 = 12345;
    s64 set_val2 = 456;
    s64 res_val = 0;
    int ret;

    op_NEW_fill(ROOT_OBJ_ID, OBJ_TYPE_NUMBER, NUM_TYPE_INT, int_obj1_name, sizeof(int_obj1_name));
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);
    op_SET_fill(ROOT_OBJ_ID + 1, &set_val1);
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);

    op_NEW_fill(ROOT_OBJ_ID, OBJ_TYPE_NUMBER, NUM_TYPE_INT, int_obj2_name, sizeof(int_obj2_name));
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);
    op_SET_fill(ROOT_OBJ_ID + 2, &set_val2);
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);

    op_CACL_fill(CALC_OP_DIV,
                 OBJ_TYPE_NUMBER, NUM_TYPE_INT, ROOT_OBJ_ID + 1,
                 OBJ_TYPE_NUMBER, NUM_TYPE_INT, ROOT_OBJ_ID + 2,
                 &res_val, sizeof(res_val));
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OK);
    EXPECT_EQ(res_val, set_val1 / set_val2);
}

TEST_F(TestTestCore, core_run_op_CALC_DIV_float_float_success)
{
    static char float_obj1_name[] = "float_obj1";
    static char float_obj2_name[] = "float_obj2";
    f64 set_val1 = 123.123;
    f64 set_val2 = 456.456;
    f64 res_val = 0;
    int ret;

    op_NEW_fill(ROOT_OBJ_ID, OBJ_TYPE_NUMBER, NUM_TYPE_FLOAT, float_obj1_name, sizeof(float_obj1_name));
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);
    op_SET_fill(ROOT_OBJ_ID + 1, &set_val1);
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);

    op_NEW_fill(ROOT_OBJ_ID, OBJ_TYPE_NUMBER, NUM_TYPE_FLOAT, float_obj2_name, sizeof(float_obj2_name));
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);
    op_SET_fill(ROOT_OBJ_ID + 2, &set_val2);
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);

    op_CACL_fill(CALC_OP_DIV,
                 OBJ_TYPE_NUMBER, NUM_TYPE_FLOAT, ROOT_OBJ_ID + 1,
                 OBJ_TYPE_NUMBER, NUM_TYPE_FLOAT, ROOT_OBJ_ID + 2,
                 &res_val, sizeof(res_val));
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OK);
    EXPECT_EQ(res_val, set_val1 / set_val2);
}

TEST_F(TestTestCore, core_run_op_CALC_DIV_String_String_invalid)
{
    static char String_obj1_name[] = "String_obj1";
    static char String_obj2_name[] = "String_obj2";
    char set_val1[] = "hello";
    char *set_val1_p = set_val1;
    char set_val2[] = " world";
    char *set_val2_p = set_val2;
    char res_val[] = "aaaaa bbbbb";
    char *res_val_p = res_val;
    int ret;

    op_NEW_fill(ROOT_OBJ_ID, OBJ_TYPE_STRING, NO_OBJ_SUBTYPE, String_obj1_name, sizeof(String_obj1_name));
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);
    op_SET_fill(ROOT_OBJ_ID + 1, &set_val1_p);
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);

    op_NEW_fill(ROOT_OBJ_ID, OBJ_TYPE_STRING, NO_OBJ_SUBTYPE, String_obj2_name, sizeof(String_obj2_name));
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);
    op_SET_fill(ROOT_OBJ_ID + 2, &set_val2_p);
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);

    op_CACL_fill(CALC_OP_DIV,
                 OBJ_TYPE_STRING, NO_OBJ_SUBTYPE, ROOT_OBJ_ID + 1,
                 OBJ_TYPE_STRING, NO_OBJ_SUBTYPE, ROOT_OBJ_ID + 2,
                 &res_val_p, sizeof(res_val));
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    EXPECT_EQ(ret, EC_OBJ_TYPE_INVALID);
}

TEST_F(TestTestCore, core_run_op_LOGIC_int_success)
{
    struct {
        enum test_core_logic_op_e logic_op;
        s64 set_val1;
        s64 set_val2;
        bool exp_res_val_bool;
        s64 exp_res_val_int;
    } logic_op_info[] = {
        // result is bool
        { LOGIC_OP_EQ, 123, 456, false },
        { LOGIC_OP_EQ, 123, 123, true },
        { LOGIC_OP_NE, 123, 456, true },
        { LOGIC_OP_NE, 123, 123, false },
        { LOGIC_OP_GE, 123, 456, false },
        { LOGIC_OP_GE, 123, 123, true },
        { LOGIC_OP_GE, 123, 1, true },
        { LOGIC_OP_GT, 123, 456, false },
        { LOGIC_OP_GT, 123, 123, false },
        { LOGIC_OP_GT, 123, 1, true },
        { LOGIC_OP_LE, 123, 456, true },
        { LOGIC_OP_LE, 123, 123, true },
        { LOGIC_OP_LE, 123, 1, false },
        { LOGIC_OP_LT, 123, 456, true },
        { LOGIC_OP_LT, 123, 123, false },
        { LOGIC_OP_LT, 123, 1, false },
        { LOGIC_OP_LOGIC_NOT, 123, 0, false },
        { LOGIC_OP_LOGIC_NOT, 0, 0, true },
        // result is int
        { LOGIC_OP_LOGIC_AND, 123, 456, false, 456 },
        { LOGIC_OP_LOGIC_AND, 0, 456, false, 0 },
        { LOGIC_OP_LOGIC_OR, 123, 456, false, 123 },
        { LOGIC_OP_LOGIC_OR, 0, 456, false, 456 },
    };
    static char int_obj1_name[] = "int_obj1";
    static char int_obj2_name[] = "int_obj2";
    s64 res_val = 0;
    int ret;

    op_NEW_fill(ROOT_OBJ_ID, OBJ_TYPE_NUMBER, NUM_TYPE_INT, int_obj1_name, sizeof(int_obj1_name));
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);

    op_NEW_fill(ROOT_OBJ_ID, OBJ_TYPE_NUMBER, NUM_TYPE_INT, int_obj2_name, sizeof(int_obj2_name));
    ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
    ASSERT_EQ(ret, EC_OK);

    for (int i = 0; i < ARRAY_SIZE(logic_op_info); i++) {
        printf("i = %d\n", i);
        op_SET(ROOT_OBJ_ID + 1, &logic_op_info[i].set_val1);
        op_SET(ROOT_OBJ_ID + 2, &logic_op_info[i].set_val2);

        op_LOGIC_fill(logic_op_info[i].logic_op,
                      OBJ_TYPE_NUMBER, NUM_TYPE_INT, ROOT_OBJ_ID + 1,
                      OBJ_TYPE_NUMBER, NUM_TYPE_INT, ROOT_OBJ_ID + 2,
                      &res_val, sizeof(res_val));
        ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
        EXPECT_EQ(ret, EC_OK);
        if (logic_op_info[i].logic_op == LOGIC_OP_LOGIC_AND || logic_op_info[i].logic_op == LOGIC_OP_LOGIC_OR) {
            EXPECT_EQ(res_val, logic_op_info[i].exp_res_val_int);
        } else {
            EXPECT_EQ(res_val, logic_op_info[i].exp_res_val_bool);
        }
    }
}