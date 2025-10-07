#include "gtest/gtest.h"
#include "mockcpp/mokc.h"

extern "C" {
#include "test_bc.h"
#include "test_core.h"
#include "error_code.h"
#include "leafpy_cfg.h"
}

class TestTestBC : public testing::Test {
protected:
    test_bc_s bc = {0};
    static u64 pc;

    static int bc_cb(test_bc_s *bc)
    {
        printf("bc_cb: op[%u] arg1[0x%llx(%llu)] arg2[0x%llx(%llu)] arg3[0x%llx(%llu)] "
            "bc_res[0x%llx(%llu)] pos[0x%llx] next_pos[0x%llx]\n",
            bc->op, bc->args.arg[0], bc->args.arg[0], bc->args.arg[1], bc->args.arg[1], bc->args.arg[2],
            bc->args.arg[2], bc->bc_res, bc->bc_res, bc->pos, bc->next_pos);
        pc++;
        return EC_OK;
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
        return test_bc_proc(&bc, bc_cb);
    }

    int bc_NEW(const char *obj_name, u8 obj_type, u8 obj_subtype)
    {
        bc_BC_init(TEST_BC_NEW);
        bc.args.obj_type1 = obj_type;
        bc.args.obj_subtype1 = obj_subtype;
        bc.args.bc_new.obj_name = (char *)obj_name;
        return test_bc_proc(&bc, bc_cb);
    }

    int bc_DEL(u32 obj_id)
    {
        bc_BC_init(TEST_BC_DEL);
        bc.args.bc_del.obj_id = obj_id;
        return test_bc_proc(&bc, bc_cb);
    }

    int bc_MOV_obj(u32 obj_id1, u32 obj_id2)
    {
        bc_BC_init(TEST_BC_MOV);
        bc.sub_op = TEST_BC_MOV_OBJ;
        bc.args.bc_mov.obj_id1 = obj_id1;
        bc.args.bc_mov.obj_id2 = obj_id2;
        return test_bc_proc(&bc, bc_cb);
    }

    int bc_MOV_val(u32 obj_id1, u64 val)
    {
        bc_BC_init(TEST_BC_MOV);
        bc.sub_op = TEST_BC_MOV_VAL;
        bc.args.bc_mov.obj_id1 = obj_id1;
        bc.args.bc_mov.val = val;
        return test_bc_proc(&bc, bc_cb);
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
        GlobalMockObject::verify();
        pc = 0;
    }
};

u64 TestTestBC::pc = 0;

TEST_F(TestTestBC, bc_proc_NOP_success)
{
    int ret = 0;
    ret = bc_NOP();
    EXPECT_EQ(ret, EC_OK);
}

TEST_F(TestTestBC, bc_proc_NEW_success)
{
    int ret = 0;
    ret = bc_NEW("int_obj1", OBJ_TYPE_NUMBER, NUM_TYPE_INT);
    EXPECT_EQ(ret, EC_OK);
}

TEST_F(TestTestBC, bc_proc_DEL_success)
{
    u32 obj_id;
    int ret = 0;

    ret = bc_NEW("int_obj1", OBJ_TYPE_NUMBER, NUM_TYPE_INT);
    ASSERT_EQ(ret, EC_OK);
    obj_id = bc.bc_new_res.obj_id;

    ret = bc_DEL(obj_id);
    EXPECT_EQ(ret, EC_OK);
}

TEST_F(TestTestBC, bc_proc_MOV_success)
{
    u32 obj_id1, obj_id2;
    int ret = 0;

    ret = bc_NEW("int_obj1", OBJ_TYPE_NUMBER, NUM_TYPE_INT);
    ASSERT_EQ(ret, EC_OK);
    obj_id1 = bc.bc_new_res.obj_id;
    
    ret = bc_NEW("int_obj2", OBJ_TYPE_NUMBER, NUM_TYPE_INT);
    ASSERT_EQ(ret, EC_OK);
    obj_id2 = bc.bc_new_res.obj_id;

    // MOV VAL
    ret = bc_MOV_val(obj_id1, 123);
    EXPECT_EQ(ret, EC_OK);
    ret = bc_MOV_val(obj_id2, 456);
    EXPECT_EQ(ret, EC_OK);

    // MOV OBJ
    ret = bc_MOV_obj(obj_id1, obj_id2);
    EXPECT_EQ(ret, EC_OK);
}