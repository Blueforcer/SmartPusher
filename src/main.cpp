
/*
 * Copyright (c) 2022 https://www.blueforcer.de (Stephan Muehl).
 * This product is licensed under an Apache license.
 */

#include <Arduino.h>
#include <PubSubClient.h>
#include <improv.h>
#include <SystemManager.h>
#include <ButtonManager.h>
#include <MqttManager.h>






void setup()
{
  delay(1000);
  Serial.begin(115200);
  SystemManager.setup();
  
  ButtonManager.setup();
  MqttManager.setup();
}

void loop()
{
 
  ButtonManager.tick();
  SystemManager.tick();
  MqttManager.tick();
}