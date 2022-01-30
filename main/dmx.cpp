#include "dmx.h"

// system includes
#include <string_view>

// esp-idf includes
#include <esp_log.h>
#include <driver/uart.h>
#include <driver/gpio.h>
#include <rom/gpio.h>

// 3rdparty lib includes
#include <espstrutils.h>

namespace {
constexpr const char * const TAG = "DMX";

constexpr auto DMX_UART_NUM = UART_NUM_2;

constexpr auto DMX_SERIAL_INPUT_PIN =  GPIO_NUM_39; // pin for dmx rx
constexpr auto DMX_SERIAL_OUTPUT_PIN = GPIO_NUM_17; // pin for dmx tx
constexpr auto DMX_SERIAL_IO_PIN =     GPIO_NUM_16;  // pin for dmx rx/tx change
} // namespace

void dmx_init()
{
    ESP_LOGI(TAG, "hello");

    uart_config_t uart_config {
        .baud_rate = 250000,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_2,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    if (const auto result = uart_param_config(DMX_UART_NUM, &uart_config); result != ESP_OK)
    {
        ESP_LOGE(TAG, "uart_param_config() failed with %s", esp_err_to_name(result));
        return;
    }

    if (const auto result = uart_set_pin(DMX_UART_NUM, DMX_SERIAL_OUTPUT_PIN, DMX_SERIAL_INPUT_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE); result != ESP_OK)
    {
        ESP_LOGE(TAG, "uart_set_pin() failed with %s", esp_err_to_name(result));
        return;
    }

    if (const auto result = uart_driver_install(DMX_UART_NUM, SOC_UART_FIFO_LEN + 1, 0, 10, nullptr, 0); result != ESP_OK)
    {
        ESP_LOGE(TAG, "uart_driver_install() failed with %s", esp_err_to_name(result));
        return;
    }

    if (false)
    if (const auto result = uart_set_mode(DMX_UART_NUM, UART_MODE_RS485_HALF_DUPLEX))
    {
        ESP_LOGE(TAG, "uart_set_mode() failed with %s", esp_err_to_name(result));
        return;
    }

    gpio_pad_select_gpio(DMX_SERIAL_IO_PIN);
    gpio_set_direction(DMX_SERIAL_IO_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(DMX_SERIAL_IO_PIN, 0); // input
}

void dmx_update()
{
    size_t length{};
    if (const auto result = uart_get_buffered_data_len(DMX_UART_NUM, &length); result != ESP_OK)
    {
        ESP_LOGW(TAG, "uart_get_buffered_data_len() failed with %s", esp_err_to_name(result));
    }
    else if (length)
    {
        char data[length];
        length = uart_read_bytes(DMX_UART_NUM, data, length, 0);

        const std::string_view received{data, length};

        ESP_LOGI(TAG, "received RS485 %s", espcpputils::toHexString(received).c_str());
    }
}
