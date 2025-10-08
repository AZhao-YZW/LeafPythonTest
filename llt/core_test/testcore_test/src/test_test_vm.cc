#include "gtest/gtest.h"
#include "mockcpp/mockcpp.hpp"

extern "C" {
#include "error_code.h"
#include "leafpy_cfg.h"
#include "test_vm.h"
#include "test_obj_type.h"
#include "test_bc.h"
#include "test_core.h"
}

class TestTestVM : public testing::Test {
protected:

    test_frame_s *vm_create_frame(test_bc_s *bc_list, u32 bc_num)
    {
        test_frame_s *frame = test_frame_create(bc_num);
        if (frame == NULL) {
            return NULL;
        }
        for (int i = 0; i < bc_num; i++) {
            frame->bc_list[i] = bc_list[i];
        }
        return frame;
    }

    void vm_fill_bc_NEW(u32 obj_vm_id1, u8 t, u8 st, const char *obj_name, u8 c, u8 p, test_bc_s *bc)
    {
        test_bc_new_fill(t, st, obj_name, c, p, bc);
        bc->obj_vm_id1 = obj_vm_id1;
    }

    void vm_fill_bc_DEL(u32 obj_vm_id1, u8 c, u8 p, test_bc_s *bc)
    {
        test_bc_del_fill(c, p, bc);
        bc->obj_vm_id1 = obj_vm_id1;
    }

    void vm_fill_bc_MOV_VAL(u32 obj_vm_id1, const void *val, u8 c, u8 p, test_bc_s *bc)
    {
        test_bc_mov_val_fill(val, c, p, bc);
        bc->obj_vm_id1 = obj_vm_id1;
        bc->exp_val = val;
    }

    void vm_fill_bc_MOV_OBJ(u32 obj_vm_id1, u32 obj_vm_id2, u8 c, u8 p, test_bc_s *bc,
        const void *exp_val)
    {
        test_bc_mov_obj_fill(c, p, bc);
        bc->obj_vm_id1 = obj_vm_id1;
        bc->obj_vm_id2 = obj_vm_id2;
        bc->exp_val = exp_val;
    }

    void vm_fill_bc_ADD(u32 obj_vm_id1, u32 obj_vm_id2, u32 obj_vm_id3, u8 c, u8 p, test_bc_s *bc,
        const void *exp_val)
    {
        test_bc_calc_fill(TEST_BC_ADD, c, p, bc);
        bc->obj_vm_id1 = obj_vm_id1;
        bc->obj_vm_id2 = obj_vm_id2;
        bc->obj_vm_id3 = obj_vm_id3;
        bc->exp_val = exp_val;
    }

    void vm_fill_bc_PRINT(u32 obj_vm_id1, u8 c, u8 p, test_bc_s *bc)
    {
    }

    static void vm_get_obj_val(u8 core_id, u8 obj_id, void *val)
    {
        test_core_op_info_s op_info = {0};
        int ret;
        // get obj_id1 value
        test_core_op_get_fill(obj_id, val, &op_info);
        ret = test_core_run(core_id, &op_info);
        EXPECT_EQ(ret, EC_OK);
    }
    static void test_vm_bc_post_check_obj_mock(test_bc_s *bc)
    {
        union {
            s64 int_val;
            f64 float_val;
        } get_val = {0};

        if (bc->op == TEST_BC_MOV) {
            vm_get_obj_val(bc->core_id, bc->args.bc_2_args.obj_id1, &get_val);
        } else if (bc->op == TEST_BC_ADD) {
            vm_get_obj_val(bc->core_id, bc->args.bc_3_args.obj_id1, &get_val);
        } else {
            return;
        }
        if (bc->args.obj_type1 == OBJ_TYPE_NUMBER && bc->args.obj_subtype1 == NUM_TYPE_INT) {
            EXPECT_EQ(get_val.int_val, *(s64 *)bc->exp_val);
        } else if (bc->args.obj_type1 == OBJ_TYPE_NUMBER && bc->args.obj_subtype1 == NUM_TYPE_FLOAT) {
            EXPECT_EQ(get_val.float_val, *(f64 *)bc->exp_val);
        }
    }

    virtual void SetUp()
    {
        int ret;

        MOCKER(test_vm_bc_post_check_obj)
            .stubs()
            .will(invoke(test_vm_bc_post_check_obj_mock));

        ret = test_core_init(LEAFPY_DEFAULT_CORE_ID);
        ASSERT_EQ(ret, EC_OK);
        ret = test_vm_init(LEAFPY_DEFAULT_CORE_ID, TEST_FRAME_QUEUE_0);
        ASSERT_EQ(ret, EC_OK);
    }

