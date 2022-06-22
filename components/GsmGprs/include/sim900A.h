
#ifndef _SIM900A_H
#define _SIM900A_H

#define ESP_INTR_FLAG_DEFAULT 0


void sim900A__HardwareInit();
void sim900A__SetPowerPin(void);
void sim900A__ClearPowerPin(void);
void sim900A__ModemBootsup(void);
void sim900__ConfigureRingInterrupt(void);

#endif