//AUTOR: CARLOS ESCOBAR
//FECHA DE CREACIÓN: 08/05/2023
//ÚLTIMA MODIFICACIÓN: 08/05/2023

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

/*******************************LIBRERIAS***********************************/
#include <stdio.h>
#include <xc.h>
#include <stdint.h>
#include <stdlib.h>
#include <pic16f887.h>
#include "librerias_lab1.h"

#define _XTAL_FREQ 4000000 //FRECUENCIA DEL OSCILADOR 
uint8_t contador = 0;

int ADC_disp_val;

uint8_t flags;
uint8_t display_val[2];

/*uint8_t tabla [16]= {
    ~0b00111111, //0
    ~0b00000110, //1
    ~0b01011011, //2
    ~0b01100110, //3
    ~0b01101101, //4
    ~0b01111101, //5
    ~0b00000111, //6
    ~0b01111111, //7
    ~0b01101111, //8
    ~0b01110111, //9
    ~0b01111100, //10
    ~0b00111001, //11
    ~0b01011110, //12
    ~0b01111001, //13
    ~0b01110001, //14
    ~0b01110111  //15
} ;*/


uint8_t tabla_hex [16] = {
    // Tabla para mostrar los dígitos hexadecimales en el display de 7 segmentos
    ~0b00111111, //0
    ~0b00000110, //1
    ~0b01011011, //2
    ~0b01001111, //3
    ~0b01100110, //4
    ~0b01101101, //5
    ~0b01111101, //6
    ~0b00000111, //7
    ~0b01111111, //8
    ~0b01101111, //9
    ~0b01110111, //A
    ~0b01111100, //B
    ~0b00111001, //C
    ~0b01011110, //D
    ~0b01111001, //E
    ~0b01110001  //F
};


//PROTOTITPOS
void setup(void); //PROTOTIPO DE LA FUNCIÓN DE CONFIGURACIÓN

void __interrupt() isr(void) //INTERRUPCIONES 
{
    if (INTCONbits.RBIF == 1)
    {
        if (PORTBbits.RB0 == 0)
        {
            PORTD++;
            contador = PORTD;
            INTCONbits.RBIF = 0; //LIMPIA LA BANDERA
        }
        if (PORTBbits.RB1 == 0)
        {
            PORTD--;
            contador = PORTD;
            INTCONbits.RBIF = 0; //LIMPIA LA BANDERA
        }
    }
    
    if (PIR1bits.ADIF == 1) //SI LA BANDERA DEL ADC ESTÁ ACTIVADA   
    {
        if (ADCON0bits.CHS == 0b0000)
        {
            ADC_disp_val = adc_read();
        }
        else 
        {
            PORTC = ADRESH;
        }
        ADIF = 0;
        return;
    }
    
    //ITERRUPCIONES DEL TIMER 0
    if (INTCONbits.T0IF)
    {
        PORTE = 0;
        if (flags == 0)
        {
            PORTC = display_val[0];
            PORTE = 1;
            flags = 1;
        }
        else if (flags == 1)
        {
            PORTC = display_val[1];
            PORTE = 2;
            flags = 0;
        }
        TMR0 = 6;
        INTCONbits.T0IF = 0; //LIMPIA LA BANDERA
    }
    
    
}

void main(void)
{
    setup();
    ioc_init(0);
    ioc_init(1);
    ADCON0bits.GO = 1; //REALIZA CONVERSIÓN DEL ADC
    
    while(1)
    {
        uint8_t hex_value = ADC_disp_val & 0xFF;

        // Obtiene los dígitos individuales del valor hexadecimal
        uint8_t hex_digit1 = hex_value >> 4; // Dígito más significativo
        uint8_t hex_digit2 = hex_value & 0x0F; // Dígito menos significativo

        // Muestra los dígitos en los displays
        display_val[0] = tabla_hex[hex_digit2];
        display_val[1] = tabla_hex[hex_digit1];
        
        if (ADCON0bits.GO == 0)
        {
            __delay_ms(10);
            ADCON0bits.GO = 1; //REALIZA CONVERSIÓN DEL ADC
        }
        
        if (contador <= ADRESH)
        {
            PORTAbits.RA2 = 1; //SI EL CONTADOR ES MENOR A ADRESH
        }
        else
        {
            PORTAbits.RA2 = 0; //SI ES MAYOR SE APAGA EL LED
        }
    }
}

void setup(void)
{
    ANSEL = 0b00000001;
    ANSELH = 0x00; //TODOS COMO DIGITALES
    TRISA = 0b00000001; //SALIDAS
    TRISB = 0x00; //ENTRADAS EL PIN 0 Y 1
    TRISC = 0x00;
    TRISD = 0x00;
    TRISE = 0x00;
    
    //LIMPIEZA DE PUERTOS
    PORTA = 0x00;
    PORTB = 0x00;   
    PORTC = 0x00;
    PORTD = 0x00;
    PORTE = 0x00;
    
    //CONFIGURACIÓN DEL OSCILADOR
    OSCCONbits.IRCF = 0b110; //4 Mhz
    OSCCONbits.SCS = 1; //RELOJ INTERNO
    
    //HBAILITA INTERRUPCIONES
   
    adc_init(0); //CANAL ANALÓGICO 0
    
    //CONFIGURACIÓN DEL TIMER 0
    OPTION_REGbits.T0CS = 0;//temporizador
    OPTION_REGbits.PSA = 0;
    OPTION_REGbits.PS = 0b001; //1:4
    TMR0 = 6;
    
    INTCONbits.T0IF = 0;
    INTCONbits.TMR0IE = 1;    
    
    INTCONbits.PEIE = 1;
    INTCONbits.GIE = 1; //HABILITA LAS INTERRUPCIONES GLOBALES
   
    return;
}
