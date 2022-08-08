

#define _XTAL_FREQ   4000000UL     // needed for the delays, set to 4 MH= your crystal frequency
// CONFIG1H
#pragma config OSC = XT         // Oscillator Selection bits (XT oscillator)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor disabled)
#pragma config IESO = OFF       // Internal/External Oscillator Switchover bit (Oscillator Switchover mode disabled)

// CONFIG2L
#pragma config PWRT = OFF       // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = SBORDIS  // Brown-out Reset Enable bits (Brown-out Reset enabled in hardware only (SBOREN is disabled))
#pragma config BORV = 3         // Brown Out Reset Voltage bits (Minimum setting)

// CONFIG2H
#pragma config WDT = ON         // Watchdog Timer Enable bit (WDT enabled)
#pragma config WDTPS = 32768    // Watchdog Timer Postscale Select bits (1:32768)

// CONFIG3H
#pragma config CCP2MX = PORTC   // CCP2 MUX bit (CCP2 input/output is multiplexed with RC1)
#pragma config PBADEN = ON      // PORTB A/D Enable bit (PORTB<4:0> pins are configured as analog input channels on Reset)
#pragma config LPT1OSC = OFF    // Low-Power Timer1 Oscillator Enable bit (Timer1 configured for higher power operation)
#pragma config MCLRE = ON       // MCLR Pin Enable bit (MCLR pin enabled; RE3 input pin disabled)

// CONFIG4L
#pragma config STVREN = ON      // Stack Full/Underflow Reset Enable bit (Stack full/underflow will cause Reset)
#pragma config LVP = ON         // Single-Supply ICSP Enable bit (Single-Supply ICSP enabled)
#pragma config XINST = OFF      // Extended Instruction Set Enable bit (Instruction set extension and Indexed Addressing mode disabled (Legacy mode))

// CONFIG5L
#pragma config CP0 = OFF        // Code Protection bit (Block 0 (000800-003FFFh) not code-protected)
#pragma config CP1 = OFF        // Code Protection bit (Block 1 (004000-007FFFh) not code-protected)
#pragma config CP2 = OFF        // Code Protection bit (Block 2 (008000-00BFFFh) not code-protected)
#pragma config CP3 = OFF        // Code Protection bit (Block 3 (00C000-00FFFFh) not code-protected)

// CONFIG5H
#pragma config CPB = OFF        // Boot Block Code Protection bit (Boot block (000000-0007FFh) not code-protected)
#pragma config CPD = OFF        // Data EEPROM Code Protection bit (Data EEPROM not code-protected)

// CONFIG6L
#pragma config WRT0 = OFF       // Write Protection bit (Block 0 (000800-003FFFh) not write-protected)
#pragma config WRT1 = OFF       // Write Protection bit (Block 1 (004000-007FFFh) not write-protected)
#pragma config WRT2 = OFF       // Write Protection bit (Block 2 (008000-00BFFFh) not write-protected)
#pragma config WRT3 = OFF       // Write Protection bit (Block 3 (00C000-00FFFFh) not write-protected)

// CONFIG6H
#pragma config WRTC = OFF       // Configuration Register Write Protection bit (Configuration registers (300000-3000FFh) not write-protected)
#pragma config WRTB = OFF       // Boot Block Write Protection bit (Boot Block (000000-0007FFh) not write-protected)
#pragma config WRTD = OFF       // Data EEPROM Write Protection bit (Data EEPROM not write-protected)

// CONFIG7L
#pragma config EBTR0 = OFF      // Table Read Protection bit (Block 0 (000800-003FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR1 = OFF      // Table Read Protection bit (Block 1 (004000-007FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR2 = OFF      // Table Read Protection bit (Block 2 (008000-00BFFFh) not protected from table reads executed in other blocks)
#pragma config EBTR3 = OFF      // Table Read Protection bit (Block 3 (00C000-00FFFFh) not protected from table reads executed in other blocks)

// CONFIG7H
#pragma config EBTRB = OFF      // Boot Block Table Read Protection bit (Boot Block (000000-0007FFh) not protected from table reads executed in other blocks)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.
#include <xc.h>
#include <stdio.h>
#include "my_ser.h"
#include "my_adc.h"
#include "lcd_x8.h"
#include <xc.h>

#define STARTVALUE  3036
float HS = 0.0;
int mode =0;
float temp, sp ;
float tempf;
float spf;
char Buffer[32];
unsigned int RPS_count=0;
unsigned int RPS=0;
int ft=0;
int start_flag=0;
int timer_flag=0;
int sec=0;
int min =0;
int hour=0;
int flag=2;
//char* m3="Count Heat";
void reloadTimer0(void)
{  
    TMR3H = (unsigned char) ((STARTVALUE >>  8) & 0x00FF);
    TMR3L =  (unsigned char)(STARTVALUE & 0x00FF ); 
    
}

