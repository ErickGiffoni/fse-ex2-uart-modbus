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

#define INT_PKG_LEN        9     // 3+4+2

#define SEND_STRING_CODE   0xB3
#define SEND_INT_CODE      0xB1

void openUart(char *path);
void closeUart();
void setCommunicationOptions();
void writeToUart(char *package, int pkgLength);

void setCrc(char *package, int pkgLength);
char verifyCrc(char *package, int pkgLength);

void sendString(char *message, int msgLength);
void getStringResponse();

void sendInt(int number);
// void getIntResponse();

#endif