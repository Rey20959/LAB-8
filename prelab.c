/* 
 * File:   prelab.c
 * Author: juane
 *
 * Created on 16 de abril de 2022, 03:43 PM
 */
// PIC16F887 Configuration Bit Settings

// 'C' source line config statements

/*
 * --------------------------- Configracion de bits --------------------------
 */
// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillador Interno sin salidas, XT
#pragma config WDTE = OFF       // WDT disabled (reinicio repetitivo del pic)
#pragma config PWRTE = OFF     // PWRT enabled  (espera de 72ms al iniciar)
#pragma config MCLRE = OFF      // El pin de MCLR se utiliza como I/O
#pragma config CP = OFF         // Sin protección de código
#pragma config CPD = OFF        // Sin protección de datos
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit 
#pragma config FCMEN = OFF       // Fail-Safe Clock Monitor Enabled bit 
#pragma config LVP = OFF         // Low Voltage Programming Enable bit 

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit 
#pragma config WRT = OFF        // Flash Program Memory Self Write 

/*
 * --------------------------- Librerias --------------------------
 */

#include <xc.h>
#include <stdint.h>


/*
 * ---------------------------- Constantes -----------------------------------
 */
#define _tmr0_value 178 ///20 MS
#define _XTAL_FREQ 4000000      //__delay_us(x)

/*
 * ----------------------------- Variables ----------------------------------
 */
char unidad, decena, centena;   //1 BYTE
int dividendo1;                  // Conversion
int dividendo;                   // Conversion
int division;                    // Conversion
char bandera;                    // Para interrupciones

/*
 * ------------------------------- Tabla ------------------------------------
 */
char tabla [10] = {
   0B00111111,	//0
   0B00000110,	//1
   0B01011011,	//2
   0B01001111,	//3
   0B01100110,	//4
   0B01101101,  //5
   0B01111101,	//6
   0B00000111,  //7 
   0B01111111,	//8
   0B01101111,	//9
};

/*
 * -------------------------- Prototipos de funciones -----------------------
 */
void setup (void);
/*
 * ------------------------------ Interrupción -------------------------------
 */
void __interrupt () isr (void)
{
    if (PIR1bits.ADIF)   // Interrupcion ADC
    {
        if(ADCON0bits.CHS == 6)
            PORTB = ADRESH;
        else
            dividendo1 = ADRESH;
        PIR1bits.ADIF = 0;
        
    }
    
        PORTDbits.RD0 = 0;      //Clear a los transistores 
        PORTDbits.RD1 = 0;      //Clear a los transistores
        PORTDbits.RD2 = 0;      //Clear a los transistores
        
    if (bandera == 3)
    {            
        PORTC = (tabla[centena]);     //En el display aparecerá la centena
        PORTDbits.RD0 = 1;            //Encender transistor
        PORTCbits.RC7 = 1;            //Encender punto del display

    }

    else if (bandera == 2)
    {
        PORTC = (tabla[decena]);    //En el display aparecerá la decena
        PORTDbits.RD1 = 1;          //Encender transistor
    }

    else if (bandera == 1)
    {
        PORTC = (tabla[unidad]);    //En el display aparecerá la unidad
        PORTDbits.RD2 = 1;          //Encender transistor
    }   

    bandera--;                      //Decrementa bandera
    if (bandera == 0)
    {
        bandera = 3;        //Volver a repetir el ciclo 
    }
    INTCONbits.T0IF = 0;
    TMR0 = _tmr0_value;
       
}

/*
 * ----------------------------- MAIN CONFIGURACION --------------------------- 
 */

void main (void)
{
    setup();
    ADCON0bits.GO = 1;
    while (1)   
    {
        if (ADCON0bits.GO == 0) 
        {
            if (ADCON0bits.CHS == 6)
                ADCON0bits.CHS = 5;
            else
            ADCON0bits.CHS = 6;
            __delay_us(50);
            ADCON0bits.GO = 1;
        }
        
        //----------- loop de conversion  binario a centenas -----------------
        {
            division = (dividendo1 << 2)+4;
            dividendo = division >>1;
            
            centena = dividendo/100;    //Centenas
            decena = (dividendo -(100 * centena))/10;   //Decenas
            unidad = dividendo - (100* centena) -(decena *10);      //Unidades
        }
       
    
         }
}
/*
 * -------------------------------- Funciones --------------------------------
 */

void setup(void)
{
    //--------------- Configuración de entradas y salidas --------------
    ANSEL = 0b01100000;
    ANSELH = 0;
    
    TRISB = 0;  //Salida
    TRISC = 0;  //Salida
    TRISD = 0;  //Salida
    TRISE = 0b011;  //Entrada
    

    //Asegurarnos que empiecen en cero
    PORTA = 0;
    PORTC = 0;
    PORTE = 0;
    PORTD = 0;
    
    //Valores iniciales
    centena = 0;
    decena = 0;
    unidad = 0;
    bandera = 3;        //Empieza en 3
    
    //---------------------- Configuración del oscilador ----------------------
    OSCCONbits.IRCF = 0b0110;   //4MHz
    OSCCONbits.SCS = 1;
    
    //------------------ Configuracipon del tmr0 ------------------------
    //Timer0 Registers Prescaler (TRP) = 256 
    //TRP - TMR0 Preset = 178 - Freq = 49.45 Hz - Period = 0.020224 seconds
    OPTION_REGbits.T0CS = 0;  // bit 5  TMR0 Clock Source Select bit
    OPTION_REGbits.T0SE = 0;  // bit 4 TMR0 Source Edge Select bit ¿
    OPTION_REGbits.PSA = 0;   // bit 3  Prescaler Assignment bit
    OPTION_REGbits.PS2 = 1;   // bits 2-0  PS2:PS0: Prescaler Rate Select bits
    OPTION_REGbits.PS1 = 1;
    OPTION_REGbits.PS0 = 1;
    TMR0 = _tmr0_value;             // preset for timer register

    INTCONbits.T0IF = 0;        //Deshabilitar la bandera
    INTCONbits.T0IE = 1;        //Activar interrupcon Timer 0
    INTCONbits.GIE = 1;         //Habilitar interrupciones
    
    //----------------------- Configuración del ADC ---------------------------
    ADCON1bits.ADFM = 0;        //Justificado a la izquierda
    ADCON1bits.VCFG0 = 0;       //voltaje de 0V-5V
    ADCON1bits.VCFG1 = 0;
    
    ADCON0bits.ADCS = 1;        //Fosc/8
    ADCON0bits.CHS = 5;         //Canal 5
    ADCON0bits.ADON = 1;        //Activa el modulo
     __delay_us(50);
    
    //------------------ Configuración de las interrupciones  -----------------
    PIR1bits.ADIF = 0;          //Deshabilita la bandera de conversion A/D
    PIE1bits.ADIE = 1;          //Activa el enable para la conversion de inter.
    INTCONbits.PEIE = 1;        //Activiar interrupciones perifericas
    
    return;

}