#include "project.h"

int circle_x = 0x3F; //원의 X 좌표
int circle_y = 0x98; //원의 Y 좌표

int energy_start = 0x33; // 에너지 게이트 (0x33이 다 찬거)
int start_x = 0x06; // 스테이지 처음 블록의 X 좌표
int start_y = 0x88; // 스테이지 처음 블록의 Y 좌표

int devil_x = 0; //이동시키는 devil의 X 좌표

int cur1_x = 0; //현재의 x,y좌표 ( devil scheduler에서 사용하려고)
int cur1_y = 0;

int stage_clear_mark_x = 0x06+0x0C;

void Clear_DDRAM(){ //화면전체 0x00
    st7586_write(ST_COMMAND,0x2A);
    st7586_write(ST_DATA,0x00);
    st7586_write(ST_DATA,0x00);
    st7586_write(ST_DATA,0x00);
    st7586_write(ST_DATA,0x7F);
    
    st7586_write(ST_COMMAND,0x2B);
    st7586_write(ST_DATA,0x00);
    st7586_write(ST_DATA,0x00);
    st7586_write(ST_DATA,0x00);
    st7586_write(ST_DATA,0x9F);
    
    st7586_write(ST_COMMAND,0x2C);
    static int i,j;
    for(j = 0; j <= 0x7F; j++){
        for(i = 0; i <= 0x9F; i++){
            st7586_write(ST_DATA,0x00);
        }
    }
    for(j = 0; j <= 42; j++){
        for(i = 0; i <= 159; i++){
            LCD_BOARD[j][i] = 0;
            RECTANGLE_BOARD[j][i] = 0;
            CIRCLE_BOARD[j][i] = 0;
            STAGE_MARK_BOARD[j][i] = 0;
        }
    }
}

void draw_pixel(int x, int y){ //점찍기 ( sub-column 고려 ) choice
    int x_mod = x%3;
    int x_val = x/3;
    char pixel_val = LCD_BOARD[x_val][y];
    
    switch(x_mod)
    {
        case 0:
            pixel_val |= 7 << 5;
            break;
        case 1:
            pixel_val |= 7 << 2;
            break;
        case 2:
            pixel_val |= 3;
    }
    
    st7586_write(ST_COMMAND,0x2A);
    st7586_write(ST_DATA,0x00);
    st7586_write(ST_DATA,x_val);
    st7586_write(ST_DATA,0x00);
    st7586_write(ST_DATA,x_val);
    
    st7586_write(ST_COMMAND,0x2B);
    st7586_write(ST_DATA,0x00);
    st7586_write(ST_DATA,y);
    st7586_write(ST_DATA,0x00);
    st7586_write(ST_DATA,y);
    
    st7586_write(ST_COMMAND,0x2C);
    st7586_write(ST_DATA,pixel_val);
    LCD_BOARD[x_val][y] = pixel_val;
}

void clear_pixel(int x, int y, int choice){ //점지우기 ( sub-column 고려 ) choice == 1 :원지우기, choice == 2 : 네모지우기
    int x_val = x/3;
    char pixel_val = LCD_BOARD[x_val][y];

    st7586_write(ST_COMMAND,0x2A);
    st7586_write(ST_DATA,0x00);
    st7586_write(ST_DATA,x_val);
    st7586_write(ST_DATA,0x00);
    st7586_write(ST_DATA,x_val);
    
    st7586_write(ST_COMMAND,0x2B);
    st7586_write(ST_DATA,0x00);
    st7586_write(ST_DATA,y);
    st7586_write(ST_DATA,0x00);
    st7586_write(ST_DATA,y);
    
    st7586_write(ST_COMMAND,0x2C);
    if(choice == 1){ //사각형이 안그려져있다면 그냥 써클지움
        if(RECTANGLE_BOARD[x_val][y] == 0 && STAGE_MARK_BOARD[x_val][y] == 0){
            st7586_write(ST_DATA,0x00);
            LCD_BOARD[x_val][y] = 0;
        }
        else st7586_write(ST_DATA,pixel_val);
        CIRCLE_BOARD[x_val][y] = 0;
    }
    else if(choice == 2){ // 원이 그려져있다면 그냥 네모지움
        if(CIRCLE_BOARD[x_val][y] == 0){
            st7586_write(ST_DATA,0x00);
            LCD_BOARD[x_val][y] = 0;
        }
        else st7586_write(ST_DATA,pixel_val);
        RECTANGLE_BOARD[x_val][y] = 0;
        STAGE_MARK_BOARD[x_val][y] = 0;
    }
    else {
        st7586_write(ST_DATA,0x00);
        LCD_BOARD[x_val][y] = 0;
    }
}

