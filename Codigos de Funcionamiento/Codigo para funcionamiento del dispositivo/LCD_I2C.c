#include <16F877A.h>
#use delay(clock=20000000)

#define I2C_SDA_PIN PIN_C4
#define I2C_SCL_PIN PIN_C3

#use I2C(master, sda=I2C_SDA_PIN, scl=I2C_SCL_PIN, fast=400000)

#define LCD_I2C_ADDRESS 0x27

void lcd_i2c_send_nibble(unsigned char data) {
   i2c_start();
   i2c_write(LCD_I2C_ADDRESS);
   i2c_write(data | 0x04);  // Enviar los primeros 4 bits (EN=1)
   delay_us(50);
   i2c_write(data & ~0x04);  // EN=0
   delay_us(50);
   i2c_stop();
}

void lcd_i2c_send_byte(unsigned char data) {
   lcd_i2c_send_nibble(data & 0xF0);  // Enviar los 4 bits superiores
   lcd_i2c_send_nibble((data << 4) & 0xF0);  // Enviar los 4 bits inferiores
}

void lcd_i2c_command(unsigned char cmd) {
   lcd_i2c_send_byte(cmd);  // Enviar comando
}

void lcd_i2c_putc(char data) {
   lcd_i2c_send_byte(data | 0x01);  // RS = 1 para enviar datos
}

void lcd_i2c_gotoxy(unsigned char x, unsigned char y) {
   unsigned char address = (y == 1) ? 0x80 : 0xC0;
   address += (x - 1);
   lcd_i2c_command(address);
}

void lcd_i2c_init() {
   delay_ms(50);  // Esperar por la inicialización del LCD
   lcd_i2c_command(0x30);  // Modo de 8 bits
   lcd_i2c_command(0x02);  // Modo de 4 bits
   lcd_i2c_command(0x28);  // Modo de 2 líneas, 4 bits
   lcd_i2c_command(0x0C);  // Encender el LCD, apagar cursor
   lcd_i2c_command(0x06);  // Incrementar cursor automáticamente
   lcd_i2c_command(0x01);  // Limpiar pantalla
}

