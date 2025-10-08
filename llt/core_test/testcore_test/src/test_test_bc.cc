#include "gtest/gtest.h"
#include "mockcpp/mokc.h"

extern "C" {
#include "test_bc.h"
#include "test_core.h"
#include "test_obj_type.h"
#include "error_code.h"
#include "leafpy_cfg.h"
}

class TestTestBC : public testing::Test {
protected:
    test_bc_s bc = {0};
    static u64 pc;

    static int bc_cb(test_bc_s *bc)
    {
        printf("bc_cb: bc_op[%u] sub_op[%u] parent_id[%u] pos[0x%llx] next_pos[0x%llx] pc[0x%llx]\n",
            bc->op, bc->sub_op, bc->parent_id, bc->pos, bc->next_pos, pc);
        test_core_print_obj_list(LEAFPY_DEFAULT_CORE_ID);
        pc++;
        return EC_OK;
    }

    void bc_check_obj(u32 obj_id, const void *val)
    {
        test_core_op_info_s op_info = {0};
        u8 obj_type = 0, obj_subtype = 0;
        union {
            s64 int_val;
            f64 float_val;
        } get_val;
        int ret;

        test_core_op_get_fill(obj_id, &get_val, &op_info);
        ret = test_core_run(LEAFPY_DEFAULT_CORE_ID, &op_info);
        EXPECT_EQ(ret, EC_OK);
        ret = test_core_obj_get_type(LEAFPY_DEFAULT_CORE_ID, obj_id, &obj_type, &obj_subtype);
        EXPECT_EQ(ret, EC_OK);
        if (obj_subtype == NUM_TYPE_INT) {
            EXPECT_EQ(get_val.int_val, *(s64 *)val);
        } else if (obj_subtype == NUM_TYPE_FLOAT) {
            EXPECT_EQ(get_val.float_val, *(f64 *)val);
        }
    }

    void bc_BC_init(enum test_bc_op_e bc_op)
    {
        bc.op = bc_op;
        bc.core_id = LEAFPY_DEFAULT_CORE_ID;
        bc.parent_id = ROOT_OBJ_ID;
        bc.bc_res = 0;
        bc.pos = pc;
        bc.next_pos = pc + 1;
    }

    int bc_NOP(void)
    {
        bc_BC_init(TEST_BC_NOP);
        return test_bc_proc(&bc, NULL, bc_cb);
    }

    int bc_NEW(const char *obj_name, u8 obj_type, u8 obj_subtype)
    {
        bc_BC_init(TEST_BC_NEW);
        bc.args.obj_type1 = obj_type;
        bc.args.obj_subtype1 = obj_subtype;
        bc.args.bc_new.obj_name = obj_name;
        return test_bc_proc(&bc, NULL, bc_cb);
    }

    int bc_DEL(u32 obj_id)
    {
        bc_BC_init(TEST_BC_DEL);
        bc.args.bc_1_arg.obj_id = obj_id;
        return test_bc_proc(&bc, NULL, bc_cb);
    }

    int bc_MOV_obj(u32 obj_id1, u32 obj_id2)
    {
        bc_BC_init(TEST_BC_MOV);
        bc.sub_op = TEST_BC_MOV_OBJ;
        bc.args.bc_2_args.obj_id1 = obj_id1;
        bc.args.bc_2_args.obj_id2 = obj_id2;
        return test_bc_proc(&bc, NULL, bc_cb);
    }

    int bc_MOV_val(u32 obj_id1, const void *val)
    {
        int ret;
        bc_BC_init(TEST_BC_MOV);
        bc.sub_op = TEST_BC_MOV_VAL;
        bc.args.bc_2_args.obj_id1 = obj_id1;
        bc.args.bc_2_args.val = val;
        ret = test_bc_proc(&bc, NULL, bc_cb);
        bc_check_obj(obj_id1, val);
        return ret;
    }

    int bc_INC(u32 obj_id)
    {
        bc_BC_init(TEST_BC_INC);
        bc.args.bc_1_arg.obj_id = obj_id;
        return test_bc_proc(&bc, NULL, bc_cb);
    }

    int bc_DEC(u32 obj_id)
    {
        bc_BC_init(TEST_BC_DEC);
        bc.args.bc_1_arg.obj_id = obj_id;
        return test_bc_proc(&bc, NULL, bc_cb);
    }

    int bc_CALC(enum test_bc_op_e bc_op, u32 obj_id1, u32 obj_id2, u32 obj_id3)
    {
        bc_BC_init(bc_op);
        bc.args.bc_3_args.obj_id1 = obj_id1;
        bc.args.bc_3_args.t1.obj_id2 = obj_id2;
        bc.args.bc_3_args.t1.obj_id3 = obj_id3;
        return test_bc_proc(&bc, NULL, bc_cb);
    }

