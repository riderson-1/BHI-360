#include "common.h"
#include <zephyr/logging/log.h>
#include <string.h>

LOG_MODULE_DECLARE(main);

void setup_SPI(imu_device_t *imu)
{
    int ret;

    // Verify SPI device is available
    if (!device_is_ready(imu->spi_spec.bus)) {
        LOG_ERR("%s: SPI bus not ready", imu->name);
        return;
    }

    // Configure interrupt pin as input
    if (imu->int_pin.port != NULL) {
        ret = gpio_pin_configure_dt(&imu->int_pin, GPIO_INPUT);
        if (ret < 0) {
            LOG_ERR("%s: Failed to configure INT pin: %d", imu->name, ret);
        } else {
            LOG_INF("%s: INT pin configured at P%u.%02u", 
                    imu->name,
                    (imu->int_pin.pin >> 5) & 1,
                    imu->int_pin.pin & 0x1F);
        }
    }

    // Release reset (set to inactive - HIGH for GPIO_ACTIVE_LOW)
    if (imu->reset_pin.port != NULL) {
        ret = gpio_pin_configure_dt(&imu->reset_pin, GPIO_OUTPUT_INACTIVE);
        if (ret < 0) {
            LOG_ERR("%s: Failed to configure reset pin: %d", imu->name, ret);
        } else {
            LOG_INF("%s: Reset pin released (inactive)", imu->name);
        }
    }

    LOG_INF("%s: SPI bus configured (8MHz)", imu->name);
}

int8_t bhi360_spi_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t length, void *intf_ptr)
{
    imu_device_t *imu = (imu_device_t *)intf_ptr;
    int ret;

    uint8_t m_tx_buf[1] = {reg_addr | 0x80};
    uint8_t m_rx_buf[length + 1];

    // Perform SPI read (read address with MSB set, then read data)
    const struct spi_buf tx_buf = {.buf = m_tx_buf, .len = sizeof(m_tx_buf)};
    const struct spi_buf_set tx = {.buffers = &tx_buf, .count = 1};

    struct spi_buf rx_buf = {.buf = m_rx_buf, .len = sizeof(m_rx_buf)};
    const struct spi_buf_set rx = {.buffers = &rx_buf, .count = 1};

    ret = spi_transceive_dt(&imu->spi_spec, &tx, &rx);
    if (ret < 0) {
        LOG_ERR("%s: SPI read failed: %d", imu->name, ret);
        return -1;
    }

    // Copy data, skipping first byte (address echo)
    memcpy(reg_data, &m_rx_buf[1], length);

    return BHY2_INTF_RET_SUCCESS;
}

int8_t bhi360_spi_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t length, void *intf_ptr)
{
    imu_device_t *imu = (imu_device_t *)intf_ptr;
    int ret;

    uint8_t m_tx_buf[length + 1];
    m_tx_buf[0] = reg_addr;
    memcpy(m_tx_buf + 1, reg_data, length);

    const struct spi_buf tx_buf = {.buf = m_tx_buf, .len = sizeof(m_tx_buf)};
    const struct spi_buf_set tx = {.buffers = &tx_buf, .count = 1};

    ret = spi_transceive_dt(&imu->spi_spec, &tx, NULL);
    if (ret < 0) {
        LOG_ERR("%s: SPI write failed: %d", imu->name, ret);
        return -1;
    }

    return BHY2_INTF_RET_SUCCESS;
}

const char *get_api_error(int8_t error_code)
{
    switch (error_code)
    {
        case BHY2_OK: return "BHY2_OK";
        case BHY2_E_NULL_PTR: return "BHY2_E_NULL_PTR";
        case BHY2_E_INVALID_PARAM: return "BHY2_E_INVALID_PARAM";
        case BHY2_E_IO: return "BHY2_E_IO";
        case BHY2_E_MAGIC: return "BHY2_E_MAGIC";
        case BHY2_E_TIMEOUT: return "BHY2_E_TIMEOUT";
        case BHY2_E_BUFFER: return "BHY2_E_BUFFER";
        default: return "Unknown error code";
    }
}

const char *get_sensor_error_text(uint8_t sensor_error)
{
    switch (sensor_error)
    {
        case 0: return "No error";
        default: return "Unknown sensor error";
    }
}

const char *get_sensor_name(uint8_t sensor_id)
{
    switch (sensor_id)
    {
        case BHY2_SENSOR_ID_RV: return "Rotation Vector";
        default: return "Unknown sensor";
    }
}