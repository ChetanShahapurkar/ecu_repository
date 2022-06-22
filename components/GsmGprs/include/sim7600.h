// Copyright 2015-2018 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_modem_dce_service.h"
#include "esp_modem.h"

/**
 * @brief Create and initialize SIM7600 object
 *
 * @param dte Modem DTE object
 * @return modem_dce_t* Modem DCE object
 */

#define ESP_INTR_FLAG_DEFAULT 0




modem_dce_t *sim7600_init(modem_dte_t *dte);
void sim7600__HardwareInit(void);
void sim7600__ModemBootsup(void);
void sim7600__Power_On(void);
void sim7600__SetPowerPin(void);
void sim7600__ClearPowerPin(void);
void sim7600__ConnectMQTT(void);
bool sim7600__GetMqttConnectStatus(void);
bool sim7600__GetMqttSubscribeStatus(void);
void sim7600__SendMqttMsg(char * _json_message);
bool sim7600__GetMqttMsgSendStatus();
void sim7600__SetMqttMsgSendStatus(bool status);
void sim7600__EnableRingInterrupt(void);
void sim7600__DisableRingInterrupt(void);
void sim7600__StopPPP(void);

bool sim7600__GetNewSmsReceivedStatus(void);
void sim7600__SetNewSmsReceivedStatus(bool status);
uint32_t sim7600__GetPPPState(void);
esp_err_t sim7600__DeleteAllSms(void);
void sim7600__Setreconnectbit(bool setb);

void sim7600__ReConnectMQTT(void );
bool sim7600__GetPPPStopStatus(void);

void sim7600__GetImei(char **buff);
void sim7600__GetOperator(char **buff);
void *sim7600__GetOperatorNew(void);
void sim7600__GetImsi(char **buff);
uint32_t sim7600__GetRssi(void);

void sim7600__mqttConnectToQueue(char * new_queue);

void sim7600__NetworkCheck(void);

 



 typedef enum LAST_CMD
{
    CMD__IDLE    = 0,
    CMD__NULL    = 1,
    CMD__SCFG    = 2,
    CMD__SOTP    = 3,
} LST_CMD_TYPE;

LST_CMD_TYPE sim7600__GetLastSmsCmd(void);
 void sim7600__SetLastSmsCmd(LST_CMD_TYPE cmd);

  bool sim7600__Getinterruptdisable_status(void);
   bool sim7600__GetSystemBootStatus(void);
   void sim7600__SetSystemBootStatus(bool status);
   esp_err_t sim7600__NetTimeSetup(void);


#ifdef __cplusplus
}
#endif
