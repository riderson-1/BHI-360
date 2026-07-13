#include <zephyr/devicetree.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>

#define IMU_MAIN_NODE   DT_NODELABEL(imu_main)
#define IMU_RESET_NODE  DT_NODELABEL(reset_imu_main)

static imu_device_t imu_devices[] = {
    {
        .spi_spec = SPI_DT_SPEC_GET(IMU_MAIN_NODE, SPI_WORD_SET(8) | SPI_TRANSFER_MSB),
        .int_pin = GPIO_DT_SPEC_GET(IMU_MAIN_NODE, int_gpios),
        .reset_pin = GPIO_DT_SPEC_GET(IMU_RESET_NODE, gpios),
        .initialized = false,
        .name = "IMU_MAIN"
    }
};

#define NUM_IMUS (sizeof(imu_devices) / sizeof(imu_devices[0]))
