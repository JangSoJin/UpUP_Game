#include "project.h"

BLE_LBS_DEF(m_lbs);        // LED Button Service instance.
NRF_BLE_GATT_DEF(m_gatt);    // GATT module instance.

static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID;    // Handle of the current connection.

void advertising_start(void) {
    ret_code_t           err_code;
    ble_gap_adv_params_t adv_params;
    
    // Start advertising
    memset(&adv_params, 0, sizeof(adv_params));
    
    adv_params.type        = BLE_GAP_ADV_TYPE_ADV_IND;
    adv_params.p_peer_addr = NULL;
    adv_params.fp          = BLE_GAP_ADV_FP_ANY;
    adv_params.interval    = APP_ADV_INTERVAL;
    adv_params.timeout     = APP_ADV_TIMEOUT_IN_SECONDS;
    
    err_code = sd_ble_gap_adv_start(&adv_params, APP_BLE_CONN_CFG_TAG);
    APP_ERROR_CHECK(err_code);
}

void ble_evt_handler(ble_evt_t const *p_ble_evt, void *p_context) {
    ret_code_t err_code;
    
    switch (p_ble_evt->header.evt_id) {
        case BLE_GAP_EVT_CONNECTED:
            NRF_LOG_INFO("Connected");
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            
            err_code = app_button_enable();
            APP_ERROR_CHECK(err_code);
            break;
            
        case BLE_GAP_EVT_DISCONNECTED:
            NRF_LOG_INFO("Disconnected");
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            err_code = app_button_disable();
            APP_ERROR_CHECK(err_code);
            advertising_start();
            break;
            
        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            // Pairing not supported
            err_code = sd_ble_gap_sec_params_reply(m_conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
            APP_ERROR_CHECK(err_code);
            break;
            
#ifndef S140
        case BLE_GAP_EVT_PHY_UPDATE_REQUEST: {
            NRF_LOG_DEBUG("PHY update request.");
            ble_gap_phys_t const phys = { .rx_phys = BLE_GAP_PHY_AUTO, .tx_phys = BLE_GAP_PHY_AUTO, };
            err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
            APP_ERROR_CHECK(err_code);
        }    break;
#endif
            
        case BLE_GATTS_EVT_SYS_ATTR_MISSING:
            // No system attributes have been stored.
            err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, 0);
            APP_ERROR_CHECK(err_code);
            break;
            
        case BLE_GATTC_EVT_TIMEOUT:
            // Disconnect on GATT Client timeout event.
            NRF_LOG_DEBUG("GATT Client Timeout.");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;
            
        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
            NRF_LOG_DEBUG("GATT Server Timeout.");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;
            
        case BLE_EVT_USER_MEM_REQUEST:
            err_code = sd_ble_user_mem_reply(p_ble_evt->evt.gattc_evt.conn_handle, NULL);
            APP_ERROR_CHECK(err_code);
            break;
            
        case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST: {
            ble_gatts_evt_rw_authorize_request_t  req;
            ble_gatts_rw_authorize_reply_params_t auth_reply;
            
            req = p_ble_evt->evt.gatts_evt.params.authorize_request;
            
            if (req.type != BLE_GATTS_AUTHORIZE_TYPE_INVALID) {
                if ((req.request.write.op == BLE_GATTS_OP_PREP_WRITE_REQ)
                    || (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_NOW)
                    || (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_CANCEL)) {
                    if (req.type == BLE_GATTS_AUTHORIZE_TYPE_WRITE) {
                        auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
                    } else {
                        auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_READ;
                    }
                    auth_reply.params.write.gatt_status = APP_FEATURE_NOT_SUPPORTED;
                    err_code = sd_ble_gatts_rw_authorize_reply(p_ble_evt->evt.gatts_evt.conn_handle, &auth_reply);
                    APP_ERROR_CHECK(err_code);
                }
            }
        } break;
            
        default: break;
    }
}