void Timer0_isr(void)
{
  
    
    INTCONbits.TMR0IF=0;
   // PORTCbits.RC5=!(PORTCbits.RC5);
 
    timer_flag=1;
   
    reloadTimer0();
  
}
void EXT_Int1_isr(void)
{
    INTCON3bits.INT1IF=0;
    start_flag=1;
    
}
void EXT_Int0_isr(void)
{
    INTCONbits.INT0IF=0;
  
    if(mode==4)
    {
        mode=0;
    }
    else
    {
        mode=mode+1;
    }
    
}
void EXT_Int2_isr(void)
{
    INTCON3bits.INT2IF=0;
    PORTCbits.RC5=0;
    start_flag=0;
}




void __interrupt(high_priority)highIsr(void)
//void interrupt high_priority highIsr(void)
{
    if(INTCONbits.TMR0IF) Timer0_isr();
       if(INTCON3bits.INT2IF)EXT_Int2_isr();
     if(INTCON3bits.INT1IF)EXT_Int1_isr();

     if (INTCONbits.INT0IF)EXT_Int0_isr();
     
    
    
   
}
void delay_ms(unsigned int n)
{
    int i;
    for (i=0; i < n; i++){
         __delaywdt_ms(1) ; 
    }
}
void setupPorts(void)
{
    ADCON0 =0;
    ADCON1 = 0x0C; //3analog input
    TRISB = 0xFF; // all pushbuttons are inputs
    TRISC = 0x80; // RX input , others output
    PORTC =0x00;
    TRISA = 0xFF; // All inputs
    TRISD = 0x00; // All outputs
    TRISE= 0x00;  // All outputs
    
}
void display(void){
char LCD[64];
    char LCDP[64];
    unsigned char C;
    unsigned char H ;
      
    
    
     if(PORTCbits.RC5 == 1){
        H = 'O';
    }
    else{
        H = 'F';
    }
    
    if( start_flag ==1 &&(sec>0 || min>0 || hour >0)){
        C = 'O';
    }
    else{
        C = 'F';
    }
    
    
    switch(mode){
        case 0:
            sprintf(LCDP, "sec           ");
            break;
        
        case 1:
            sprintf(LCDP, "10 sec          ");
            break;
        case 2:
            sprintf(LCDP, "min          ");
            break;
        case 3:
            sprintf(LCDP, "10 min      ");
            break;
        case 4:
            sprintf(LCDP, "hour      ");
            break;
        
    }
    
    
    
     lcd_gotoxy(1, 1);
    sprintf(LCD, "Time:%02d:%02d:%02d ", hour,min,sec);
    lcd_puts(LCD);
    
    lcd_gotoxy(1, 2);
    sprintf(LCD, "CT:%6.2fC", temp);
    lcd_puts(LCD);
    
    lcd_gotoxy(12, 2);
    sprintf(LCD, "CK:");
    lcd_puts(LCD);
    lcd_gotoxy(12, 3);
    sprintf(LCD, "HT:");
    lcd_puts(LCD);
    
    lcd_gotoxy(1, 3);
    sprintf(LCD, "SP:%6.2fC",sp);
    lcd_puts(LCD);
    
    lcd_gotoxy(15, 3);
    lcd_putc(H);
    
   
    lcd_gotoxy(15, 2);
    lcd_putc(C);
    
    
    
    lcd_gotoxy(1, 4);
    sprintf(LCD, "MD: ");
    lcd_puts(LCD);
    
    lcd_gotoxy(5, 4);
    lcd_puts(LCDP);
    



}

