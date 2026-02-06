class PWMController {
public:
  enum class Direction { Neutral, Left, Right };
  enum class Force { Low, Medium, High };

  PWMController() = default;
  PWMController(const PWMController &) = delete;
  PWMController &operator=(const PWMController &) = delete;

  void setImpulse(Direction dir, Force frc);

private:
  Direction m_lastDir = Direction::Neutral;
  Force m_lastFrc = Force::Low;
};