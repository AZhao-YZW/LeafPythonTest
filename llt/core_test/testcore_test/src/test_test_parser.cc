#include "gtest/gtest.h"
#include "mockcpp/mokc.h"
#include <string.h>

extern "C" {
#include "error_code.h"
#include "leafpy_cfg.h"
#include "test_parser.h"
}

class TestTestParser : public testing::Test {
protected:
    test_frame_callback_s frame_cb;

    static int run_frame_stub(u8 core_id, test_frame_s *frame)
    {
        return EC_OK;
    }

    virtual void SetUp()
    {
        int ret;
        frame_cb.run_frame = run_frame_stub;
        ret = test_frame_register(TEST_FRAME_QUEUE_0, 0, &frame_cb);
        ASSERT_EQ(ret, EC_OK);
    }

    virtual void TearDown()
    {
        test_frame_free_all();
        GlobalMockObject::verify();
    }
};

TEST_F(TestTestParser, parser_parse_code_success)
{
    char code[] = "a = 1";
    int ret = test_parser_parse_code(code, strlen(code));
    EXPECT_EQ(ret, EC_OK);
}

TEST_F(TestTestParser, parser_parse_code_null)
{
    char code[] = "a = 1";
    int ret = test_parser_parse_code(NULL, strlen(code));
    EXPECT_EQ(ret, EC_PARAM_INVALID);
}