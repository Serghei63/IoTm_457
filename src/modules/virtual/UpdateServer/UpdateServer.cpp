#include "Global.h"
#include "classes/IoTItem.h"

class UpdateServer : public IoTItem {
   public:
    UpdateServer(String parameters) : IoTItem(parameters) {}

    void onModuleOrder(String &key, String &value) {
        if (key == "startUpdate") {
            upgrade_firmware(3, value);
        }
    }

    ~UpdateServer() {};
};

void* getAPI_UpdateServer(String subtype, String param) {
    if (subtype == F("UpdateServer")) {
        return new UpdateServer(param);
    } else {
        return nullptr;
    }
}
