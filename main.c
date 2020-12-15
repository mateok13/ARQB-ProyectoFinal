/*
 * File:   newmain.c
 * Author: User
 *
 * Created on 9 de diciembre de 2020, 07:56 PM
 */

//configuracion de los bits para el lcd
#define RS PORTBbits.RB2
#define EN PORTBbits.RB3
#define D4 PORTBbits.RB4
#define D5 PORTBbits.RB5
#define D6 PORTBbits.RB6
#define D7 PORTBbits.RB7
//librerias que usamos
#include <xc.h>
#include "pic18f4550.h"
#define _XTAL_FREQ 8000000
#define device_id_write 0xD0
#define device_id_read 0xD1
#include "lcd.h"
#include "I2C_Master_File.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
//variables
unsigned char tecla; //variable para capturar la tecla presionada del teclado
char aux_contrasenia[4]; //variable para guardar la contraseña que se digita
char contrasenia[4] = {'2', '0', '2', '0'}; //contraseña definida para activar la seguridad
unsigned int retardo = 100; //retardo en la lectura del teclado
char buffer_TX[] = "temperatura alta\r";
char buffer_TX2[] = "temperatura regular\r";
int sec, min, hour;
char estado_dia = 'N'; //este valida que tiempo es, si es de dia o es de noche

//estado del teclado n1,n2,n3,n4 son los digitos que conforman la contraseña

enum teclado_estado {
    n1, n2, n3, n4, en
};
//estado inicial del teclado
char estado_teclado = n1;
//variable usada para validar la contraseña
int contador = 0;
//configuracion analogica del sensor termico
//este metodo lo consegui en internet para hacer la conversion analogica del sensor
//ya que este se conecta a una entrada analogica
short getCad(char canal) {
    ADCON1 = 0b00001100;
    switch (canal) {
        case 0:
            ADCON0 = 0b00000010;
            break;
    }
    ADCON2 = 0b00101001;
    ADON = 1;
    GO_DONE = 1;
    while (GO_DONE == 1);
    

    return ADRESH;
}

void RTC_Read_Clock(char read_clock_address) {
    I2C_Start(device_id_write);
    I2C_Write(read_clock_address); /* address from where time needs to be read*/
    I2C_Repeated_Start(device_id_read);
    sec = I2C_Read(0); /*read data and send ack for continuous reading*/
    min = I2C_Read(0); /*read data and send ack for continuous reading*/
    hour = I2C_Read(1); /*read data and send nack for indicating stop reading*/

}

//inicializacion del teclado

void Keypad_Init() {
    TRISD = 0B00001111; //inica el puerto del teclado
}
//metodo para capturar las teclas que se presionen del teclado

unsigned char Keypad_Key_Press(void) {
    const unsigned char keypad_deco[17] = {'7', '8', '9', '/', '4', '5', '6', '*', '1', '2', '3', '-', 13, '0', '=', '+', 0};
    unsigned int tec = 0, fila;

    for (fila = 0b00000001; fila < 0b00010000; fila <<= 1) {
        LATD = fila << 4; //lo traslada 4  posicones para sacar por las filas
        if (PORTDbits.RD0 == 1) break;
        tec++;
        if (PORTDbits.RD1 == 1) break;
        tec++;
        if (PORTDbits.RD2 == 1) break;
        tec++;
        if (PORTDbits.RD3 == 1) break;
        tec++;
    }
    PORTD = 0x00;
    return keypad_deco[tec]; //retorna el codigo deseado de la tecla
}
//lectura del teclado,se captura y valida 1 por 1 las teclas presionadas

