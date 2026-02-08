#pragma once

#include "globalTypes.h"
class ControlPanel {
public:
  enum class Mode { engaged, Disengaged };
  enum class Source { Compass, Wind, Gps };

  struct Intent {
    Mode mode = Mode::Disengaged;
    Source source = Source::Compass;
    uint16_t targetCourse = 0;
  };

  Intent readIntent() const;
  NavigationSource m_activeSource;
};