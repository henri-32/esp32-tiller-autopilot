#include <cstdint>

//This library is used to build hardwaretests. Each hardwaretest has to implement run_hardwaretest()
int run_hardwaretest();

extern "C" void app_main();

static bool testRan = false;
static bool askedForTest = false;
uint8_t test_return_val;

