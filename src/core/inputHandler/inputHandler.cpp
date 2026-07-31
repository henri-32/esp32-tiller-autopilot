#include "core/inputHandler.h"
#include "drivers/uartDriver.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "protocol/autopilotWireProtocol.h"
#include "protocol/internalMessageProtocol.h"
#include "telemetry/runtimeTypes.h"

void vInputHandlerTask(void* pvParameters)
{
  InputHandler* inputHandler = static_cast<InputHandler*>(pvParameters);
  constexpr uint16_t input_message_size =
      MessageOffsets::payload + InputHandlePayloadOffsets::payload_length;
  uint8_t read_buffer[input_message_size];

  while (true)
  {
    // Blocking read from Uart Buffer
    const int bytes_read = inputHandler->uartDriver_->read(UartInterface::GATEWAY, read_buffer,
                                                           sizeof(read_buffer), portMAX_DELAY);
    if (bytes_read != static_cast<int>(input_message_size))
    {
      continue;
    }

    // Get Message from buffer
    const Message<InputHandleData_t> msg = mp_read_InputHandleMessage_from_buffer(
        read_buffer, static_cast<uint16_t>(bytes_read));
    if (msg.header.type != static_cast<uint8_t>(CommandType::InputHandleData))
    {
      continue;
    }

    TelemetryMessage int_msg = initialize_TelemetryMessage_with_payload(msg.payload);

    xQueueSend(inputHandler->queueBundle_.telemetry_msg, &int_msg, 0);
  }
};

esp_err_t InputHandler::init()
{
  queueBundle_ = queueServer_->get_telemetry_bundle();
  if (uartDriver_ == nullptr || queueBundle_.telemetry_msg == nullptr ||
      queueBundle_.runtime_log == nullptr)
  {
    runtimeLogMessage_.payload.init_status = static_cast<uint8_t>(InitStatus::FAIL);
    if (queueBundle_.runtime_log != nullptr)
    {
      xQueueSend(queueBundle_.runtime_log, &runtimeLogMessage_, pdMS_TO_TICKS(100));
    }
    return ESP_FAIL;
  }

  if (xTaskCreate(vInputHandlerTask, "inputHandler", 10000, this, 5, nullptr) == pdPASS)
  {
    runtimeLogMessage_.payload.init_status = static_cast<uint8_t>(InitStatus::OK);
    xQueueSend(queueBundle_.runtime_log, &runtimeLogMessage_, pdMS_TO_TICKS(100));
    return ESP_OK;
  }
  else
  {
    runtimeLogMessage_.payload.init_status = static_cast<uint8_t>(InitStatus::FAIL);
    xQueueSend(queueBundle_.runtime_log, &runtimeLogMessage_, pdMS_TO_TICKS(100));
    return ESP_FAIL;
  }
};
