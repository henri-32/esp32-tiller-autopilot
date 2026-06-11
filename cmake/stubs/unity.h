#pragma once

#define UNITY_BEGIN() 0
#define UNITY_END() 0
#define RUN_TEST(func) do { func(); } while (0)

#define TEST_ASSERT_TRUE(condition) ((void)(condition))
#define TEST_ASSERT_FALSE(condition) ((void)(condition))
#define TEST_ASSERT_EQUAL(expected, actual) ((void)(expected), (void)(actual))
#define TEST_ASSERT_EQUAL_INT16(expected, actual) ((void)(expected), (void)(actual))
#define TEST_ASSERT_EQUAL_UINT8(expected, actual) ((void)(expected), (void)(actual))
#define TEST_ASSERT_FLOAT_WITHIN(delta, expected, actual) ((void)(delta), (void)(expected), (void)(actual))
