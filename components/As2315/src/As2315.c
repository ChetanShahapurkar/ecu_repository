
#include "As2315.h"

// C program to generate random numbers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<time.h>

char *As2315__GetTemperature(void)
{
    return "31.6";
}

char *As2315__GetHumidity(void)
{
    srand((unsigned int)time(NULL));

    float a = 5.0;
    float numm = ((float)rand()/(float)(RAND_MAX)) * a;
    numm = 33.0 + numm;
    
    // myPrintf(numm);

    // uint8_t number;
    char string[20];
    sprintf(string, "%f", numm);
    return string;

}

char result[100];

void myPrintf(float fVal)
{
    
    int dVal, dec, i;

    fVal += 0.005;   // added after a comment from Matt McNabb, see below.

    dVal = fVal;
    dec = (int)(fVal * 100) % 100;

    memset(result, 0, 100);
    result[0] = (dec % 10) + '0';
    result[1] = (dec / 10) + '0';
    result[2] = '.';

    i = 3;
    while (dVal > 0)
    {
        result[i] = (dVal % 10) + '0';
        dVal /= 10;
        i++;
    }

    for (i=strlen(result)-1; i>=0; i--)
        putc(result[i], stdout);
}



float As2315__GetHumidityNew( void)
{
    float a = 2.0;
    float def_temp = 41.0;
    float x = (float)rand()/(float)(RAND_MAX/a);
    def_temp = def_temp + x;
    printf("Humidity  : %f\n", def_temp);

    return def_temp;
    
}

float As2315__GetTemperatureNew( void)
{
    float a = 2.0;
    float def_temp = 36.0;
    float x = (float)rand()/(float)(RAND_MAX/a);
    def_temp = def_temp + x;
    printf("Temperature   : %f\n", def_temp);

    return def_temp;
    
}




 
// Driver program
int main(void)
{
    // This program will create different sequence of
    // random numbers on every program run
 
    // Use current time as seed for random generator
    
}