void ble_stack_init(void) {
    ret_code_t err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);
    
    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);
    
    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);
    
    // Register a handler for BLE events.
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}

void gap_params_init(void) {
    ret_code_t              err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;
    
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
    err_code = sd_ble_gap_device_name_set(&sec_mode, (const uint8_t *)DEVICE_NAME, strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);
    
    memset(&gap_conn_params, 0, sizeof(gap_conn_params));
    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;
    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}

void gatt_init(void) {
    ret_code_t err_code = nrf_ble_gatt_init(&m_gatt, NULL);
    APP_ERROR_CHECK(err_code);
}

void up(){
    clear_circle(circle_x,circle_y,0x08);
    nrf_delay_ms(10);
    circle_y -= 0x18;
    draw_circle(circle_x,circle_y,0x08);
    draw_rectangle(energy_start,0x04,0x03,0x07,1); // 올라갈때 에너지 깎임
    st7586_write(ST_COMMAND,0x29);
    energy_start += 0x06;
}

void down(){
    //nrf_drv_gpiote_out_toggle(BSP_LED_3);
    clear_circle(circle_x,circle_y,0x08);
    nrf_delay_ms(10);
    circle_y += 0x18;
    draw_circle(circle_x,circle_y,0x08);
}

void left(){
    clear_circle(circle_x,circle_y,0x08);
    nrf_delay_ms(10);
    circle_x -= 0x08;
    draw_circle(circle_x,circle_y,0x08);
}

void right(){
    clear_circle(circle_x,circle_y,0x08);
    nrf_delay_ms(10);
    circle_x += 0x08;
    draw_circle(circle_x,circle_y,0x08);
}

void button_handler(nrf_drv_gpiote_pin_t pin){ //버튼 핸들러
    switch (pin){
        case 0: //좌
            left();
            break;
            
        case 1: //우
            right();
            break;
            
        case 2: //위
            up();
            break;
            
        case 3: //아래
            down();
            break;
            
        default:
            break;
    }
}

void button_scheduler_event_handler(void *p_event_data, uint16_t event_size){ //버튼+스케줄러
    button_handler(*((nrf_drv_gpiote_pin_t*)p_event_data));
}

static void button_write_handler(uint16_t conn_handle, ble_lbs_t * p_lbs, uint8_t pin_number){
    app_sched_event_put(&pin_number, sizeof(pin_number), button_scheduler_event_handler);
}


void services_init(void) {
    ret_code_t     err_code;
    ble_lbs_init_t init;
    
    init.led_write_handler = button_write_handler;
    
    err_code = ble_lbs_init(&m_lbs, &init);
    APP_ERROR_CHECK(err_code);
}

void advertising_init(void) {
    ret_code_t    err_code;
    ble_advdata_t advdata;
    ble_advdata_t srdata;
    ble_uuid_t    adv_uuids[] = {{LBS_UUID_SERVICE, m_lbs.uuid_type}};
    
    // Build and set advertising data
    memset(&advdata, 0, sizeof(advdata));
    
    advdata.name_type          = BLE_ADVDATA_FULL_NAME;
    advdata.include_appearance = true;
    advdata.flags              = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
    
    memset(&srdata, 0, sizeof(srdata));
    srdata.uuids_complete.uuid_cnt = sizeof(adv_uuids) / sizeof(adv_uuids[0]);
    srdata.uuids_complete.p_uuids  = adv_uuids;
    
    err_code = ble_advdata_set(&advdata, &srdata);
    APP_ERROR_CHECK(err_code);
}

// conn_params
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt) {
    ret_code_t err_code;
    
    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED) {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}
static void conn_params_error_handler(uint32_t nrf_error) { APP_ERROR_HANDLER(nrf_error); }
void conn_params_init(void) {
    ret_code_t             err_code;
    ble_conn_params_init_t cp_init;
    
    memset(&cp_init, 0, sizeof(cp_init));
    
    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;
    
    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}
