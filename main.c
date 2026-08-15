#include <reg52.h>
#include <string.h>
#include <stdio.h>


// 引脚定义

sbit LCD_RS = P2^6;
sbit LCD_RW = P2^5;
sbit LCD_EN = P2^7;
#define LCD_DataPort P0

#define SEG_PORT P0
#define DIG_PORT P2

#define KEY_PORT P1
#define COL_PORT P0
#define ROW_PORT P1

sbit BEEP = P1^5; // 蜂鸣器引脚


// 段码表 & 笑脸

unsigned char code segCode[] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66,
    0x6D, 0x7D, 0x07, 0x7F, 0x6F,
    0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71,
    0x00, 0x40, 0x73, 0x38, 0x3E, 0x3F, 0x54
};

unsigned char code smile_pattern[8] = {
    0x00, 0x66, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00
};


// 延时

void delay_us(unsigned int t) { while(t--); }
void delay_ms(unsigned int t)
{
    unsigned int i, j;
    for(i = t; i > 0; i--)
        for(j = 110; j > 0; j--);
}


// 蜂鸣器控制 (高电平触发)

void BEEP_On()   { BEEP = 1; }
void BEEP_Off()  { BEEP = 0; }

void BEEP_Short()
{
    BEEP_On();
    delay_ms(50);
    BEEP_Off();
}

void BEEP_OK()
{
    BEEP_On(); delay_ms(80); BEEP_Off();
    delay_ms(80);
    BEEP_On(); delay_ms(80); BEEP_Off();
}

void BEEP_Error()
{
    BEEP_On();
    delay_ms(600);
    BEEP_Off();
}


// LCD驱动

void LCD_WriteCmd(unsigned char cmd)
{
    LCD_DataPort = cmd;
    LCD_RS = 0;
    LCD_RW = 0;
    LCD_EN = 1;
    delay_us(10);
    LCD_EN = 0;
    delay_ms(2);
}

void LCD_WriteData(unsigned char dat)
{
    LCD_DataPort = dat;
    LCD_RS = 1;
    LCD_RW = 0;
    LCD_EN = 1;
    delay_us(10);
    LCD_EN = 0;
    delay_ms(2);
}

void LCD_Init()
{
    delay_ms(15);
    LCD_WriteCmd(0x38);
    LCD_WriteCmd(0x0C);
    LCD_WriteCmd(0x06);
    LCD_WriteCmd(0x01);
    delay_ms(2);
}

void LCD_SetCursor(unsigned char row, unsigned char col)
{
    unsigned char addr;
    if(row == 0) addr = 0x00 + col;
    else addr = 0x40 + col;
    LCD_WriteCmd(0x80 | addr);
}

void LCD_ShowString(unsigned char row, unsigned char col, unsigned char *str)
{
    LCD_SetCursor(row, col);
    while(*str != '\0') {
        LCD_WriteData(*str);
        str++;
    }
}

void LCD_Clear()
{
    LCD_WriteCmd(0x01);
    delay_ms(2);
}

void LCD_ShowChar(unsigned char row, unsigned char col, unsigned char ch)
{
    LCD_SetCursor(row, col);
    LCD_WriteData(ch);
}


// 数码管

void Display_Digit(unsigned char pos, unsigned char num)
{
    unsigned char i;
    DIG_PORT = 0xFF;
    SEG_PORT = segCode[num];
    DIG_PORT = ~(0x01 << pos);
    for(i = 0; i < 5; i++);
    DIG_PORT = 0xFF;
}

void Display_Number(unsigned int num)
{
    unsigned char digits[8];
    unsigned char i;
    unsigned int temp;
    for(i = 0; i < 8; i++) digits[i] = 16;
    if(num == 0) { digits[7] = 0; }
    else {
        i = 7;
        temp = num;
        while(temp > 0 && i > 0) {
            digits[i] = temp % 10;
            temp /= 10;
            i--;
        }
    }
    for(i = 0; i < 8; i++) {
        Display_Digit(i, digits[i]);
    }
}