void activar_seguridad() {
    tecla = Keypad_Key_Press(); //se captura la tecla presionada
    //configuracion pines de los leds y el sensor
    TRISA = 0b00000001;
    //leds
    PORTAbits.RA3 = 0; //buzzer alarma activada
    PORTAbits.RA5 = 0; //led seguridad activada led rojo
    PORTAbits.RA4 = 1; //led seguridad desactivada led verde
    switch (estado_teclado) {
        case n1:
            if ((tecla != 0) && (isdigit(tecla))) {
                aux_contrasenia[0] = tecla; //se guarda la tecla presionada para contruir la contraseña
                estado_teclado = n2; //se pasa al siguite digito
                Lcd_Set_Cursor(2, 1);
                Lcd_Write_String("                    "); //limpiar lcd
                Lcd_Set_Cursor(2, 8);
                Lcd_Write_Char('*');
                __delay_ms(400); // retardo antirrebote
            }
            break;

        case n2:
            if ((tecla != 0) && (isdigit(tecla))) //isdigit=si es digito
            {
                aux_contrasenia[1] = tecla; //se guarda la tecla presionada para contruir la contraseña
                estado_teclado = n3; //se pasa al siguite digito
                Lcd_Set_Cursor(2, 9);
                Lcd_Write_Char('*');
                __delay_ms(400); // retardo antirrebote
            }
            break;

        case n3:

            if ((tecla != 0) && (isdigit(tecla))) {
                aux_contrasenia[2] = tecla; //se guarda la tecla presionada para contruir la contraseña
                estado_teclado = n4; //se pasa al siguite digito
                Lcd_Set_Cursor(2, 10);
                Lcd_Write_Char('*');
                __delay_ms(400); // retardo antirrebote
            }
            break;

        case n4:
            if ((tecla != 0) && (isdigit(tecla))) {
                aux_contrasenia[3] = tecla; //se guarda la tecla presionada para contruir la contraseña
                estado_teclado = en; //se pasa a un estado que validara la contraseña
                Lcd_Set_Cursor(2, 11);
                Lcd_Write_Char('*');
                __delay_ms(400); // retardo antirrebote
            }
            break;

        case en:

            if ((tecla != 0) && (tecla != '=')) // no se presiono el igual para validar la contraseña
            {
                estado_teclado = n1; // vuelve al estado de  captura del primer numero
                Lcd_Set_Cursor(2, 8);
                Lcd_Write_String("      "); // borra la pantalla
                __delay_ms(600); // retardo antirrebote
            }// si si se presiono el enter
            else if ((tecla != 0) && (tecla == '=')) //si se presiono el igual
            {
                //ciclo para recorrer los vectores con las contraseñas (guardada,ingresada)
                for (int i = 0; i <= 3; i++) {
                    //compara la contraseña guardada "2020" con la contraseña ingresada 
                    if (contrasenia[i] == aux_contrasenia[i]) {
                        //validar contraseña
                        contador++;
                    }
                }
                //si el contador es 4 las contraseñas fueron iguales
                if (contador == 4) {
                    Lcd_Set_Cursor(2, 1);
                    Lcd_Write_String("contraseña correcta");
                    __delay_ms(600);
                    //configuracion del lcd
                    D4 = 0;
                    D5 = 0;
                    D6 = 0;
                    D7 = 0;
                    //configuracion pines led
                    ADCON1 = 0b00001111;
                    //valiable para capturar la temperatura del sensor
                    float temperatura;
                    char Temperatura[10];
                    Lcd_Clear();
                    TRISA = 0b00000001;
                    TRISC = 0b00000000;
                    Lcd_Set_Cursor(1, 3);
                    Lcd_Write_String("Temperatura:");
                    char secs[10], mins[10], hours[10];
                    I2C_Init();
                    __delay_ms(10);
                    RTC_Read_Clock(0); /*gives second,minute and hour*/
                    I2C_Stop();
                    __delay_ms(1000);
                    //hour = hour & (0x3f);
                    sprintf(secs, "%x ", sec); /*%x for reading BCD format from RTC DS1307*/
                    sprintf(mins, "%x:", min);
                    sprintf(hours, "Hora:%x:", hour);
                    Lcd_Set_Cursor(2, 3);
                    Lcd_Write_String(hours);
                    Lcd_Set_Cursor(2, 11);
                    Lcd_Write_String(mins);
                    Lcd_Set_Cursor(2, 14);
                    Lcd_Write_String(secs);
                    PORTAbits.RA5 = 1; //led seguridad activada led rojo
                    PORTAbits.RA4 = 0; //led seguridad desactivada led verde
                    //Configura UART a 9600 baudios
                    TRISCbits.RC6 = 0; //  Pin RC6 como salida digital para TX.
                    TXSTAbits.TX9 = 0; //  Modo-8bits.
                    TXSTAbits.TXEN = 1; //  Habilita Transmisión.
                    TXSTAbits.SYNC = 0; //  Modo-Asíncrono.
                    TXSTAbits.BRGH = 0; //  Modo-Baja Velocidad de Baudios.
                    BAUDCONbits.BRG16 = 0; //  Baudios modo-8bits.
                    RCSTAbits.SPEN = 1; //  Hbilita el Módulo SSP como UART.
                    SPBRG = (unsigned char) (((_XTAL_FREQ / 9600) / 64) - 1); //baudios  = 9600
                    while (contador == 4) {
                        temperatura = (getCad(0)*0.02 * 100); //conversion de la temperatura leida del sensor
                        sprintf(Temperatura, "%.2f", temperatura);
                        Lcd_Set_Cursor(1, 15);
                        Lcd_Write_String(Temperatura);
                        if (temperatura > 30) {
                            //valida si es de dia o de noche D para dia, N para noche
                            //rango noche hour <= 5 && hour >= 0 || hour >= 19 && hour <= 23
                            //rango dia hour >= 6 && hour <=18
                            if (hour > 6 && hour < 0x18) {
                                PORTCbits.RC0 = 0;
                            } else {
                                PORTCbits.RC0 = 1;
                            }
                            //registro de la temperatura en la consola
                            for (int i = 0; i < 18; i++) {
                                //  espera a que el registro de transmisión este disponible o vacio.
                                while (!TXSTAbits.TRMT) {
                                }
                                //  escribe el dato que se enviará a través de TX.
                                TXREG = buffer_TX[i];
                            }
                            PORTAbits.RA3 = 1; //buzzer alarma activada
                        } else {
                            //registro de la temperatura en la consola
                            for (int i = 0; i < 21; i++) {
                                //  espera a que el registro de transmisión este disponible o vacio.
                                while (!TXSTAbits.TRMT) {
                                }
                                //  escribe el dato que se enviará a través de TX.
                                TXREG = buffer_TX2[i];
                            }
                            PORTAbits.RA3 = 0; //buzzer alarma activada
                            PORTCbits.RC0 = 0;
                        }
                        __delay_ms(1000);
                    }
                } else {
                    aux_contrasenia[4] = 0; //final de cadena
                    retardo = atoi(aux_contrasenia); // lo pasa a numero y calcula el retardo deseado
                    estado_teclado = n1; // vuelve al estado de caprura del primer numero.
                    Lcd_Set_Cursor(2, 1);
                    Lcd_Write_String("contraseña invalida");
                    __delay_ms(600); // retardo antirrebote
                }
            }
            break;
    }
}

void securityHouse(signed int valor) {
    while (valor > 0) {
        __delay_us(700);
        activar_seguridad();
        valor--;
    }
}

void main(void) {
    TRISB = 0;
    ADCON1 = 0b00001111;
    Keypad_Init();
    Lcd_Init();
    Lcd_Clear();
    Lcd_Set_Cursor(1, 6);
    Lcd_Write_String("TECLADO");
    while (1) {
        securityHouse(retardo);
    }
}