    virtual void TearDown()
    {
        test_core_free_all();
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

TEST_F(TestTestVM, vm_run_frame_py_int_1_plus_2_success)
{
    test_bc_s bc_list[10] = {0};
    test_frame_s *frame = NULL;
    u8 core_id = LEAFPY_DEFAULT_CORE_ID, parent_id = ROOT_OBJ_ID;
    u32 vm_id1, vm_id2, vm_id3;
    s64 val, exp_val;
    int ret, i = 0;

    /**
     * python:
     *      1 + 2
     * 
     * bytecode:
     * [0]  NEW "anon_1" int
     * [1]  NEW "anon_2" int
     * [2]  NEW "anon_3" int
     * [3]  MOV <anon_1> 1
     * [4]  MOV <anon_2> 2
     * [5]  ADD <anon_3> <anon_1> <anon_2>
     * [6]  (PRINT <anon_3>)
     * [7]  DEL <anon_1>
     * [8]  DEL <anon_2>
     * [9]  DEL <anon_3>
     * 
     * vm_id_map:
     * [0]  <anon_1>
     * [1]  <anon_2>
     * [2]  <anon_3>
     */
    vm_id1 = 0;
    vm_id2 = 1;
    vm_id3 = 2;
    vm_fill_bc_NEW(vm_id1, OBJ_TYPE_NUMBER, NUM_TYPE_INT, "anon_1", core_id, parent_id, &bc_list[i++]);
    vm_fill_bc_NEW(vm_id2, OBJ_TYPE_NUMBER, NUM_TYPE_INT, "anon_2", core_id, parent_id, &bc_list[i++]);
    vm_fill_bc_NEW(vm_id3, OBJ_TYPE_NUMBER, NUM_TYPE_INT, "anon_3", core_id, parent_id, &bc_list[i++]);
    val = 1;
    vm_fill_bc_MOV_VAL(vm_id1, &val, core_id, parent_id, &bc_list[i++]);
    val = 2;
    vm_fill_bc_MOV_VAL(vm_id1, &val, core_id, parent_id, &bc_list[i++]);
    exp_val = 3;
    vm_fill_bc_ADD(vm_id3, vm_id1, vm_id2, core_id, parent_id, &bc_list[i++], &exp_val);
    vm_fill_bc_PRINT(vm_id1, core_id, parent_id, &bc_list[i++]);
    vm_fill_bc_DEL(vm_id1, core_id, parent_id, &bc_list[i++]);
    vm_fill_bc_DEL(vm_id2, core_id, parent_id, &bc_list[i++]);
    vm_fill_bc_DEL(vm_id3, core_id, parent_id, &bc_list[i++]);

    // run frame
    frame = vm_create_frame(bc_list, ARRAY_SIZE(bc_list));
    ASSERT_NE(frame, (void *)NULL);

    ret = test_vm_run_frame(LEAFPY_DEFAULT_CORE_ID, frame);
    EXPECT_EQ(ret, EC_OK);
}

TEST_F(TestTestVM, vm_run_frame_py_float_1_plus_2_plus_3_success)
{
    test_bc_s bc_list[13] = {0};
    test_frame_s *frame = NULL;
    u8 core_id = LEAFPY_DEFAULT_CORE_ID, parent_id = ROOT_OBJ_ID;
    u32 vm_id1, vm_id2, vm_id3;
    f64 val, exp_val;
    int ret, i = 0;

    /**
     * python:
     *      1 + 2 + 3
     * 
     * bytecode:
     * [0]  NEW "anon_1" float
     * [1]  NEW "anon_2" float
     * [2]  NEW "anon_3" float
     * [3]  MOV <anon_1> 1.0
     * [4]  MOV <anon_2> 2.0
     * [5]  ADD <anon_3> <anon_1> <anon_2>
     * [6]  MOV <anon_1> <anon_3>
     * [7]  MOV <anon_2> 3.0
     * [8]  ADD <anon_3> <anon_1> <anon_2>
     * [9]  (PRINT <anon_3>)
     * [10] DEL <anon_1>
     * [11] DEL <anon_2>
     * [12] DEL <anon_3>
     * 
     * vm_id_map:
     * [0]  <anon_1>
     * [1]  <anon_2>
     * [2]  <anon_3>
     */
    vm_id1 = 0;
    vm_id2 = 1;
    vm_id3 = 2;
    vm_fill_bc_NEW(vm_id1, OBJ_TYPE_NUMBER, NUM_TYPE_FLOAT, "anon_1", core_id, parent_id, &bc_list[i++]);
    vm_fill_bc_NEW(vm_id2, OBJ_TYPE_NUMBER, NUM_TYPE_FLOAT, "anon_2", core_id, parent_id, &bc_list[i++]);
    vm_fill_bc_NEW(vm_id3, OBJ_TYPE_NUMBER, NUM_TYPE_FLOAT, "anon_3", core_id, parent_id, &bc_list[i++]);
    val = 1.0;
    vm_fill_bc_MOV_VAL(vm_id1, &val, core_id, parent_id, &bc_list[i++]);
    val = 2.0;
    vm_fill_bc_MOV_VAL(vm_id1, &val, core_id, parent_id, &bc_list[i++]);
    exp_val = 3.0;
    vm_fill_bc_ADD(vm_id1, vm_id2, vm_id3, core_id, parent_id, &bc_list[i++], &exp_val);
    vm_fill_bc_MOV_OBJ(vm_id1, vm_id3, core_id, parent_id, &bc_list[i++], &exp_val);
    val = 3.0;
    vm_fill_bc_MOV_VAL(vm_id1, &val, core_id, parent_id, &bc_list[i++]);
    exp_val = 6.0;
    vm_fill_bc_ADD(vm_id1, vm_id2, vm_id3, core_id, parent_id, &bc_list[i++], &exp_val);
    vm_fill_bc_PRINT(vm_id1, core_id, parent_id, &bc_list[i++]);
    vm_fill_bc_DEL(vm_id1, core_id, parent_id, &bc_list[i++]);
    vm_fill_bc_DEL(vm_id2, core_id, parent_id, &bc_list[i++]);
    vm_fill_bc_DEL(vm_id3, core_id, parent_id, &bc_list[i++]);

    // run frame
    frame = vm_create_frame(bc_list, ARRAY_SIZE(bc_list));
    ASSERT_NE(frame, (void *)NULL);

    ret = test_vm_run_frame(LEAFPY_DEFAULT_CORE_ID, frame);
    EXPECT_EQ(ret, EC_OK);
}