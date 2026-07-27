#include "core/inputHandler/inputHandler.h"
#include "cobs-c/cobs.h"
#include "drivers/uartDriver.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "logging/message_protocol.h"

static struct Context
{
  void* THIS = nullptr;
  QueueBundle_t queueBundle;
  UartDriver* uartDriver;
} context;

void vInputHandlerTask(void* pvParameters)
{
  QueueBundle_t bundle = static_cast<Context*>(pvParameters)->queueBundle;
  UartDriver* uartDriver = static_cast<Context*>(pvParameters)->uartDriver;
  uint8_t read_buffer[1024];

  uint8_t decoded_buffer[256];
  uint16_t decoded_len = 0;
  uint8_t encoded_buffer[256];
  uint16_t encoded_len = 0;
  uint8_t byte = 0;
  while (true)
  {
    // Read from Uart Buffer
    uint16_t bytes_read = uartDriver->read(UartInterface::USB_INTERFACE, read_buffer,
                                           sizeof(read_buffer), portMAX_DELAY);

    // COBS Decode Buffer
    for (int i = 0; i < bytes_read; ++i)
    {
      byte = read_buffer[i];

      if (byte != 0x00)
      {
        if (encoded_len >= sizeof(encoded_buffer))
        {
          encoded_len = 0;
          continue;
        }
        encoded_buffer[encoded_len] = byte;
        encoded_len++;
        continue;
      }

      if (encoded_len == 0)
      {
        continue;
      }

      // This Code is reachable with byte == 0x00 so encoded_buffer is a complete COBS encoded
      // Message
      cobs_decode_result dec_res =
          cobs_decode(decoded_buffer, sizeof(decoded_buffer), encoded_buffer, encoded_len);
      decoded_len = dec_res.out_len;
      encoded_len = 0;
      if (dec_res.status != COBS_DECODE_OK)
      {
        continue;
      }

     if (decoded_len < sizeof(Message<InputHandleData>))
      {
        continue;
      }

      // Get Message from buffer
      Message<InputHandleData> msg =
          mp_read_InputHandleMessage_from_buffer(decoded_buffer, decoded_len);

      // send the InputHandleData into the Queue
      xQueueSend(bundle.data, &msg.payload, 0);
    }
  }
};

esp_err_t InputHandler::init()
{
  Context context;
  context.THIS = this;
  context.queueBundle = queueServer_->get_input_handle();

  xTaskCreate(vInputHandlerTask, "inputHandler", 10000, &context, 5, nullptr);

  if (context.queueBundle.data != nullptr)
  {
    return ESP_OK;
  }
  else
  {
    return ESP_FAIL;
  }
};
