#include "project.h"

static const nrf_drv_spi_t st7586_spi = NRF_DRV_SPI_INSTANCE(ST7586_SPI_INSTANCE);  /**< SPI instance. */
static volatile bool st7586_spi_xfer_done = false;  /**< Flag used to indicate that SPI instance completed the transfer. */

/**
 * @brief SPI user event handler.
 * @param event
 */
void spi_event_handler(nrf_drv_spi_evt_t const * p_event,void * p_context){ //SPI 이벤트
    st7586_spi_xfer_done = true;
}

void st7586_write(const uint8_t category, const uint8_t data){ //category 0이면 명령, 1이면 데이터
    static int err_code;
    nrf_gpio_pin_write(RATIO_SPI0_LCD_A0, category);
    
    st7586_spi_xfer_done = false;
    err_code = nrf_drv_spi_transfer(&st7586_spi, &data, 1, &rx_data, 0);
    APP_ERROR_CHECK(err_code);
    while (!st7586_spi_xfer_done) {
        __WFE();
    }
    nrf_delay_us(10);
}

void st7586_pinout_setup(){ // spi setup
    static int err_code;
    nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
    spi_config.ss_pin   = RATIO_SPI0_LCD_CS; //Slave Select pin
    spi_config.miso_pin = NRF_DRV_SPI_PIN_NOT_USED; //Output
    spi_config.mosi_pin = RATIO_SPI0_LCD_MOSI; //Input
    spi_config.sck_pin  = RATIO_SPI0_LCD_SCK; //Clock pin
    spi_config.frequency = NRF_SPI_FREQ_1M;
    spi_config.mode = NRF_DRV_SPI_MODE_3;
    
    err_code = nrf_drv_spi_init(&st7586_spi, &spi_config, spi_event_handler, NULL);
    APP_ERROR_CHECK(err_code);
    
    nrf_gpio_pin_set(RATIO_SPI0_LCD_A0);
    nrf_gpio_cfg_output(RATIO_SPI0_LCD_A0);
    
    nrf_gpio_pin_clear(RATIO_SPI0_LCD_BSTB);
    nrf_gpio_cfg_output(RATIO_SPI0_LCD_BSTB);
    
}

/**@brief Function for initializing low frequency clock.
 */
void clock_initialization(){
    NRF_CLOCK->LFCLKSRC            = (CLOCK_LFCLKSRC_SRC_Xtal << CLOCK_LFCLKSRC_SRC_Pos); //외부
    NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_LFCLKSTART    = 1; //시작한다는 의미!
    
    while (NRF_CLOCK->EVENTS_LFCLKSTARTED == 0){// Do nothing.
    }
}
