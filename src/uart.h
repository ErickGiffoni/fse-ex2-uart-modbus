/**
 * @file uart.h
 
 * @author Erick Giffoni (github.com/ErickGiffoni)
 * @brief C module for communication in a Raspberry 4 using UART
 * @version 0.1
 * @date 2022-02-10
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef  UART_H
#define  UART_H

#define DEVICE_ADDRESS     0x01

#define CODE_23            0x23
#define CODE_16            0x16

#define RES_PKG_LEN        255   // length of response
#define INT_PKG_LEN        9     // 3+4+2
#define FLOAT_PKG_LEN      9

#define SEND_STRING_CODE   0xB3
#define SEND_INT_CODE      0xB1
#define SEND_FLOAT_CODE    0xB2

void openUart(char *path);
void closeUart();
void setCommunicationOptions();
void writeToUart(char *package, int pkgLength);

void setCrc(char *package, int pkgLength);
char verifyCrc(char *package, int pkgLength);

void getResponse();

void sendString(char *message, int msgLength);
void getStringResponse();

void sendInt(int number);
void getIntResponse();

void sendFloat(float number);
void getFloatResponse();

#endif