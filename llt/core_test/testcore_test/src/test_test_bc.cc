#include "gtest/gtest.h"
#include "mockcpp/mokc.h"

extern "C" {
#include "test_core.h"
#include "error_code.h"
#include "leafpy_cfg.h"
}

TEST(TestBC, test)
{
    EXPECT_TRUE(true);
}