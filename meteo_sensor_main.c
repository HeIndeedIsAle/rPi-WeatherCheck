/*
=========================================
   Filename:	meteo-sensor-main.c
   Author:	Aleksey C.
   Version:	3.0
   Last edit:	25/02/2025
      Description:
Read temperature and Humidity data
from SHT21 sensor and save it in raw
then report data in real numbers.
Sampling and Showing rate are passed
through command line.
==========================================
*/

#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

int fd;					//made it global so i can use it in every function
uint8_t elements = 0;			//made it global so i can use it in every function

//Data structure named samples
struct samples {
	uint32_t timestamp;
	uint16_t rawTempValue;
	uint16_t rawHumidityValue;
	uint16_t rawRg;			//this was made to use as an example.
};

float rawHumToData(uint16_t rawHum) {
	return(-6.0 + (125.0 * (rawHum / 65536.0f)));	//got an error when using 2^16 so converted it to 65536.0
}

float rawTempToData(uint16_t rawTemp) {
	return(-46.85 + (172.72 * (rawTemp / 65536.0f)));
}

void writeValues(time_t callTime, struct samples *itemPtr) {

	uint16_t rawTemp = 0;
	uint16_t rawHm = 0;

	//save temperature Raw data
	wiringPiI2CWrite(fd,0xE3);                              // Command to read T
       	rawTemp = wiringPiI2CRead(fd) << 8;                     // Read MSB
       	rawTemp |= wiringPiI2CRead(fd) & 0x00FF;                // Read LSB

	//save humidity Raw data
       	wiringPiI2CWrite(fd, 0xE5);                             // Command to read HM
	rawHm = wiringPiI2CRead(fd) << 8;                       // Read MSB
	rawHm |= wiringPiI2CRead(fd) & 0x00FF;                  // Read LSB

	//append data to pointed data structure
	itemPtr->timestamp = callTime;
	itemPtr->rawTempValue = rawTemp;
	itemPtr->rawHumidityValue = rawHm;
	itemPtr->rawRg = 10;
}

void readValues(struct samples *itemPtr) {

	printf("\n\n****************************\n   Riepilogo campionamenti  \n*****************************");

	float hmMean = 0;
	float tempMean = 0;

	float realHm = 0;
	float realTemp = 0;

	//converting from Unix Time to normal date
	char dateBuffer[80]; //buffer to be able to convert

		for(uint8_t i = 0; i < elements; i++) {

			time_t epoch_time = (time_t)itemPtr->timestamp;
			struct tm *local_time = localtime(&epoch_time);
			strftime(dateBuffer, sizeof(dateBuffer), "%d/%m/%Y - %H:%M:%S", local_time); //converting to string in giorno-mese-anno/ore-minuti-secondi

			realTemp = rawTempToData(itemPtr->rawTempValue);
			tempMean += realTemp;

			realHm = rawHumToData(itemPtr->rawHumidityValue);
			hmMean += realHm;

			printf("\n===[Campionamento: %d]===\nTimestamp: %s", i + 1, dateBuffer);
			printf("\nTemperatura registrata: %.2f°C", realTemp);
			printf("\nUmidità relativa registrata: %.2f%%", realHm);
			printf("\nESEMPIO: Acqua piovuta: %d (non reale)", itemPtr->rawRg);
			fflush(stdout);		//print out any buffered text
			itemPtr++;
		}
	printf("\n\nTemperatura media: %.2f°C", tempMean / elements);
	printf("\nUmidità relativa media: %.2f%%", hmMean / elements);
	fflush(stdout);
}

int main (int argc, char* argv[]) {

	if (argc != 3) {
		printf("\nERRORE! | Non sono stati correttamente inseriti i parametri\n");
		return(0);
	}

uint16_t samplingRate = atoi(argv[1]);
uint16_t readingRate = atoi(argv[2]);

	if(samplingRate > readingRate) {
		printf("\nERRORE! | I valori inseriti non sono nell'ordine corretto\n");
		return(0);
	}

wiringPiSetup();
fd = wiringPiI2CSetup(0x40);

	if (fd == -1) {
        	printf ("\nFailed to init I2C communication.\n");
        	return -1;
    	}


elements = readingRate / samplingRate;		//global

struct samples *samplesMainPtr;
samplesMainPtr = malloc(sizeof(*samplesMainPtr) * elements);
struct samples *samplesStartPtr;
samplesStartPtr = samplesMainPtr;		//i have a pointer that is static to the start of the structure
	
time_t samplingStartingTime;
samplingStartingTime = time(NULL);
uint8_t doubleCheck = 0;

	while(true) {

	time_t currentTime = time(NULL);	//Instead of checking time in each IF statement,
						//i just set it for each while iteration

		if(currentTime >= (samplingStartingTime + samplingRate)) {
			writeValues(currentTime, samplesMainPtr);
			samplingStartingTime = time(NULL);
			samplesMainPtr++;
			doubleCheck++;
		}

		if(doubleCheck == elements) {
			samplesMainPtr = samplesStartPtr;	//replacing the pointer to the start of the
			readValues(samplesMainPtr);		//structure so i can read values
			doubleCheck = 0;
		}

	sleep(1);	//1s pause to limit CPU usage
        }

free(samplesStartPtr);
return(0);
}