void Display_String(unsigned char *str)
{
    unsigned char i;
    unsigned char map[8];
    for(i = 0; i < 8; i++) {
        if(str[i] >= '0' && str[i] <= '9') map[i] = str[i] - '0';
        else if(str[i] >= 'A' && str[i] <= 'F') map[i] = str[i] - 'A' + 10;
        else if(str[i] == 'P') map[i] = 18;
        else if(str[i] == 'L') map[i] = 19;
        else if(str[i] == 'U') map[i] = 20;
        else if(str[i] == 'O') map[i] = 21;
        else if(str[i] == 'n') map[i] = 22;
        else map[i] = 16;
    }
    for(i = 0; i < 8; i++) {
        Display_Digit(i, map[i]);
    }
}


// 点阵

void Show_Smile()
{
    unsigned char i, j;
    for(j = 0; j < 50; j++) {
        for(i = 0; i < 8; i++) {
            ROW_PORT = ~(0x01 << i);
            COL_PORT = smile_pattern[i];
            delay_us(50);
            ROW_PORT = 0xFF;
        }
    }
    ROW_PORT = 0xFF;
    COL_PORT = 0x00;
}


// 矩阵键盘扫描（恢复你确认过正确的版本）

unsigned char code keyMap[4][4] = {
    {  0 ,  'E',  'M',  'B'}, // Row 0
    { '#',  '9',  '6',  '3'}, // Row 1
    { '0',  '8',  '5',  '2'}, // Row 2
    { '*',  '7',  '4',  '1'}  // Row 3
};

unsigned char Key_Scan()
{
    unsigned char row, col, temp;
    
    for(row = 0; row < 4; row++) {
        KEY_PORT = ~(0x01 << row);
        temp = KEY_PORT & 0xF0;
        if(temp != 0xF0) {
            delay_ms(20);
            temp = KEY_PORT & 0xF0;
            if(temp != 0xF0) {
                if((temp & 0x10) == 0) col = 0;
                else if((temp & 0x20) == 0) col = 1;
                else if((temp & 0x40) == 0) col = 2;
                else if((temp & 0x80) == 0) col = 3;
                else continue;
                
                while((KEY_PORT & 0xF0) != 0xF0);
                return keyMap[row][col];
            }
        }
    }
    return 0;
}


// 定时器

unsigned int timer_count = 0;
bit timer_1s_flag = 0;

void Timer0_Init()
{
    TMOD &= 0xF0;
    TMOD |= 0x01;
    TH0 = 0x4C;
    TL0 = 0x00;
    ET0 = 1;
    EA = 1;
    TR0 = 1;
}

void Timer0_ISR() interrupt 1
{
    TH0 = 0x4C;
    TL0 = 0x00;
    timer_count++;
    if(timer_count >= 1000) {
        timer_count = 0;
        timer_1s_flag = 1;
    }
}


// 主程序

unsigned char password[6] = {1,2,3,4,5,6};
unsigned char input[6];
unsigned char step = 0;
unsigned char error_count = 0;
bit is_locked = 0;
unsigned int lock_remain = 0;