void draw_rectangle(int x,int y, int w, int h, int choice){ //채워진 네모 그리기 ( 시작x, 시작y, width, height ) choice == 1 : 지우기
    static int i,j,temp_rec;
    
    if(choice == 1){
        for(i = x; i <= (x+w); i++){
            for(j = y; j <= (y+h); j++){
                clear_pixel(i,j,2);
            }
        }
    }
    else if(choice == 0){
        for(i = x; i <= (x+w); i++){
            for(j = y; j <= (y+h); j++){
                draw_pixel(i,j);
                temp_rec = i/3;
                RECTANGLE_BOARD[temp_rec][j] = 1;
            }
        }
    }
    else{ //choice == 2 : 스테이지 클리어 마크, choice == 3 : HP 게이트
        for(i = x; i <= (x+w); i++){
            for(j = y; j <= (y+h); j++){
                draw_pixel(i,j);
                temp_rec = i/3;
                STAGE_MARK_BOARD[temp_rec][j] = choice;
            }
        }
    }
}

void draw_notfill_rectangle(int x, int y, int w, int h){ // 채워지지않은 네모 그리기 ( 시작x, 시작y, width, height ) - 여기서는 에너지 막대틀에 쓰임
    static int i,temp_rec,temp_rec1;
    for(i = x; i <= (x+w); i++){
        temp_rec = i/3;
        draw_pixel(i,y);
        draw_pixel(i,y+h);
        RECTANGLE_BOARD[temp_rec][y] = 1;
        RECTANGLE_BOARD[temp_rec][y+h] = 1;
    }
    for(i = y; i <= (y+h); i++){
        temp_rec = x/3;
        temp_rec1 = (x+w)/3;
        draw_pixel(x,i);
        draw_pixel(x+w,i);
        RECTANGLE_BOARD[temp_rec][i] = 1;
        RECTANGLE_BOARD[temp_rec1][i] = 1;
    }
}


 void clear_circle(int x, int y, int r){ //원 비우기
     int temp_x = r-1;
     int temp_y = 0;
     int dx = 1;
     int dy = 1;
     int err = dx - (r <<1);
     
     while(temp_x >= temp_y){
         clear_pixel(x+temp_x,y+temp_y,1);
         clear_pixel(x+temp_y,y+temp_x,1);
         clear_pixel(x-temp_y,y+temp_x,1);
         clear_pixel(x-temp_x,y+temp_y,1);
         clear_pixel(x-temp_x,y-temp_y,1);
         clear_pixel(x-temp_y,y-temp_x,1);
         clear_pixel(x+temp_y,y-temp_x,1);
         clear_pixel(x+temp_x,y-temp_y,1);

         if(err <= 0){
             temp_y++;
             err += dy;
             dy += 2;
         }
         
         if(err >0){
             temp_x--;
             dx += 2;
             err += dx - (r <<1);
         }
     }
 }
 
 
 void draw_circle(int x,int y,int r){ //원그리기
     int temp_x = r-1;
     int temp_y = 0;
     int dx = 1;
     int dy = 1;
     int err = dx - (r <<1);
     
     while(temp_x >= temp_y){
         draw_pixel(x+temp_x,y+temp_y);
         draw_pixel(x+temp_y,y+temp_x);
         draw_pixel(x-temp_y,y+temp_x);
         draw_pixel(x-temp_x,y+temp_y);
         draw_pixel(x-temp_x,y-temp_y);
         draw_pixel(x-temp_y,y-temp_x);
         draw_pixel(x+temp_y,y-temp_x);
         draw_pixel(x+temp_x,y-temp_y);

         CIRCLE_BOARD[(x+temp_x)/3][y+temp_y] = 1;
         CIRCLE_BOARD[(x+temp_y)/3][y+temp_x] = 1;
         CIRCLE_BOARD[(x-temp_y)/3][y+temp_x] = 1;
         CIRCLE_BOARD[(x-temp_x)/3][y+temp_y] = 1;
         CIRCLE_BOARD[(x-temp_x)/3][y-temp_y] = 1;
         CIRCLE_BOARD[(x-temp_y)/3][y-temp_x] = 1;
         CIRCLE_BOARD[(x+temp_y)/3][y-temp_x] = 1;
         CIRCLE_BOARD[(x+temp_x)/3][y-temp_y] = 1;

         if(err <= 0){
             temp_y++;
             err += dy;
             dy += 2;
         }
         
         if(err >0){
             temp_x--;
             dx += 2;
             err += dx - (r <<1);
         }
     }
 }

