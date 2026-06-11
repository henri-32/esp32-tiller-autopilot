$(LIB_BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@) 
	@echo 'library rebuild...'
	@$(CC) -c $< $(LIB_INCLUDE) -o $@
	@echo 'CC $< -> $@'

$(LIB_BUILD_DIR)/%.o: lib/%.cpp
	@mkdir -p $(dir $@) 
	@$(CXX) -c $(LIB_SRC) $(LIB_INCLUDE) -o $@
	@echo 'CACHED_CXX $< -> $@'

