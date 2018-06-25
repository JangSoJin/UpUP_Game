#pragma once
#include "nrf_drv_spi.h"
#include "app_util_platform.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "boards.h"
#include "app_error.h"
#include <string.h>
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_drv_spi.h"
#include <stdint.h>
#include "nrf.h"

//버튼
#include "nrf_drv_gpiote.h"
#include "app_button.h"

//스케줄러 + 타이머 + 버튼
#include "app_scheduler.h"
#include "app_timer.h"
//#include "bsp.h"
#include "nordic_common.h"

/* ble */
#include "ble.h"
#include "ble_err.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_conn_params.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "ble_lbs.h"
#include "nrf_ble_gatt.h"

#define LEFT                    BSP_BUTTON_0
#define RIGHT                   BSP_BUTTON_1
#define UP                      BSP_BUTTON_2
#define DOWN                    BSP_BUTTON_3

#define LEDBUTTON_LED                   BSP_BOARD_LED_2                  

#define DEVICE_NAME                     "UPUPGame"

#define APP_FEATURE_NOT_SUPPORTED       BLE_GATT_STATUS_ATTERR_APP_BEGIN + 2    // Reply when unsupported features are requested.
#define APP_BLE_OBSERVER_PRIO           3
#define APP_BLE_CONN_CFG_TAG            1
#define APP_ADV_INTERVAL                64    // The advertising interval (in units of 0.625 ms; this value corresponds to 40 ms)
#define APP_ADV_TIMEOUT_IN_SECONDS      BLE_GAP_ADV_TIMEOUT_GENERAL_UNLIMITED    // The advertising time-out (in units of seconds). When set to 0, we will never time out.
#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(100, UNIT_1_25_MS)    // Minimum acceptable connection interval (0.5 seconds).
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(200, UNIT_1_25_MS)    // Maximum acceptable connection interval (1 second).
#define SLAVE_LATENCY                   0
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)    // Connection supervisory time-out (4 seconds).
#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(20000)    // Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (15 seconds).
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(5000)    // Time between each call to sd_ble_gap_conn_param_update after the first call (5 seconds).
#define MAX_CONN_PARAMS_UPDATE_COUNT    3    // Number of attempts before giving up the connection parameter negotiation.
#define BUTTON_DETECTION_DELAY          APP_TIMER_TICKS(50)
#define DEAD_BEEF                       0xDEADBEEF    // Value used as error code on stack dump, can be used to identify stack location on stack unwind.

#define ST7586_SPI_INSTANCE  0/**< SPI instance index. */

#define ST_COMMAND    0
#define ST_DATA        1

#define RATIO_SPI0_LCD_SCK          4
#define RATIO_SPI0_LCD_A0            28
#define RATIO_SPI0_LCD_MOSI            29
#define RATIO_SPI0_LCD_BSTB            30
#define RATIO_SPI0_LCD_CS            31

#define LCD_INIT_DELAY(t) nrf_delay_ms(t)

//스케줄러를 위한
#define SCHED_MAX_EVENT_DATA_SIZE       sizeof(app_timer_event_t)
#define SCHED_QUEUE_SIZE 200

//타이머를 위한
#define ENERGY_DELAY APP_TIMER_TICKS(10000)
#define MAP_DELAY APP_TIMER_TICKS(5000)
#define DEVIL_DELAY APP_TIMER_TICKS(3000)
#define CHECK_DELAY APP_TIMER_TICKS(1000)


unsigned char rx_data;

unsigned char LCD_BOARD[43][160];
unsigned char CIRCLE_BOARD[43][160];
unsigned char RECTANGLE_BOARD[43][160];
unsigned char STAGE_MARK_BOARD[43][160];

extern int circle_x;
extern int circle_y;

extern int energy_start;
extern int start_x;
extern int start_y;

extern int stage_clear_mark_x;

void Clear_DDRAM(); //화면 지우기

void draw_pixel(int x, int y); //점찍기

void clear_pixel(int x, int y, int choice); //점지우기

void draw_rectangle(int x,int y, int w, int h, int choice); //사각형그리기

void draw_notfill_rectangle(int x, int y, int w, int h); //안채워진 사각형

void clear_circle(int x, int y, int r); //원지우기

void draw_circle(int x,int y,int r); //원그리기

void draw_triangle(int x,int y,int choice); //삼각형그리기

void ani_cross(int x, int y,int r); //애니메이션(테스트용)

void draw_start(); //스타트화면 그리기(문구)

void draw_clear(); //클리어화면 그리기(문구)

void draw_gameover(); //게임오버화면 그리기(문구)

void Initiallization_ST7586S(); //LCD 부팅 초기화

void spi_event_handler(nrf_drv_spi_evt_t const * p_event,void * p_context); //SPI관련 핸들러

void st7586_write(const uint8_t category, const uint8_t data); //command에 따라 명령

void st7586_pinout_setup(); // 핀 설정

void clock_initialization(); //초기화

void up(); //위

void down(); //아래

void left(); //왼쪽

void right(); //오른쪽

void button_handler(nrf_drv_gpiote_pin_t pin); //버튼 핸들러

void button_scheduler_event_handler(void *p_event_data, uint16_t event_size); //스케줄러에 담는

void gpiote_event_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action); //스케줄러에 담는

void gpio_config(); //버튼 설정

void drawing_init(); //초기 화면 (스타트문구 화면 + 에너지게이트 초기화)

void energy_scheduler(void * p_context); // 에너지바 줄이는 스케줄러

void map_scheduler(void * p_context); //맵 아래로 내려오게하는 스케줄러

void devil_scheduler(void * p_context); // 악당 움직이는 스케줄러

void check_scheduler(void * p_context); //예외사항 체크하는 스케줄러

void timers_init(); //타이머 만들고 초기화하는 부분

void timers_start(); //타이머 시작부분

void stage(int s_x, int s_y); // 스테이지 맵 그리기

void clear_stage(int s_x, int s_y); //스테이지 맵 지우기

void game_close(); //종료시키는 (게임오버)

/* ble 부분 */

void conn_params_init(void);

void advertising_init(void);

void services_init(void);

void gatt_init(void);

void gap_params_init(void);

void ble_stack_init(void);

void advertising_start(void);

void buttons_init(void);
