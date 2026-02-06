#include <Arduino.h>

class PWMController;

class CoreSteeringController {
public:
  struct Courses {
    uint16_t targetCourse = 0;
    uint16_t currentCourse = 0;
  };

  explicit CoreSteeringController(PWMController &pwm);
  void setTargetCourse(uint16_t target);
  void setCurrentCourse(uint16_t current);

  void computeSteeringAction();

private:
  int8_t getCorrectionInDegrees(uint16_t current, uint16_t target);
  PWMController &m_pwm;
  Courses m_courses{};
};