void main(void) {
    setupPorts();
    init_adc_no_lib();
    INTCON3bits.INT2IF=1;
    INTCON = 0; // disable interrupts first, then enable the ones u want
    
    INTCON = 0;
    RCONbits.IPEN =0;
    
    INTCONbits.INT0IE = 1;
    INTCONbits.TMR0IE=1;
    
    
    INTCON2 = 0;
    
    INTCON3 = 0;
    INTCON3bits.INT1IE = 1;
    
    INTCON2bits.INTEDG1 = 1;
    INTCON2bits.INTEDG0= 1;
    T0CON=0X80;
    
    PIE1 = 0;
    PIR1 = 0;
    IPR1 = 0;
    PIE2 = 0;
    PIE2 = 0;
    PIR2 = 0;
    IPR2 = 0;
    
    INTCONbits.GIEH = 1;  // enable global interrupt bits
    INTCONbits.GIEL = 1;
   
   lcd_init();
   lcd_send_byte(0, 1);  
   
     PORTCbits.RC1=1;
     start_flag=0;
    while(1)
    {
         
        CLRWDT();  // no need for this inside the delay below
        //PORTDbits.RD5 = !PORTDbits.RD5; 
      //  delay_ms(5000);//read ADC AN0,AN1, AN2 every 5 seconds
        
        spf = read_adc_voltage(0);
        sp=spf*40.0;
        
        
        tempf =read_adc_voltage(2);
        temp=tempf*200.0;
        
     
       
         if(temp <= sp) {
                    PORTCbits.RC5 = 1;
                } 
                else if(temp >= (sp + HS)) {
                    PORTCbits.RC5 = 0;
    
            }
 
        
        
        
        
        if(start_flag==1){
            if(sec>0 || min>0 || hour>0     ){
            if(sec==0 && min==0){
                hour=hour-1;
                min=59;
                sec=59;
                
            }
            if(sec==0){
                min=min-1;
            sec=59;
            }
            
            sec--;
            }
            
            if(sec==0&& min==0&&hour==0)
            {
                flag=1;
            }
            
        }
            
              
        if(flag){
             for(int i=0;i<4;i++){
             PORTCbits.RC1=0;
             delay_ms(100);
             PORTCbits.RC1=1;
                 
                 
             }
             flag=0;
            
        }
        
       
        
        
        if(PORTBbits.RB3==0){
            delay_ms(500);
            if(mode==0){
                if(sec<59)
                sec++;
                else {
                   
                    sec=0;
                    min++;
                     if(min==60){
                         min=0;
                         hour++;
                    }
                }  
                
            }
            else if(mode==1){
                if(sec<50)
                sec=sec+10;
                
                else{
                    
                    sec=sec-50;
                    min++;
                    if(min==60){
                        hour++;
                       min=0; 
                    }
                }    
                
            }
            else if(mode==2){
                if(min<59)
                min++;
                else {
                    min=0;
                    hour++;
                    if(hour==25){
                        hour--;
                        min=0;
                        sec=0;
                    }
                } 
                
            }
            
            else if(mode==3){
                if(min<50)
                min=min+10;
                else{
                    min=min+10;
                    hour++;
                    min=min-60;
                   if(hour==25){
                        hour--;
                        min=0;
                        sec=0;
                    }
                } 
                
            }
            
            else if(mode==4){
                if(hour==24){
                    
                }
                else
                {
                    hour++;
                }
                    
                    
                    
            }
        }
        
        
        
        
        if(PORTBbits.RB4==0){
              delay_ms(500);
            if(mode==0){
                if(sec==0){
                    if(min>0)
                min--;
                sec=59;
                if(min==0 && hour >0 ){
                    hour--;
                    min=59;
                    sec=59;
                    
                }
                
                
                }
                else if(sec==0 && min==0 && hour==0)
                {
                
                }
                else {
                   
                    sec--;
                } 
                
                
            }
            else if(mode==1){
                
                if(sec>=10){
                    sec=sec-10;
                }
                else if(sec<10 && min>0){
                    min--;
                    sec=sec+60;
                    sec=sec-10;
                }
                else if (sec<10 && min==0 &&hour>0){
                    hour--;
                    min=59;
                    sec=sec+60;
                    sec=sec-10;
                }
               else if(sec==0 && min==0 && hour==0)
                {
                
                }
                
                
            }
            else if(mode==2){
                if(min==0){
                    if(hour>0){
                hour--;
                min=59;}
                if(hour==0 ){
                   
                    
                }
                
                
                }
                else if(sec==0 && min==0 && hour==0)
                {
                
                }
                else {
                   
                    min--;
                } 
                
            }
            
            else if(mode==3){
                  if(min>10){
                    min=min-10;
                }
                else if(min<10 && hour>0){
                    hour--;
                    min=min+60;
                    min=min-10;
                }
                else if (min<10 && hour==0){
                    
                }
              
            }
            
            else if(mode==4){
                if(hour>0){
                    hour--;
                }
                else if(hour==0)
                {
                   
                }
                    
                    
                    
            }
            
            
        }
           if(PORTBbits.RB5==0){
               
        
            sec=0;
            min=0;
            hour=0;
            
            start_flag=0;
           
            PORTCbits.RC5 = 0;
        }
        
    
    
        
     timer_flag=0;
      
        
        
        
        
        display();
        
        
    }
    return;
}