void draw_triangle(int x, int y,int choice){ // choice == 0 : 그리기, choice == 1 : 지우기
    static int i,j,temp_rec;
    static int wi,he; // 가로길이
    wi = 0;
    he = y + 0x06; //높이
    if(choice == 0){
        for(j = y; j <= he; j++){
            for(i = x-wi; i <= x+wi; i++){
                draw_pixel(i,j);
                temp_rec = i/3;
                STAGE_MARK_BOARD[temp_rec][j] = 4;
            }
            wi++;
        }
    }
    else if(choice == 1){
        for(j = y; j <= he; j++){
            for(i = x-wi; i <= x+wi; i++){
                clear_pixel(i,j,2);
            }
            wi++;
        }
    }
}
 
 void ani_cross(int x, int y,int r){ //대각선으로 애니메이션
     draw_circle(x,y,r);
     st7586_write(ST_COMMAND,0x29);
     nrf_delay_ms(60);
     clear_circle(x,y,r);
     
     while(true){
         x -= 0x01;
         y -= 0x03;
         if(x <= 0 || y <= 0) break;
         draw_circle(x,y,r);
         st7586_write(ST_COMMAND,0x29);
         nrf_delay_ms(60);
         clear_circle(x,y,r);
     }
 }

void draw_start(){ // START 화면에 문구 출력
    /* S */
    draw_rectangle(0x06,0x30,0x12,0x04,0);
    draw_rectangle(0x06,0x30,0x04,0x12,0);
    draw_rectangle(0x06,0x42,0x12,0x04,0);
    draw_rectangle(0x14,0x42,0x04,0x12,0);
    draw_rectangle(0x06,0x54,0x12,0x04,0);
    
    /* T */
    draw_rectangle(0x20,0x30,0x12,0x04,0);
    draw_rectangle(0x26,0x30,0x04,0x28,0);
    
    /* A */
    draw_rectangle(0x39,0x30,0x12,0x04,0);
    draw_rectangle(0x39,0x30,0x04,0x28,0);
    draw_rectangle(0x48,0x30,0x04,0x28,0);
    draw_rectangle(0x39,0x42,0x12,0x04,0);
    
    /* R */
    draw_rectangle(0x53,0x30,0x12,0x04,0);
    draw_rectangle(0x53,0x30,0x04,0x28,0);
    draw_rectangle(0x61,0x30,0x04,0x12,0);
    draw_rectangle(0x53,0x42,0x12,0x04,0);
    
    static int wid,pix,hei;
    wid = 0x54;
    for(hei = 0x42; hei <= 0x58; hei++){
        for(pix = 0; pix <= 0x04; pix++){
            draw_pixel(wid+pix,hei);
        }
        wid++;
    }
    
    /* T */
    draw_rectangle(0x6D,0x30,0x12,0x04,0);
    draw_rectangle(0x73,0x30,0x04,0x28,0);
}

