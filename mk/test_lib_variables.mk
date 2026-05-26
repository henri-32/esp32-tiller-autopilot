LIB_BUILD_DIR := build_lib

LIB_INCLUDE := -Ilib/Unity/src

LIB_C_SRC := lib/Unity/src/unity.c

LIB_CPP_SRC := 

LIB_OBJS := $(patsubst %.c,$(LIB_BUILD_DIR)/%.o,$(LIB_C_SRC)) \
	$(patsubst %.cpp,$(LIB_BUILD_DIR)/%.o,$(LIB_CPP_SRC))