void main()
{
    unsigned char i;
    unsigned char key;
    unsigned char new_pwd[6];
    unsigned char admin_step;
    unsigned int admin_timeout;
    unsigned char str[8];
    bit correct;
    
    BEEP_Off();
    LCD_Init();
    Timer0_Init();
    
    LCD_ShowString(0, 0, "Enter Password:");
    LCD_ShowString(1, 0, "                ");
    
    while(1)
    {
        //  锁定状态 
        if(is_locked) {
            if(timer_1s_flag) {
                timer_1s_flag = 0;
                if(lock_remain > 0) {
                    lock_remain--;
                    LCD_ShowString(0, 0, "  Locked!    ");
                    sprintf(str, "  %d s  ", lock_remain);
                    LCD_ShowString(1, 0, str);
                    Display_Number(lock_remain);
                } else {
                    is_locked = 0;
                    error_count = 0;
                    LCD_Clear();
                    LCD_ShowString(0, 0, "Enter Password:");
                    LCD_ShowString(1, 0, "                ");
                    Display_String("  OPEN   ");
                    BEEP_OK();
                    delay_ms(1000);
                    Display_Number(0);
                }
            }
            continue;
        }
        
        Display_Number(step);
        key = Key_Scan();
        if(key == 0) continue;
        
        // 管理员模式（S8 键）
        if(key == 'M') {
            BEEP_Short();
            LCD_Clear();
            LCD_ShowString(0, 0, "Admin Mode");
            LCD_ShowString(1, 0, "New Pwd:");
            
            admin_step = 0;
            admin_timeout = 0;
            
            while(admin_timeout < 1500) {
                key = Key_Scan();
                if(key != 0) {
                    admin_timeout = 0;
                    if(key >= '0' && key <= '9' && admin_step < 6) {
                        new_pwd[admin_step] = key - '0';
                        admin_step++;
                        LCD_ShowChar(1, 8 + admin_step - 1, '*');
                        BEEP_Short();
                    }
                    else if(key == 'E') {
                        if(admin_step == 6) {
                            for(i = 0; i < 6; i++) password[i] = new_pwd[i];
                            LCD_Clear();
                            LCD_ShowString(0, 0, "Success!"); // 绝对不乱码的提示
                            LCD_ShowString(1, 0, "Pwd Updated");
                            BEEP_OK();
                            Show_Smile();
                            delay_ms(1500);
                            break;
                        } else {
                            admin_step = 0;
                            LCD_ShowString(1, 8, "        ");
                            BEEP_Error();
                        }
                    }
                    else if(key == 'B') {
                        if(admin_step > 0) {
                            admin_step--;
                            LCD_ShowChar(1, 8 + admin_step, ' ');
                            BEEP_Short();
                        }
                    }
                    else if(key == 'M') {
                        BEEP_Short();
                        break;
                    }
                }
                delay_ms(10);
                admin_timeout++;
            }
            
            LCD_Clear();
            LCD_ShowString(0, 0, "Enter Password:");
            LCD_ShowString(1, 0, "                ");
            step = 0;
            continue; 
        }
        
        //  正常输入模式 
        if(key >= '0' && key <= '9' && step < 6) {
            input[step] = key - '0';
            step++;
            LCD_ShowChar(1, step-1, key);
            BEEP_Short();
            delay_ms(200);
            LCD_ShowChar(1, step-1, '*');
        }
        else if(key == 'E') {
            if(step == 6) {
                correct = 1;
                for(i = 0; i < 6; i++) {
                    if(input[i] != password[i]) {
                        correct = 0;
                        break;
                    }
                }
                
                if(correct) {
                    LCD_Clear();
                    LCD_ShowString(0, 0, "  Unlock OK!");
                    LCD_ShowString(1, 0, "  Welcome!");
                    Display_String("  WELCOME");
                    BEEP_OK();
                    Show_Smile();
                    delay_ms(1500);
                    
                    error_count = 0;
                    LCD_Clear();
                    LCD_ShowString(0, 0, "Enter Password:");
                    LCD_ShowString(1, 0, "                ");
                } else {
                    error_count++;
                    LCD_Clear();
                    LCD_ShowString(0, 0, "   Error!");
                    LCD_ShowString(1, 0, "  Try Again");
                    Display_String("  ERROR  ");
                    BEEP_Error();
                    delay_ms(1000);
                    
                    if(error_count >= 3) {
                        is_locked = 1;
                        lock_remain = 30;
                        LCD_Clear();
                        LCD_ShowString(0, 0, "  Locked!");
                        LCD_ShowString(1, 0, "  30 s");
                    } else {
                        LCD_Clear();
                        LCD_ShowString(0, 0, "Enter Password:");
                        LCD_ShowString(1, 0, "                ");
                    }
                }
                step = 0;
            } else {
                step = 0;
                LCD_ShowString(1, 0, "                ");
                BEEP_Error();
            }
        }
        else if(key == 'B') {
            if(step > 0) {
                step--;
                LCD_ShowChar(1, step, ' ');
                BEEP_Short();
            }
        }
    }
}