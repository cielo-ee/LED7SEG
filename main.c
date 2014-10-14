/*
 * File:   main.c
 * Author: syu
 * 7 segment LED unit
 * Created on 2014/09/27, 14:15
 */

#include <pic.h>
#include "thermo.h"

//#define _XTAL_FREQ 19660800
#define _XTAL_FREQ 20000000


#pragma config FOSC   = HS
#pragma config WDTE   = 0N
#pragma config PWRTE  = ON
#pragma config MCLRE  = OFF
#pragma config CP     = OFF
#pragma config BOREN  = OFF
#pragma config IESO   = OFF
#pragma config FCMEN  = OFF
#pragma config LVP    = OFF
#pragma config DEBUG  = OFF


#pragma config BOR4V = BOR40V
#pragma config WRT   = OFF


#pragma config IDLOC1 = 0x78;
#pragma config IDLOC2 = 0x76;

unsigned char timer5ms;
unsigned char led7_dig;

unsigned char led7_ptn[4];
unsigned char sw_status;
unsigned int freq;

const unsigned char font7s[] = {
    0b11101011,
    0b10110011,
    0b10110011,
    0b10111010,
    0b01111000,
    0b11011010,
    0b11011011,
    0b10101000,
    0b11111011,
    0b11111010,
    0b11001111,
    0b11011010,
    0b11001111,
    0b11001111,
    0b11011010,
    0b01011011,
    0b11011100,
    0b01011011,
    0b01001011
};

const unsigned char digisel[] = {
    0b00010000,
    0b00100000,
    0b01000000,
    0b10000000
};

void interrupt entry(){
    unsigned char s;

    if(INTCONbits.T0IF){
        INTCONbits.T0IF = 0;
        TMR0 += 64;


        PORTC &= 0b00001111;
        PORTB = led7_ptn[led7_dig];
        PORTC = digisel[led7_dig];
        led7_dig++;
        led7_dig &= 0b000000111;

        s = PORTC & 0b000001110;
        if(s != 0b00001110){
            sw_status = s >> 1;

        }
        if(++timer5ms >= 200){
            timer5ms = 0;
            T1CONbits.TMR1ON = 0;
            if(PIR1bits.TMR1IF){
                freq = 0xffff;
                PIR1bits.TMR1IF = 0;
            }else{
                freq = TMR1;
            }
            TMR1 = 0;
            T1CONbits.TMR1ON = 1;
        }
    }

}

void led7_putd(int n){
    unsigned char i;

    if(n > 9999){
        led7_ptn[3] = 0b00011011;
        led7_ptn[2] = 0b11010001;
        led7_ptn[1] = 0b01000011;
        led7_ptn[0] = 0b00011011;
        return;
    }
    if(n < -999){
        led7_ptn[3] = 0b01101011;
        led7_ptn[2] = 0b11010001;
        led7_ptn[1] = 0b01000011;
        led7_ptn[0] = 0b00011011;
        return;
    }

    if(n == 0){
        led7_ptn[3] = 0b00000000;
        led7_ptn[2] = 0b00000000;
        led7_ptn[1] = 0b00000000;
        led7_ptn[0] = 0b11101011;
        return;
    }

    if(n < 0){
        n = -n;
        for(i = 0; i < 3 && n > 0; i++){
            led7_ptn[i] = font7s[ n % 10];
            n /= 10;
        }
        led7_ptn[i++] = 0b00010000;
    }
    else{
        for(i = 0; i< 4 && n > 0; i++){
            led7_ptn[i] = font7s[n%10];
            n /= 10;
        }
    }
    for(;i<4;i++){
        led7_ptn[i] = 0b00000000;
    }
}

void main(){
    int tmp;
    int hum;
    int atm;
    unsigned long adc;

    sw_status = 0b00000111;
    timer5ms = 0;
    led7_dig = 0;


    ANSELH = 0;
    ADCON0 = 0b10000001;
    ADCON1 = 0b10000000;
    TRISB  = 0;
    TRISC &= 0b0001111;
    OPTION_REG = 0b100001110;
    INTCONbits.T0IF = 0;
    INTCONbits.T0IE = 1;
    T1CON = 0b00000111;
    PIR1bits.TMR1IF = 0;

    INTCONbits.PEIE = 1;
    ei();

    while(1){
        switch(sw_status){

            case 3:
                tmp = ftot(freq);
                led7_putd(tmp);
                led7_ptn[1] |= 0b00000100;
                __delay_ms(500);
                break;
            case 5:
                ADCON0bits.CHS = 0;
                __delay_ms(500);
                ADCON0bits.GO_nDONE = 1;
                while(ADCON0bits.GO_nDONE);
                adc = ADRESH * 256 + ADRESL;
                hum = adc * 1110 / 1024 - 95;
                led7_putd(hum);
                led7_ptn[1] |= 0b00000100;
                break;
            case 6:
                ADCON0bits.CHS = 1;
                __delay_ms(500);
                ADCON0bits.GO_nDONE = 1;
                while(ADCON0bits.GO_nDONE);
                adc = ADRESH * 256 + ADRESL;
                atm = adc * 122 / 1024 + 952;
                led7_putd(atm);
                break;
            default:
                led7_ptn[3] = 0b00010001;
                led7_ptn[2] = 0b00111011;
                led7_ptn[1] = 0b01111010;
                led7_ptn[0] = 0b00100100;
        }
    }
}