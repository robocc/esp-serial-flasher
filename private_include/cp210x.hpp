#pragma once
#include "esp_check.h"
#include "usb/usb_types_ch9.h"

#define CP210X_VID 0x10c4
#define CP210X_PID 0xea60

// @see AN571: CP210x Virtual COM Port Interface, chapter 5
#define CP210X_CMD_IFC_ENABLE (0x00)      // Enable or disable the interface
#define CP210X_CMD_SET_BAUDDIV (0x01)     // Set the baud rate divisor
#define CP210X_CMD_GET_BAUDDIV (0x02)     // Get the baud rate divisor
#define CP210X_CMD_SET_LINE_CTL (0x03)    // Set the line control
#define CP210X_CMD_GET_LINE_CTL (0x04)    // Get the line control
#define CP210X_CMD_SET_BREAK (0x05)       // Set a BREAK
#define CP210X_CMD_IMM_CHAR (0x06)        // Send character out of order
#define CP210X_CMD_SET_MHS (0x07)         // Set modem handshaking
#define CP210X_CMD_GET_MDMSTS (0x08)      // Get modem status
#define CP210X_CMD_SET_XON (0x09)         // Emulate XON
#define CP210X_CMD_SET_XOFF (0x0A)        // Emulate XOFF
#define CP210X_CMD_SET_EVENTMASK (0x0B)   // Set the event mask
#define CP210X_CMD_GET_EVENTMASK (0x0C)   // Get the event mask
#define CP210X_CMD_GET_EVENTSTATE (0x16)  // Get the event state
#define CP210X_CMD_SET_RECEIVE (0x17)     // Set receiver max timeout
#define CP210X_CMD_GET_RECEIVE (0x18)     // Get receiver max timeout
#define CP210X_CMD_SET_CHAR (0x0D)        // Set special character individually
#define CP210X_CMD_GET_CHARS (0x0E)       // Get special characters
#define CP210X_CMD_GET_PROPS (0x0F)       // Get properties
#define CP210X_CMD_GET_COMM_STATUS (0x10) // Get the serial status
#define CP210X_CMD_RESET (0x11)           // Reset
#define CP210X_CMD_PURGE (0x12)           // Purge
#define CP210X_CMD_SET_FLOW (0x13)        // Set flow control
#define CP210X_CMD_GET_FLOW (0x14)        // Get flow control
#define CP210X_CMD_EMBED_EVENTS                                                \
  (0x15) // Control embedding of events in the data stream
#define CP210X_CMD_GET_BAUDRATE (0x1D)    // Get the baud rate
#define CP210X_CMD_SET_BAUDRATE (0x1E)    // Set the baud rate
#define CP210X_CMD_SET_CHARS (0x19)       // Set special characters
#define CP210X_CMD_VENDOR_SPECIFIC (0xFF) // Read/write latch values

#define CP210X_READ_REQ                                                        \
  (USB_BM_REQUEST_TYPE_TYPE_VENDOR | USB_BM_REQUEST_TYPE_RECIP_INTERFACE |     \
   USB_BM_REQUEST_TYPE_DIR_IN)
#define CP210X_WRITE_REQ                                                       \
  (USB_BM_REQUEST_TYPE_TYPE_VENDOR | USB_BM_REQUEST_TYPE_RECIP_INTERFACE |     \
   USB_BM_REQUEST_TYPE_DIR_OUT)

esp_err_t cp210_init(cdc_acm_dev_hdl_t cdc) {
  ESP_LOGI("cp210x", "Enabling CP210x interface");
  ESP_RETURN_ON_ERROR(cdc_acm_host_send_custom_request(cdc, CP210X_WRITE_REQ,
                                                       CP210X_CMD_IFC_ENABLE, 1,
                                                       0, 0, NULL),
                      "CP210X", );
  uint32_t baudrate = 921600;
  ESP_RETURN_ON_ERROR(cdc_acm_host_send_custom_request(
                          cdc, CP210X_WRITE_REQ, CP210X_CMD_SET_BAUDRATE, 0, 0,
                          sizeof(baudrate), (uint8_t *)&baudrate),
                      "CP210X", );
  return ESP_LOADER_SUCCESS;
}

esp_err_t cp210_host_set_control_line_state(cdc_acm_dev_hdl_t cdc, bool dtr,
                                            bool rts) {
  const uint16_t wValue = dtr | ((uint16_t)rts << 1) | 0x0300;
  ESP_RETURN_ON_ERROR(cdc_acm_host_send_custom_request(cdc, CP210X_WRITE_REQ,
                                                       CP210X_CMD_SET_MHS,
                                                       wValue, 0, 0, NULL),
                      "CP210X", );
  return ESP_LOADER_SUCCESS;
}

void cp210_enter_bootloader(cdc_acm_dev_hdl_t cdc) {
  cp210_host_set_control_line_state(cdc, false, true);
  loader_port_delay_ms(SERIAL_FLASHER_BOOT_HOLD_TIME_MS);
  cp210_host_set_control_line_state(cdc, true, false);
  loader_port_delay_ms(SERIAL_FLASHER_RESET_HOLD_TIME_MS);
  cp210_host_set_control_line_state(cdc, true, true);
}

void cp210_reset(cdc_acm_dev_hdl_t cdc) {
  cp210_host_set_control_line_state(cdc, false, true);
  loader_port_delay_ms(SERIAL_FLASHER_RESET_HOLD_TIME_MS);
  cp210_host_set_control_line_state(cdc, true, true);
}