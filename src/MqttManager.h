#ifndef MqttManager_h
#define MqttManager_h

#include <ButtonManager.h>
#include <SystemManager.h>

class MqttManager_
{
    private:
        MqttManager_() = default;
     
    public:
        static MqttManager_ &getInstance();

        void setup();
        void tick();
        void publish(const char *topic,const char *payload);
        void HAState(uint8_t btn, const char *state);
};

extern MqttManager_ &MqttManager;

#endif