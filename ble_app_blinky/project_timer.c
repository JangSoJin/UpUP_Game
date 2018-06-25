#include "project.h"

APP_TIMER_DEF(energy_timer);
APP_TIMER_DEF(map_timer);
APP_TIMER_DEF(check_timer);

void energy_scheduler(void * p_context){ // 에너지바 줄이는 인터럽트
    UNUSED_PARAMETER(p_context);
    draw_rectangle(energy_start,0x04,0x03,0x07,1); // 10초마다 에너지 깎임
    st7586_write(ST_COMMAND,0x29);
    energy_start += 0x06;
    if(energy_start >= 0x7F) game_close();
}

void map_scheduler(void * p_context){ //맵 아래로 내려오게하는 인터럽트
    UNUSED_PARAMETER(p_context);
    clear_stage(start_x,start_y);
    clear_circle(circle_x,circle_y,0x08);
    nrf_delay_ms(50);
    start_y += 0x18;
    circle_y += 0x18;
    draw_circle(circle_x,circle_y,0x08);
    stage(start_x,start_y);
}

void check_scheduler(void * p_context){
    static int i,j,temp_rec;
    static bool flag = false;
    UNUSED_PARAMETER(p_context);
    
    for(i = circle_x-0x08; i <= circle_x+0x08; i++){ //스테이지클리어마크를 위한 탐색부분
        temp_rec = i/3;
        for(j = circle_y-0x08; j <= circle_y+0x08; j++){
            if(STAGE_MARK_BOARD[temp_rec][j] == 2){
                flag = true;
                break;
            }
        }
        if(flag) break;
    }
    
    if(flag){ // 스테이지 클리어마크를 먹었을때
        app_timer_stop_all();
        Clear_DDRAM();
        draw_clear();
        st7586_write(ST_COMMAND,0x29);
        Clear_DDRAM();
        nrf_delay_ms(1000);
        st7586_write(ST_COMMAND,0x28);
    }
    
    else if(circle_x-0x07 < 0 ||
            circle_x+0x07 > 0x7F ||
            circle_y-0x07 < 0x07 ||
            circle_y+0x07 > 0x9F){ //화면밖으로 이동했을때
        game_close();
    }
    
    //아래 예외사항은 떠있는 경우를 뜻함
    else if((RECTANGLE_BOARD[(circle_x-1)/3][circle_y+0x08] == 0 ||
             RECTANGLE_BOARD[(circle_x-2)/3][circle_y+0x08] == 0 ||
             RECTANGLE_BOARD[circle_x/3][circle_y+0x08] == 0 )&&
            circle_y+0x08 < 0x9F){
        while(circle_y+0x08 <= 0x9F){ //내려오는 애니메이션
            clear_circle(circle_x,circle_y,0x08);
            nrf_delay_ms(10);
            circle_y += 0x18;
            draw_circle(circle_x,circle_y,0x08);
        }
        game_close();
    }
    
}

void timers_init(){ //타이머 만들고 초기화하는 부분
    ret_code_t err_code;
    
    err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);
    
    err_code = app_timer_create(&energy_timer,APP_TIMER_MODE_REPEATED,energy_scheduler);
    APP_ERROR_CHECK(err_code);
    
    err_code = app_timer_create(&map_timer,APP_TIMER_MODE_REPEATED,map_scheduler);
    APP_ERROR_CHECK(err_code);
    
    err_code = app_timer_create(&check_timer,APP_TIMER_MODE_REPEATED,check_scheduler);
    APP_ERROR_CHECK(err_code);
}

void timers_start(){ //타이머 시작부분
    ret_code_t err_code;
    
    err_code = app_timer_start(energy_timer,ENERGY_DELAY,NULL);
    APP_ERROR_CHECK(err_code);
    
    err_code = app_timer_start(map_timer,MAP_DELAY,NULL);
    APP_ERROR_CHECK(err_code);
    
    err_code = app_timer_start(check_timer,CHECK_DELAY,NULL);
    APP_ERROR_CHECK(err_code);
}

void game_close(){ //종료시키는 (게임오버)
    Clear_DDRAM();
    draw_gameover();
    st7586_write(ST_COMMAND,0x29);
    nrf_delay_ms(1000);
    Clear_DDRAM();
    app_timer_stop_all();
    st7586_write(ST_COMMAND,0x28);
    return;
}
