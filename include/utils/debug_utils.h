#pragma once

#ifdef MEM_LOG

#define ADD_MEMORY_VALUES() uint16_t free_task_stack = 0

#define WRITE_FREE_TASK_STACK_TO_CONTEXT(CONTEXT) \
    do \
		{ \
	    const uint16_t mark = uxTaskGetStackHighWaterMark(nullptr); \
		(CONTEXT)->free_task_stack = (mark <= 65000) ? mark : 64999; \
		} \
while (0) 


#else 
#define WRITE_FREE_TASK_STACK_TO_CONTEXT(CONTEXT) do {} while (0)
#define ADD_MEMORY_VALUES()
#endif
