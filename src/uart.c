/**
 * @file uart.c
 
 * @author Erick Giffoni (github.com/ErickGiffoni)
 * @brief C module implementation for communication in a Raspberry 4 using UART
 * @version 0.1
 * @date 2022-02-10
 * 
 * Ref 1          : https://gitlab.com/fse_fga/exercicios/exercicio-1-uart
 * Ref 2 (MODBUS) : https://gitlab.com/fse_fga/exercicios/exercicio-2-uart-modbus
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "uart.h"

#include "../crc/crc16.h"

#include <fcntl.h>      // open
#include <stdlib.h>     // malloc
#include <unistd.h>     // write
#include <string.h>     // strncpy
#include <termios.h>    // tcgetattr, struct termios...
#include <stdio.h>

static int uartDescriptor = -1;
static struct termios commOptions;

void openUart(char *path){
   uartDescriptor = open(path, O_RDWR | O_NOCTTY | O_NDELAY);

   if(uartDescriptor == -1){
      printf("Error openning UART");
      exit(1);
   }

   return;
} // end of openUart

void closeUart(){
   close(uartDescriptor);
} // end of closeUart

/**
 * Access UART hardware and sets its configurations
 */
void setCommunicationOptions(){
   tcgetattr(uartDescriptor, &commOptions);

   commOptions.c_cflag = B9600 | CS8 | CLOCAL | CREAD;  //<Set baud rate
   commOptions.c_iflag = IGNPAR;
   commOptions.c_oflag = 0;
   commOptions.c_lflag = 0;

   tcflush(uartDescriptor, TCIFLUSH);
   tcsetattr(uartDescriptor, TCSANOW, &commOptions);

   return;
} // end of setCommunicationOptions

/**
 * Configures the message package according to MODBUS-RTU p2p
 * 
 * @param package The package to be configured
 * @param isItARequest If equals to 0, then it is not a request, meaning the user is not requesting info but sending
 * @param dataType 0 for string, 1 for int, 2 for float
 */
void configurePackage(char *package, char isItARequest, int dataType){
   package[0]  =  DEVICE_ADDRESS;
   package[1]  =  isItARequest ? CODE_23 : CODE_16;

   switch (dataType){
      case 0:
      /* string */
         package[2]  =  SEND_STRING_CODE;
         break;
   
      default:
         printf("configurePackage: wrong data type!\nExiting...\n");
         exit(2);
   }

   return;
}

void sendString(char *message, int msgLength){
   int pkgLength = 4+msgLength+2;
   char *package = (char *) malloc(pkgLength * sizeof(char));

   configurePackage(package, 0, 0);
   // package[0]  =  DEVICE_ADDRESS;
   // package[1]  =  CODE_16;
   // package[2]  =  SEND_STRING_CODE;
   package[3]  =  msgLength;

   strncpy(package+4, message, msgLength);   // without \0

   short crc16 = calcula_CRC(package, pkgLength-2);

   memcpy(&package[pkgLength-2], &crc16, 2);

   int numOfBytesWritten = 0;
   numOfBytesWritten = write(uartDescriptor, &package[0], pkgLength);

   return;
} // end of sendString