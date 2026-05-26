MODE ?= TEST
# TEST for no additional flags 
# DEBUG for debug symbols 
# SAN for sanitizer symbols 


DEBUG_FLAGS := -g -O0 
SAN_FLAGS := -fsanitize=address -fno-omit-frame-pointer


