#ifndef MqttManager_h
#define MqttManager_h

#include <ButtonManager.h>
#include <SystemManager.h>
#include "TCMenu_menu.h"

class MqttManager_
{
    private:
        MqttManager_() = default;
     
    public:
        static MqttManager_ &getInstance();

        void setup();
        void tick();
        void publish(const char *topic,const char *payload);
};

extern MqttManager_ &MqttManager;

#endif