    int bc_CMP(u32 obj_id1, u32 obj_id2, u32 obj_id3)
    {
        bc_BC_init(TEST_BC_CMP);
        bc.args.bc_3_args.obj_id1 = obj_id1;
        bc.args.bc_3_args.t1.obj_id2 = obj_id2;
        bc.args.bc_3_args.t1.obj_id3 = obj_id3;
        return test_bc_proc(&bc, NULL, bc_cb);
    }

    virtual void SetUp()
    {
        test_bc_s tmp_bc = {0};
        int ret;
        ret = test_core_init(LEAFPY_DEFAULT_CORE_ID);
        ASSERT_EQ(ret, EC_OK);
        bc = tmp_bc;
    }

    virtual void TearDown()
    {
        test_core_free_all();
        pc = 0;
        GlobalMockObject::verify();
    }
};

u64 TestTestBC::pc = 0;

TEST_F(TestTestBC, bc_proc_NOP_success)
{
    int ret = bc_NOP();
    EXPECT_EQ(ret, EC_OK);
}

TEST_F(TestTestBC, bc_proc_NEW_success)
{
    int ret = bc_NEW("int_obj1", OBJ_TYPE_NUMBER, NUM_TYPE_INT);
    EXPECT_EQ(ret, EC_OK);
}

TEST_F(TestTestBC, bc_proc_DEL_success)
{
    u32 obj_id;
    int ret;

    ret = bc_NEW("int_obj1", OBJ_TYPE_NUMBER, NUM_TYPE_INT);
    ASSERT_EQ(ret, EC_OK);
    obj_id = bc.bc_new_res.obj_id;

    ret = bc_DEL(obj_id);
    EXPECT_EQ(ret, EC_OK);
}

TEST_F(TestTestBC, bc_proc_MOV_success)
{
    u32 obj_id1, obj_id2;
    s64 val1 = 123, val2 = 456;
    int ret;

    ret = bc_NEW("int_obj1", OBJ_TYPE_NUMBER, NUM_TYPE_INT);
    ASSERT_EQ(ret, EC_OK);
    obj_id1 = bc.bc_new_res.obj_id;
    
    ret = bc_NEW("int_obj2", OBJ_TYPE_NUMBER, NUM_TYPE_INT);
    ASSERT_EQ(ret, EC_OK);
    obj_id2 = bc.bc_new_res.obj_id;

    // MOV VAL
    ret = bc_MOV_val(obj_id1, &val1);
    EXPECT_EQ(ret, EC_OK);
    ret = bc_MOV_val(obj_id2, &val2);
    EXPECT_EQ(ret, EC_OK);

    // MOV OBJ
    ret = bc_MOV_obj(obj_id1, obj_id2);
    EXPECT_EQ(ret, EC_OK);
    val1 = val2;
    bc_check_obj(obj_id1, &val1);
    bc_check_obj(obj_id2, &val2);
}

TEST_F(TestTestBC, bc_proc_INC_int_success)
{
    u32 obj_id;
    s64 val = 123;
    int ret;

    ret = bc_NEW("int_obj", OBJ_TYPE_NUMBER, NUM_TYPE_INT);
    ASSERT_EQ(ret, EC_OK);
    obj_id = bc.bc_new_res.obj_id;

    ret = bc_MOV_val(obj_id, &val);
    ASSERT_EQ(ret, EC_OK);
    
    ret = bc_INC(obj_id);
    EXPECT_EQ(ret, EC_OK);
    val += 1;
    bc_check_obj(obj_id, &val);
}

TEST_F(TestTestBC, bc_proc_INC_float_success)
{
    u32 obj_id;
    f64 val = 123.123;
    int ret;

    ret = bc_NEW("float_obj", OBJ_TYPE_NUMBER, NUM_TYPE_FLOAT);
    ASSERT_EQ(ret, EC_OK);
    obj_id = bc.bc_new_res.obj_id;

    ret = bc_MOV_val(obj_id, &val);
    ASSERT_EQ(ret, EC_OK);
    
    ret = bc_INC(obj_id);
    EXPECT_EQ(ret, EC_OK);
    val += 1.0;
    bc_check_obj(obj_id, &val);
}

TEST_F(TestTestBC, bc_proc_DEC_int_success)
{
    u32 obj_id;
    s64 val = 123;
    int ret;

    ret = bc_NEW("int_obj", OBJ_TYPE_NUMBER, NUM_TYPE_INT);
    ASSERT_EQ(ret, EC_OK);
    obj_id = bc.bc_new_res.obj_id;

    ret = bc_MOV_val(obj_id, &val);
    ASSERT_EQ(ret, EC_OK);
    
    ret = bc_DEC(obj_id);
    EXPECT_EQ(ret, EC_OK);
    val -= 1;
    bc_check_obj(obj_id, &val);
}