void draw_clear(){ // CLEAR 화면에 문구 출력
    /* C */
    draw_rectangle(0x06,0x30,0x12,0x04,0);
    draw_rectangle(0x06,0x30,0x04,0x28,0);
    draw_rectangle(0x06,0x54,0x12,0x04,0);
    
    /* L */
    draw_rectangle(0x20,0x30,0x04,0x28,0);
    draw_rectangle(0x20,0x54,0x12,0x04,0);
    
    /* E */
    draw_rectangle(0x39,0x30,0x12,0x04,0);
    draw_rectangle(0x39,0x30,0x04,0x28,0);
    draw_rectangle(0x39,0x44,0x12,0x04,0);
    draw_rectangle(0x39,0x54,0x12,0x04,0);
    
    /* A */
    draw_rectangle(0x53,0x30,0x12,0x04,0);
    draw_rectangle(0x53,0x30,0x04,0x28,0);
    draw_rectangle(0x62,0x30,0x04,0x28,0);
    draw_rectangle(0x53,0x42,0x12,0x04,0);
    
    /* R */
    draw_rectangle(0x6D,0x30,0x12,0x04,0);
    draw_rectangle(0x6D,0x30,0x04,0x28,0);
    draw_rectangle(0x7C,0x30,0x04,0x12,0);
    draw_rectangle(0x6D,0x42,0x12,0x04,0);
    
    static int wid,pix,hei;
    wid = 0x6E;
    for(hei = 0x46; hei <= 0x58; hei++){
        for(pix = 0; pix <= 0x04; pix++){
            draw_pixel(wid+pix,hei);
        }
        wid++;
    }
}

void draw_gameover(){ // GAME OVER 화면에 문구 출력
    
    /* G */
    draw_rectangle(0x09,0x20,0x18,0x04,0);
    draw_rectangle(0x09,0x20,0x04,0x28,0);
    draw_rectangle(0x09,0x44,0x18,0x04,0);
    draw_rectangle(0x20,0x34,0x04,0x14,0);
    draw_rectangle(0x13,0x34,0x10,0x04,0);

    /* A */
    draw_rectangle(0x28,0x20,0x15,0x04,0);
    draw_rectangle(0x28,0x20,0x04,0x28,0);
    draw_rectangle(0x39,0x20,0x04,0x28,0);
    draw_rectangle(0x28,0x32,0x15,0x04,0);

    /* M */
    draw_rectangle(0x42,0x20,0x09,0x04,0);
    draw_rectangle(0x42,0x20,0x04,0x28,0);
    draw_rectangle(0x49,0x20,0x04,0x12,0);
    draw_rectangle(0x49,0x30,0x09,0x04,0);
    draw_rectangle(0x50,0x20,0x04,0x14,0);
    draw_rectangle(0x50,0x20,0x09,0x04,0);
    draw_rectangle(0x57,0x20,0x04,0x28,0);

    /* E */
    draw_rectangle(0x60,0x20,0x04,0x28,0);
    draw_rectangle(0x60,0x20,0x15,0x04,0);
    draw_rectangle(0x60,0x32,0x15,0x04,0);
    draw_rectangle(0x60,0x44,0x15,0x04,0);

    /* O */
    draw_rectangle(0x09,0x60,0x16,0x04,0);
    draw_rectangle(0x09,0x60,0x04,0x28,0);
    draw_rectangle(0x09,0x84,0x16,0x04,0);
    draw_rectangle(0x20,0x60,0x04,0x28,0);

    /* V */
    static int wid = 0x28, hei = 0,pix;
    
    for(hei = 0x60; hei <= 0x84; hei++){
        for(pix = 0; pix <= 0x04; pix++){
            draw_pixel(wid+pix,hei);
        }
        if(hei % 6 == 0) wid++;
    } //왼쪽 대각선
    
    wid = 0x39;
    for(hei = 0x60; hei <= 0x84; hei++){
        for(pix = 0; pix <= 0x04; pix++){
            draw_pixel(wid+pix,hei);
        }
        if(hei % 6 == 0) wid--;
    } //오른쪽 대각선
    
    draw_rectangle(0x30,0x84,0x06,0x04,0);

    
    /* E */
    draw_rectangle(0x42,0x60,0x04,0x28,0);
    draw_rectangle(0x42,0x60,0x15,0x04,0);
    draw_rectangle(0x42,0x72,0x15,0x04,0);
    draw_rectangle(0x42,0x84,0x15,0x04,0);
    
    /* R */
    draw_rectangle(0x60,0x60,0x12,0x04,0);
    draw_rectangle(0x60,0x60,0x04,0x28,0);
    draw_rectangle(0x6F,0x60,0x04,0x15,0);
    draw_rectangle(0x60,0x72,0x12,0x04,0);
    wid = 0x61;
    for(hei = 0x76; hei <= 0x88; hei++){
        for(pix = 0; pix <= 0x04; pix++){
            draw_pixel(wid+pix,hei);
        }
        wid++;
    }
}

