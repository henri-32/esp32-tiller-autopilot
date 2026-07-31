#include "drivers/uartDriver.h"
#include "cobs-c/cobs.h"
#include "driver/uart.h"
#include "soc/gpio_num.h"

esp_err_t UartDriver::init()
/* The Systems Uart Configuration is centralized in this Driver instead of spreading them over
 * different config files, as there is no need to change the wiring of the interfaces
 */

{
  const uint16_t gateway_rx_buffer_size = 2048;
  const uint16_t gateway_tx_buffer_size = 1028;

  uart_config_t gatewayConfig = {
      .baud_rate = 115200,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .rx_flow_ctrl_thresh = 0,
  };

  if (uart_param_config(UART_NUM_0, &gatewayConfig) != ESP_OK)
  {
    return ESP_FAIL;
  }

  if (uart_driver_install(UART_NUM_0, gateway_rx_buffer_size, gateway_tx_buffer_size, 0, nullptr,
                          0) != ESP_OK)
  {
    return ESP_FAIL;
  }

  const uint16_t gps_rx_buffer_size = 512;
  const uint8_t gps_tx_buffer_size = 0;

  uart_config_t gpsConfig = {
      .baud_rate = 9600,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .rx_flow_ctrl_thresh = 0,
  };

  if (uart_param_config(UART_NUM_2, &gpsConfig) != ESP_OK)
  {
    return ESP_FAIL;
  }

  if (uart_driver_install(UART_NUM_2, gps_rx_buffer_size, gps_tx_buffer_size, 0, nullptr, 0) !=
      ESP_OK)
  {
    return ESP_FAIL;
  }

  if (uart_set_pin(UART_NUM_2, GPIO_NUM_17, GPIO_NUM_16, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE) !=
      ESP_OK)
  {
    return ESP_FAIL;
  }
  return ESP_OK;
};

int UartDriver::write(UartInterface channel, const void* src, size_t size)
{
  uart_port_t port;
  switch (channel)
  {
  case UartInterface::GATEWAY:
  {
    port = UART_NUM_0;
    constexpr uint8_t delimiter = 0x00;

    uint8_t encoded[COBS_ENCODE_DST_BUF_LEN_MAX(size)];
    cobs_encode_result result = cobs_encode(encoded, sizeof(encoded), src, size);
    if (result.status != COBS_ENCODE_OK)
    {
      return -1;
    }

    const int msg_write = uart_write_bytes(port, encoded, result.out_len);
    const int delimiter_write = uart_write_bytes(port, &delimiter, sizeof(delimiter));

    if (msg_write == static_cast<int>(result.out_len) &&
        delimiter_write == static_cast<int>(sizeof(delimiter)))
    {
      return 0;
    }
    else
    {
      return -1;
    }
  }

  case UartInterface::GPS:
  {
    // currently no write operation towards the gps interface implemented
    return -1;
  }
  }
  return -1;
};

int UartDriver::read(UartInterface channel, void* dest, uint16_t dest_buf_len,
                     TickType_t timeout) const
{
  uart_port_t port;
  switch (channel)
  {
  case UartInterface::GATEWAY:
  {
    port = UART_NUM_0;
    while (true)
    {
      uint8_t byte = 0;
      if (uart_read_bytes(port, &byte, sizeof(byte), timeout) !=
          static_cast<int>(sizeof(byte)))
      {
        return -1;
      }

      if (byte != 0x00)
      {
        if (discard_gateway_frame_)
        {
          continue;
        }

        if (gateway_encoded_length_ >= gateway_encoded_buffer_size_)
        {
          gateway_encoded_length_ = 0;
          discard_gateway_frame_ = true;
          continue;
        }

        gateway_encoded_buffer_[gateway_encoded_length_++] = byte;
        continue;
      }

      if (discard_gateway_frame_)
      {
        gateway_encoded_length_ = 0;
        discard_gateway_frame_ = false;
        continue;
      }

      if (gateway_encoded_length_ == 0)
      {
        continue;
      }

      const cobs_decode_result dec_res =
          cobs_decode(dest, dest_buf_len, gateway_encoded_buffer_, gateway_encoded_length_);
      gateway_encoded_length_ = 0;
      if (dec_res.status != COBS_DECODE_OK)
      {
        return -1;
      }

      return static_cast<int>(dec_res.out_len);
    }
  }
  break;

  case UartInterface::GPS:
  {
    port = UART_NUM_2;
    int bytes_read = uart_read_bytes(port, dest, dest_buf_len, timeout);
    if (bytes_read <= 0)
    {
      return -1;
    }

    return bytes_read;
  }
  break;
  }
  return -1;
};