TEST_F(TestTestBC, bc_proc_DEC_float_success)
{
    u32 obj_id;
    f64 val = 123.123;
    int ret;

    ret = bc_NEW("float_obj", OBJ_TYPE_NUMBER, NUM_TYPE_FLOAT);
    ASSERT_EQ(ret, EC_OK);
    obj_id = bc.bc_new_res.obj_id;

    ret = bc_MOV_val(obj_id, &val);
    ASSERT_EQ(ret, EC_OK);
    
    ret = bc_DEC(obj_id);
    EXPECT_EQ(ret, EC_OK);
    val -= 1.0;
    bc_check_obj(obj_id, &val);
}

TEST_F(TestTestBC, bc_proc_CALC_int_success)
{
    u32 obj_id1, obj_id2, obj_id3;
    struct {
        enum test_bc_op_e bc_op;
        s64 val1;
        s64 val2;
        s64 val3;
        s64 exp_val1;
    } calc_val_map[] = {
        { TEST_BC_ADD, 123, 456, 789, 456 + 789 },
        { TEST_BC_SUB, 123, 456, 789, 456 - 789 },
        { TEST_BC_MUL, 123, 456, 789, 456 * 789 },
        { TEST_BC_DIV, 123, 45600, 789, 45600 / 789 },
    };
    int ret;

    ret = bc_NEW("int1_obj", OBJ_TYPE_NUMBER, NUM_TYPE_INT);
    ASSERT_EQ(ret, EC_OK);
    obj_id1 = bc.bc_new_res.obj_id;

    ret = bc_NEW("int2_obj", OBJ_TYPE_NUMBER, NUM_TYPE_INT);
    ASSERT_EQ(ret, EC_OK);
    obj_id2 = bc.bc_new_res.obj_id;

    ret = bc_NEW("int3_obj", OBJ_TYPE_NUMBER, NUM_TYPE_INT);
    ASSERT_EQ(ret, EC_OK);
    obj_id3 = bc.bc_new_res.obj_id;

    for (int i = 0; i < ARRAY_SIZE(calc_val_map); i++) {
        ret = bc_MOV_val(obj_id1, &calc_val_map[i].val1);
        ASSERT_EQ(ret, EC_OK);

        ret = bc_MOV_val(obj_id2, &calc_val_map[i].val2);
        ASSERT_EQ(ret, EC_OK);

        ret = bc_MOV_val(obj_id3, &calc_val_map[i].val3);
        ASSERT_EQ(ret, EC_OK);
        
        ret = bc_CALC(calc_val_map[i].bc_op, obj_id1, obj_id2, obj_id3);
        EXPECT_EQ(ret, EC_OK);
        bc_check_obj(obj_id1, &calc_val_map[i].exp_val1);
        bc_check_obj(obj_id2, &calc_val_map[i].val2);
        bc_check_obj(obj_id3, &calc_val_map[i].val3);
    }
}

TEST_F(TestTestBC, bc_proc_CALC_float_success)
{
    u32 obj_id1, obj_id2, obj_id3;
    struct {
        enum test_bc_op_e bc_op;
        f64 val1;
        f64 val2;
        f64 val3;
        f64 exp_val1;
    } calc_val_map[] = {
        { TEST_BC_ADD, 123.123, 456.465, 789.789, 456.465 + 789.789 },
        { TEST_BC_SUB, 123.123, 456.465, 789.789, 456.465 - 789.789 },
        { TEST_BC_MUL, 123.123, 456.465, 789.789, 456.465 * 789.789 },
        { TEST_BC_DIV, 123.123, 456.465, 789.789, 456.465 / 789.789 },
    };
    int ret;

    ret = bc_NEW("float1_obj", OBJ_TYPE_NUMBER, NUM_TYPE_FLOAT);
    ASSERT_EQ(ret, EC_OK);
    obj_id1 = bc.bc_new_res.obj_id;

    ret = bc_NEW("float2_obj", OBJ_TYPE_NUMBER, NUM_TYPE_FLOAT);
    ASSERT_EQ(ret, EC_OK);
    obj_id2 = bc.bc_new_res.obj_id;

    ret = bc_NEW("float3_obj", OBJ_TYPE_NUMBER, NUM_TYPE_FLOAT);
    ASSERT_EQ(ret, EC_OK);
    obj_id3 = bc.bc_new_res.obj_id;

    for (int i = 0; i < ARRAY_SIZE(calc_val_map); i++) {
        ret = bc_MOV_val(obj_id1, &calc_val_map[i].val1);
        ASSERT_EQ(ret, EC_OK);

        ret = bc_MOV_val(obj_id2, &calc_val_map[i].val2);
        ASSERT_EQ(ret, EC_OK);

        ret = bc_MOV_val(obj_id3, &calc_val_map[i].val3);
        ASSERT_EQ(ret, EC_OK);
        
        ret = bc_CALC(calc_val_map[i].bc_op, obj_id1, obj_id2, obj_id3);
        EXPECT_EQ(ret, EC_OK);
        bc_check_obj(obj_id1, &calc_val_map[i].exp_val1);
        bc_check_obj(obj_id2, &calc_val_map[i].val2);
        bc_check_obj(obj_id3, &calc_val_map[i].val3);
    }
}

