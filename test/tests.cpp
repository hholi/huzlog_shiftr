// tests.cpp
#include "../huz/dummy.cpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
 
TEST(InitialTest, SpecialNo) { 
    ASSERT_EQ(42, dummyF());
}
 
 
int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
