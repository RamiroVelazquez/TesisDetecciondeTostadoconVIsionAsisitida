#include <16F877A.h>  // Archivo de configuraci�n del PIC16F877A
#fuses HS, NOWDT, NOPROTECT, BROWNOUT, PUT, NOLVP  // Configuraci�n de fusibles
#use delay(clock=20000000)  // Configuraci�n de frecuencia de reloj a 20 MHz


#use I2C(master, sda=PIN_C4, scl=PIN_C3, fast=400000)  // Configuraci�n I2C para el LCD
#include "i2c_Flex_LCD.c"  // Librer�a para control del LCD I2C

#define TIEMPO_ENCENDIDO 5000  // Duraci�n de encendido de cada etapa de vibraci�n

void main() {
    setup_ccp1(CCP_PWM);  // Configurar CCP1 en modo PWM
    setup_timer_2(T2_DIV_BY_16, 255, 1);
    set_pwm1_duty(0);  // Apagar PWM al inicio
    lcd_init(0x4E, 16, 2);  // Inicializar el LCD en la direcci�n I2C 0x4E

    // Mensaje de arranque en el LCD
    lcd_gotoxy(1, 1);
    LCD_PUTC("Iniciando...");  // Mensaje breve de inicio

    delay_ms(1000);  // Retardo reducido para cargar
    
    lcd_clear();
    delay_ms(200);  // Esperar un momento antes de continuar

    // Mensaje de bienvenida en el LCD
    lcd_gotoxy(1, 1);
    LCD_PUTC("Bienvenido!");
    lcd_gotoxy(1, 2);
    LCD_PUTC("Sistema Listo");
    delay_ms(1000);  // Mantener el mensaje visible por 1 segundo
    lcd_clear();  // Limpiar el LCD
    
    while(TRUE) {
        int duty = 0;  // Ciclo de trabajo inicial del motor apagado

        // Selecci�n de nivel de potencia con PWM en funci�n de los pines de entrada
        if(input(PIN_B0)) {  // Nivel de potencia 1 (26% de PWM)
            duty = (int)(0.26 * 1023);
            lcd_gotoxy(1, 1);
            lcd_putc("Potencia Nivel 1");  // Mostrar nivel de vibraci�n 1
        } 
        else if(input(PIN_B1)) {  // Nivel de potencia 2 (52% de PWM)
            duty = (int)(0.52 * 1023);
            lcd_gotoxy(1, 1);
            lcd_putc("Potencia Nivel 2");  // Mostrar nivel de vibraci�n 2
        } 
        else if(input(PIN_B2)) {  // Nivel de potencia 3 (80% de PWM)
            duty = (int)(0.80 * 1023);
            lcd_gotoxy(1, 1);
            lcd_putc("Potencia Nivel 3");  // Mostrar nivel de vibraci�n 3
        }

        // Si el bot�n de acci�n (PIN_B3) se presiona, iniciar el motor con el nivel de potencia seleccionado
        while((input(PIN_B0) && input(PIN_B3)) || (input(PIN_B1) && input(PIN_B3)) || (input(PIN_B2) && input(PIN_B3))) {
            for(int i = 0; i < 3; i++) {  // Repetir tres veces para las etapas de vibraci�n
                lcd_clear();  // Limpiar el LCD al inicio de cada etapa
                lcd_gotoxy(1, 1);

                // Mostrar la etapa actual en el LCD
                LCD_PUTC("Etapa V. y R.:");
                lcd_gotoxy(1, 2);
                lcd_putc((i + 1) + '0');  // Mostrar el n�mero de la etapa (1, 2, o 3)
                
                set_pwm1_duty(duty);
                delay_ms(1500);  // Reducir el retardo para disminuir la carga en Proteus
                
                set_pwm1_duty(0);
                delay_ms(500);  // Reducir pausa para evitar sobrecarga
                
                // Activar el ESPCAM en cada etapa
                output_high(PIN_D1);  // Activar ESPCAM para captura de im�genes
                delay_ms(2000);  // Reducir tiempo de activaci�n de ESPCAM
                output_low(PIN_D1);  // Apagar ESPCAM
            }
            lcd_clear();  // Limpiar el LCD despu�s de completar las tres etapas
            break;
         }
           set_pwm1_duty(0);  // Si no se presiona el bot�n B3, el motor permanece apagado
        }
    }

