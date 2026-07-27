#include "drivers/uartDriver.h"
#include "cobs-c/cobs.h"
#include "driver/uart.h"

esp_err_t UartDriver::init()
{
  const uint16_t rx_buffer_size = 2048;
  const uint16_t tx_buffer_size = 1028;

  uart_config_t uart_config = {
      .baud_rate = 115200,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .rx_flow_ctrl_thresh = 0,
  };

  esp_err_t conf = uart_param_config(UART_NUM_0, &uart_config);
  esp_err_t install =
      uart_driver_install(UART_NUM_0, rx_buffer_size, tx_buffer_size, 0, nullptr, 0);

  if (conf == ESP_OK && install == ESP_OK)
  {
    return ESP_OK;
  }
  else
  {
    return ESP_FAIL;
  }
};

int UartDriver::write(UartInterface channel, const void* src, size_t size)
{
  uint8_t delimiter = 0x00;
  uart_port_t port;

  auto write_delimiter = [delimiter, port]()
  { return uart_write_bytes(port, &delimiter, sizeof(delimiter)); };

  if (channel == UartInterface::USB_INTERFACE)
  {
    uint8_t encoded[COBS_ENCODE_DST_BUF_LEN_MAX(size)];
    cobs_encode_result result = cobs_encode(encoded, sizeof(encoded), src, size);
    if (result.status != COBS_ENCODE_OK)
    {
      return -1;
    }

    port = UART_NUM_0;

    esp_err_t msg_write = uart_write_bytes(port, encoded, result.out_len);
    esp_err_t del_write = write_delimiter();

    if (msg_write == ESP_OK && del_write == ESP_OK)
    {
      return 0;
    }
    else
    {
      return -1;
    }
  }
  // Currently only USB_INTERFACE is supported
  else
  {
    return -1;
  }
};

int UartDriver::read(UartInterface channel, void* dest, uint16_t dest_buf_len, TickType_t timeout)
{
  uart_port_t port;
  if (channel == UartInterface::USB_INTERFACE)
  {
    port = UART_NUM_0;
    uint8_t buf_enc[1024];

    if (uart_read_bytes(port, buf_enc, sizeof(buf_enc), timeout) <= 0)
    {
      return -1;
    }
    else
    {

      cobs_decode_result dec_res = cobs_decode(buf_enc, sizeof(buf_enc), dest, dest_buf_len);
      if (dec_res.status == COBS_DECODE_OK)
      {
        return dec_res.out_len;
      }
      else
      {
        return -1;
      }
    }
  }
// currently only USB_INTERFACE is supported
  else
  {
    return -1;
  }
};
