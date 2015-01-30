#ifndef AAM_CONFIG_H
#define AAM_CONFIG_H
/**
 * 0: Normal
 * 1: Cannot Read Uploaded Image
 * 2: Not Find Face
 * 3: SystemBusy
 * 4: Age Input Error
 * 5:
 */
 static int processState = 0;
 static char* resultPath = NULL;

 class AgingException {
 public:
    AgingException() {
        stateCode = 0;
        messageSet[0] = "Normal";
        messageSet[1] = "Cannot Read Uploaded Image";
        messageSet[2] = "Not Find Face";
        messageSet[3] = "System Busy";
        messageSet[4] = "Age Input Error";
        messageSet[5] = "";
        messageSet[6] = "";
        messageSet[7] = "";
        messageSet[8] = "";
        messageSet[9] = "";
        messageSet[10] = "";
        messageSet[11] = "";
    }

    AgingException(int stateCode) {
        this->stateCode = stateCode;
        messageSet[0] = "Normal";
        messageSet[1] = "Cannot Read Uploaded Image";
        messageSet[2] = "Not Find Face";
        messageSet[3] = "System Busy";
        messageSet[4] = "Age Input Error";
        messageSet[5] = "";
        messageSet[6] = "";
        messageSet[7] = "";
        messageSet[8] = "";
        messageSet[9] = "";
        messageSet[10] = "";
        messageSet[11] = "";
    }

    void setStateCode(int stateCode) {
        this->stateCode = stateCode;
    }

    int getStateCode() {
        return stateCode;
    }

    char* getMessage() {
        return messageSet[stateCode];
    }

    char* getMessage(int code) {
        return messageSet[code];
    }
 private:
    int stateCode;
    char* messageSet[12];
 };
#endif
