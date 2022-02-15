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

static int              uartDescriptor = -1;
static struct termios   commOptions;
static char             responsePackage[RES_PKG_LEN+1];
static int              globalResPkgLen = 0;

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

void writeToUart(char *package, int pkgLength){
   int numOfBytesWritten = 0;
   numOfBytesWritten = write(uartDescriptor, &package[0], pkgLength);

   return;
} // end of writeToUart

void setCrc(char *package, int pkgLength){
   short crc16 = calcula_CRC(package, pkgLength-2);
   memcpy(&package[pkgLength-2], &crc16, 2);

   return;
} // end of setCrc

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
         
      case 1:
      /* int */
         package[2]  =  SEND_INT_CODE;
         break;

      case 2:
      /* float */
         package[2]  =  SEND_FLOAT_CODE;
         break;
   
      default:
         printf("configurePackage: wrong data type!\nExiting...\n");
         exit(2);
   }

   return;
} // end of configurePackage

char verifyCrc(char *package, int pkgLength){
   short crc16       = calcula_CRC(package, pkgLength-2);
   short providedCrc = 0;
   memcpy(&providedCrc, &package[pkgLength-2], 2);

   if(crc16 != providedCrc){
      printf("verifyCrc: CRC error detected!\n");
      return 0;
   }

   return 1;
} // end of verifyCrc

/**
 * Reads a response from Rx
*/
void getResponse(){
   memset(responsePackage, 0x00, RES_PKG_LEN);
   sleep(2);   // waits a bit... get it? a bit lol

   globalResPkgLen = read(uartDescriptor, (void *) responsePackage, RES_PKG_LEN);

   if(globalResPkgLen < 0){
      printf("getResponse: read error\n");
      exit(3);
   } 
   else if(globalResPkgLen == 0){
      printf("getResponse: got no response\n");
      return;
   }
   else {
      char crcVerified = verifyCrc(responsePackage, globalResPkgLen);

      if(!crcVerified){
         printf("getResponse: provided CRC does not match\n");
         return;
      }

      printf("getResponse: received %d bytes\n", globalResPkgLen);
   }

   return;
} // end of getResponse

void sendString(char *message, int msgLength){
   int pkgLength = 4+msgLength+2;
   char *package = (char *) malloc(pkgLength * sizeof(char));

   configurePackage(package, 0, 0);
   package[3]  =  msgLength;

   strncpy(package+4, message, msgLength);   // without \0

   setCrc(package, pkgLength);

   writeToUart(package, pkgLength);

   free(package);

   return;
} // end of sendString

void getStringResponse(){
   getResponse();

   int messageLength = responsePackage[3];
   char *resMessage  = (char *) malloc(messageLength*sizeof(char));
   memcpy(resMessage, &responsePackage[4], messageLength);

   printf("getStringResponse: got string: %s\n", resMessage);

   // TO-DO: verificar cabecalho

   return;
} // end of getStringResponse

void sendInt(int number){
   char package[INT_PKG_LEN];
   configurePackage(package, 0, 1);

   memcpy(&package[3], &number, 4);

   setCrc(package, INT_PKG_LEN);

   writeToUart(package, INT_PKG_LEN);

   return;
} // end of sendInt

void getIntResponse(){
   getResponse();

   int number;
   memcpy(&number, &responsePackage[3], 4);
   printf("getIntResponse: number received is %d\n", number);

   // TO-DO: verificar cabecalho

   return;
} // end of getIntResponse

void sendFloat(float number){
   char package[FLOAT_PKG_LEN];
   configurePackage(package, 0, 2);

   memcpy(&package[3], &number, 4);

   setCrc(package, FLOAT_PKG_LEN);

   writeToUart(package, FLOAT_PKG_LEN);

   return;
} // end of sendFloat

void getFloatResponse(){
   getResponse();

   float number;
   memcpy(&number, &responsePackage[3], 4);
   printf("getFloatResponse: number received is %f\n", number);

   // TO-DO: verificar cabecalho

   return;
}