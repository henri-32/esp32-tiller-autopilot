include mk/global_variables.mk

TEST_BUILD_DIR := build_test/$(MODE)

TEST_BIN := build_test/unit_tests

ifeq ($(MODE), TEST)
	TEST_CXX_FLAGS := -std=c++20 -Wall -Wextra 

else ifeq ($(MODE), DEBUG) 
	TEST_CXX_FLAGS := -std=c++20 -Wall -Wextra $(DEBUG_FLAGS) 

else ifeq ($(MODE), SAN) 
	TEST_CXX_FLAGS := -std=c++20 -Wall -Wextra $(SAN_FLAGS)

else 
	$(error Unknown MODE '$(MODE)')
endif


TEST_INCLUDE := -Iinclude

#UNIT_TESTS
TEST_SRC := \
	test/csc_unit_test.cpp 

TEST_OBJS := $(patsubst %.cpp,$(TEST_BUILD_DIR)/%.o,$(TEST_SRC))

#PRODUCTIVE CODE THATS TESTED
SRC_UNDER_TEST := \
	src/core/steering/csc/csc.cpp \
	src/core/steering/csc/deadband.cpp \
	src/core/steering/csc/headingErrorCalculator.cpp \
	src/core/steering/csc/observationBuffer.cpp \
	src/core/steering/csc/steeringGuard.cpp


SRC_UNDER_TEST_OBJS := $(patsubst %.cpp,$(TEST_BUILD_DIR)/%.o,$(SRC_UNDER_TEST))
