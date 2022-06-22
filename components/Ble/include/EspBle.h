
void EspBle__Init(void);
void EspBle__Deinit(void);
void EspBle__PublishMsg(void);
char * EspBle__GetNewBleMsg(void);
void EspBle__Clearbuffers(void);
bool EspBle__NewMsgReceived(char **msg);

void EspBle__SetDeviceName(char * dev_name);


void EspBle__Enable(void);
void EspBle__Disable(void);