void Initiallization_ST7586S(){ //LCD 초기화
    nrf_gpio_pin_clear(RATIO_SPI0_LCD_BSTB);
    nrf_delay_ms(100);
    nrf_gpio_pin_set(RATIO_SPI0_LCD_BSTB);
    
    nrf_delay_ms(120);
    st7586_write(ST_COMMAND,0xD7);
    st7586_write(ST_DATA,0x9F);
    st7586_write(ST_COMMAND,0xE0);
    st7586_write(ST_DATA,0x00);
    nrf_delay_ms(10);
    st7586_write(ST_COMMAND,0xE3);
    nrf_delay_ms(20);
    st7586_write(ST_COMMAND,0xE1);
    st7586_write(ST_COMMAND,0x11);
    st7586_write(ST_COMMAND,0x28);
    nrf_delay_ms(50);
    st7586_write(ST_COMMAND,0xC0);
    st7586_write(ST_DATA,0x53);
    st7586_write(ST_DATA,0x01);
    st7586_write(ST_COMMAND,0xC3);
    st7586_write(ST_DATA,0x02);
    st7586_write(ST_COMMAND,0xC4);
    st7586_write(ST_DATA,0x06);
    st7586_write(ST_COMMAND,0xD0);
    st7586_write(ST_DATA,0x1D);
    st7586_write(ST_COMMAND,0xB5);
    st7586_write(ST_DATA,0x00);
    st7586_write(ST_COMMAND,0x39);
    st7586_write(ST_COMMAND,0x3A);
    st7586_write(ST_DATA,0x02);
    st7586_write(ST_COMMAND,0x36);
    st7586_write(ST_DATA,0x00);
    st7586_write(ST_COMMAND,0xB0);
    st7586_write(ST_DATA,0x9F);
    st7586_write(ST_COMMAND,0xB4);
    st7586_write(ST_DATA,0xA0);
    st7586_write(ST_COMMAND,0x30);
    st7586_write(ST_DATA,0x00);
    st7586_write(ST_DATA,0x00);
    st7586_write(ST_DATA,0x00);
    st7586_write(ST_DATA,0x77);
    st7586_write(ST_COMMAND,0x20);
    
    Clear_DDRAM();
    
    st7586_write(ST_COMMAND,0x29);
}

void drawing_init(){
    Initiallization_ST7586S(); //LCD부팅
    
    draw_start(); // 초기 START 화면
    st7586_write(ST_COMMAND,0x29);
    nrf_delay_ms(1000);
    Clear_DDRAM();
    
    draw_notfill_rectangle(0x32,0x03,0x4B,0x09); //초기 에너지막대
    
    /* start */
    draw_rectangle(energy_start,0x04,(0x7F-energy_start),0x08,0); //초기 에너지막대 그림
    draw_circle(circle_x,circle_y,0x08);
    
    stage(start_x,start_y);

}
