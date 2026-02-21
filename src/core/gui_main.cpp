#include "core/systemController.h"

#include <chrono>
#include <cstdint>
#include <thread>

int main() {
  SystemController systemController;
  const auto t0 = std::chrono::steady_clock::now();

  while (true) {
    const auto now = std::chrono::steady_clock::now();
    const auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(now - t0).count();
    const auto loopTimestamp = static_cast<uint32_t>(elapsed);

    systemController.tick(loopTimestamp);

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
  }
}
