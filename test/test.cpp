#include <memory>

#include <gtest/gtest.h>
#include <rapidjson/rapidjson.h>

#include "../include/device.hpp"

using namespace rapidjson;
using namespace hc_client;

const char* json_object = "{\"id\":32167,\"name\":\"AwesomeDevice\",\"roomID\":35,\"type\":\"com.fibaro.temperatureSensor\",\"baseType\":\"com.fibaro.multilevelSensor\",\"enabled\":true,\"visible\":true,\"isPlugin\":true,\"parentId\":1975,\"remoteGatewayId\":0,\"viewXml\":false,\"configXml\":false,\"interfaces\":[],\"properties\":{\"dead\":\"false\",\"deviceControlType\":\"0\",\"deviceIcon\":\"30\",\"emailNotificationID\":\"0\",\"emailNotificationType\":\"0\",\"liliOffCommand\":\"\",\"liliOnCommand\":\"\",\"log\":\"\",\"logTemp\":\"\",\"manufacturer\":\"\",\"model\":\"\",\"offset\":\"0.00\",\"pushNotificationID\":\"0\",\"pushNotificationType\":\"0\",\"remoteGatewayId\":\"0\",\"saveLogs\":\"true\",\"showFireAlarm\":\"true\",\"showFreezeAlarm\":\"false\",\"smsNotificationID\":\"0\",\"smsNotificationType\":\"0\",\"unit\":\"C\",\"userDescription\":\"\",\"value\":\"22.93\"},\"actions\":{},\"created\":1511189425,\"modified\":1511189425,\"sortOrder\":6}";

TEST (DeviceConstruction, Name) {
  Document document;
  document.Parse(json_object);

  Device d(document);

  EXPECT_EQ("AwesomeDevice", d.name());
}

TEST (DeviceConstruction, Id) {
  Document document;
  document.Parse(json_object);

  Device d(document);

  EXPECT_EQ(32167, d.id());
}

TEST (DeviceConstruction, Type) {
  Document document;
  document.Parse(json_object);

  auto device = simpleDeviceFactory(document);

  EXPECT_EQ(typeid(TemperatureSensor), typeid(*(device.get())));
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
