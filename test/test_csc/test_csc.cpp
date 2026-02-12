#include <unity.h>
#include "core/steering/csc.h"
#include "core/config.h"
#include <cstdint>

SteeringController_Config config;
CoreSteeringController csc(config);

void setUp() {}
void tearDown() {}

void test_no_correction_inside_tolerance()
{
    csc.setInternalTarget(100);

    for (int i = 0; i < 100; i++)
    {
        csc.currentHDG(105);  // +5°, default tolerance = 10°
        auto intent = csc.tick(i * 1000);
        TEST_ASSERT_FALSE(intent.has_value());
    }
}

void test_correction_outside_tolerance()
{
    SteeringController_Config config;
    CoreSteeringController csc(config);

    csc.setInternalTarget(340);

    bool intentOccurred = false;
    uint32_t time = 0;

    for (int i = 0; i < 120; i++)  // 120 Sekunden Simulation
    {
        time += 1000;              // 1 Hz Takt

        csc.currentHDG(5);       // +20° Fehler konstant

        auto intent = csc.tick(time);

        if (intent.has_value())
        {
            intentOccurred = true;

            // Optional: Richtung prüfen
            TEST_ASSERT_EQUAL(SteeringDirection::Left, intent->dir);

            break; // wir brauchen nur einen erfolgreichen Intent
        }
    }

    TEST_ASSERT_TRUE(intentOccurred);
}


int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_no_correction_inside_tolerance);
    RUN_TEST(test_correction_outside_tolerance);
    return UNITY_END();
}
