// void sd_main(void);

void SdCard__RenameFile(const char *oldname, const char* newname);

void SDCard__WriteToFile(char *file_name , char *dt);

// void SdCard__InitFTP(void);

void SdCard_main(void);
void SDCardManager__Handler(void);
void SDCardManager__Init(void);
void SdCard__DeleteFile(const char *filename);
void SDCard__ReadFile(const char *file_name);
void SDCard__WriteToOTPFile(char *file_name ,char *dt);
void SDCard__WriteToConfigFile(char *file_name ,char *dt);
bool SdCard__GetConfigUpdateStatus();
void SdCard__SetConfigUpdateStatus(bool status);
void SDCard__WriteToSDFile_Telemetry(char* folder_name,char *file_name ,char *dt);
void SDCard__WriteToSDFile_Diagnostics(char* folder_name,char *file_name ,char *dt);
void SDCard__WriteToSDFile_Events(char* folder_name,char *file_name ,char *dt);
void ftp__SetConfigurationFileUopdatedStatus(bool status);
bool ftp__GetConfigurationFileUopdatedStatus(void);
char *SdCard__GetFileContent();
char *SdCard__ReadLine(char *file_name);


void SdCard__CheckRenameFile();