TEST_F(TestTestBC, bc_proc_CMP_int_success)
{
    u32 obj_id1, obj_id2, obj_id3;
    struct {
        s64 val1;
        s64 val2;
        s64 val3;
        s64 exp_val1;
    } cmp_val_map[] = {
        { 123, 789, 456, 1 },
        { 123, 456, 456, 0 },
        { 123, 456, 789, -1 },
    };
    int ret;

    ret = bc_NEW("int1_obj", OBJ_TYPE_NUMBER, NUM_TYPE_INT);
    ASSERT_EQ(ret, EC_OK);
    obj_id1 = bc.bc_new_res.obj_id;

    ret = bc_NEW("int2_obj", OBJ_TYPE_NUMBER, NUM_TYPE_INT);
    ASSERT_EQ(ret, EC_OK);
    obj_id2 = bc.bc_new_res.obj_id;

    ret = bc_NEW("int3_obj", OBJ_TYPE_NUMBER, NUM_TYPE_INT);
    ASSERT_EQ(ret, EC_OK);
    obj_id3 = bc.bc_new_res.obj_id;

    for (int i = 0; i < ARRAY_SIZE(cmp_val_map); i++) {
        ret = bc_MOV_val(obj_id1, &cmp_val_map[i].val1);
        ASSERT_EQ(ret, EC_OK);

        ret = bc_MOV_val(obj_id2, &cmp_val_map[i].val2);
        ASSERT_EQ(ret, EC_OK);

        ret = bc_MOV_val(obj_id3, &cmp_val_map[i].val3);
        ASSERT_EQ(ret, EC_OK);
        
        ret = bc_CMP(obj_id1, obj_id2, obj_id3);
        EXPECT_EQ(ret, EC_OK);
        bc_check_obj(obj_id1, &cmp_val_map[i].exp_val1);
        bc_check_obj(obj_id2, &cmp_val_map[i].val2);
        bc_check_obj(obj_id3, &cmp_val_map[i].val3);
    }
}

TEST_F(TestTestBC, bc_proc_CMP_float_success)
{
    u32 obj_id1, obj_id2, obj_id3;
    struct {
        s64 val1;
        f64 val2;
        f64 val3;
        s64 exp_val1;
    } cmp_val_map[] = {
        { 123, 789.789, 456.465, 1 },
        { 123, 456.465, 456.465, 0 },
        { 123, 456.465, 789.789, -1 },
    };
    int ret;

    ret = bc_NEW("int1_obj", OBJ_TYPE_NUMBER, NUM_TYPE_INT);
    ASSERT_EQ(ret, EC_OK);
    obj_id1 = bc.bc_new_res.obj_id;

    ret = bc_NEW("float2_obj", OBJ_TYPE_NUMBER, NUM_TYPE_FLOAT);
    ASSERT_EQ(ret, EC_OK);
    obj_id2 = bc.bc_new_res.obj_id;

    ret = bc_NEW("float3_obj", OBJ_TYPE_NUMBER, NUM_TYPE_FLOAT);
    ASSERT_EQ(ret, EC_OK);
    obj_id3 = bc.bc_new_res.obj_id;

    for (int i = 0; i < ARRAY_SIZE(cmp_val_map); i++) {
        ret = bc_MOV_val(obj_id1, &cmp_val_map[i].val1);
        ASSERT_EQ(ret, EC_OK);

        ret = bc_MOV_val(obj_id2, &cmp_val_map[i].val2);
        ASSERT_EQ(ret, EC_OK);

        ret = bc_MOV_val(obj_id3, &cmp_val_map[i].val3);
        ASSERT_EQ(ret, EC_OK);
        
        ret = bc_CMP(obj_id1, obj_id2, obj_id3);
        EXPECT_EQ(ret, EC_OK);
        bc_check_obj(obj_id1, &cmp_val_map[i].exp_val1);
        bc_check_obj(obj_id2, &cmp_val_map[i].val2);
        bc_check_obj(obj_id3, &cmp_val_map[i].val3);
    }
}