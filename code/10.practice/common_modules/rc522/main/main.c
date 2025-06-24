#include <esp_log.h>
#include "rc522.h"
#include "driver/rc522_spi.h"
#include "rc522_picc.h"

static const char *TAG = "rc522-basic-example";

#define RC522_SPI_BUS_GPIO_MISO    (25)
#define RC522_SPI_BUS_GPIO_MOSI    (23)
#define RC522_SPI_BUS_GPIO_SCLK    (19)
#define RC522_SPI_SCANNER_GPIO_SDA (22)
#define RC522_SCANNER_GPIO_RST     (-1) // soft-reset

static rc522_spi_config_t driver_config = {
    .host_id = SPI3_HOST,
    .bus_config = &(spi_bus_config_t){
        .miso_io_num = RC522_SPI_BUS_GPIO_MISO,
        .mosi_io_num = RC522_SPI_BUS_GPIO_MOSI,
        .sclk_io_num = RC522_SPI_BUS_GPIO_SCLK,
    },
    .dev_config = {
        .spics_io_num = RC522_SPI_SCANNER_GPIO_SDA,
    },
    .rst_io_num = RC522_SCANNER_GPIO_RST,
};

static rc522_driver_handle_t driver;
static rc522_handle_t scanner;

static void on_picc_state_changed(void *arg, esp_event_base_t base, int32_t event_id, void *data)
{
    rc522_picc_state_changed_event_t *event = (rc522_picc_state_changed_event_t *)data;
    rc522_picc_t *picc = event->picc;

    if (picc->state == RC522_PICC_STATE_ACTIVE) {
        rc522_picc_print(picc);
    }
    else if (picc->state == RC522_PICC_STATE_IDLE && event->old_state >= RC522_PICC_STATE_ACTIVE) {
        ESP_LOGI(TAG, "Card has been removed");
    }
}

void app_main()
{
    rc522_spi_create(&driver_config, &driver);
    rc522_driver_install(driver);

    rc522_config_t scanner_config = {
        .driver = driver,
    };

    rc522_create(&scanner_config, &scanner);
    rc522_register_events(scanner, RC522_EVENT_PICC_STATE_CHANGED, on_picc_state_changed, NULL);
    rc522_start(scanner);
}
