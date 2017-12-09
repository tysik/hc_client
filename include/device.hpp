#pragma once

#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>

#include <iostream>
#include <memory>
#include <vector>
#include <stdexcept>

namespace hc_client {

/**
 * @brief The DeviceType enum
 *
 * Enumerates all known types of devices.
 */
enum class DeviceType { TemperatureSensor, UnknownDevice };

/**
 * @brief The Device class
 *
 * A base class for the devices.
 */
class Device {
public:

  //
  // Constructors
  //
  Device() = delete;
  Device(const Device& rhs) = default;
  Device(Device&& rhs) = default;

  /**
   * @brief Construction of device from a JSON element
   *
   * @param element is a parsed JSON element
   */
  Device(const rapidjson::Value& element) {
     if (!element.IsObject())
      throw std::logic_error("Provided JSON element is not a valid device");

     if (element.HasMember("id"))
       id_ = element["id"].GetInt();

     if (element.HasMember("name"))
       name_ = element["name"].GetString();

     if (element.HasMember("enabled"))
       enabled_ = element["enabled"].GetBool();

     type_ = DeviceType::UnknownDevice;
  }

  //
  // Getter methods
  //
  DeviceType type() const {
    return type_;
  }

  std::string name() const {
    return name_;
  }

  int id() const {
    return id_;
  }

  bool enabled() const {
    return enabled_;
  }

  //
  // Operator overloads
  //
  Device& operator=(const Device& rhs) = default;
  Device& operator=(Device&& rhs) = default;

  friend std::ostream &operator<<(std::ostream& out, const Device& device) {
    out << "Device: " << device.id_ << " : " << device.name_ << " is " <<
           (device.enabled_ ? "enabled" : "disabled") << ".";

    return out;
  }

protected:

  DeviceType type_;     /**< Type of the device */
  std::string name_;    /**< Literal name of the device */
  int id_;              /**< Numerical id of the device */
  bool enabled_;        /**< Whether the device is enabled or not */
};


/**
 * @brief The TemperatureSensor class
 */
class TemperatureSensor : public Device {
public:

  //
  // Constructors
  //
  TemperatureSensor() = delete;
  TemperatureSensor(const TemperatureSensor& rhs) = default;
  TemperatureSensor(TemperatureSensor&& rhs) = default;


  /**
   * @brief Construction of temperature sensor from a JSON element
   *
   * @param element is a parsed JSON element
   */
  TemperatureSensor(const rapidjson::Value& element) :
    Device(element)
  {
    if (!element.IsObject())
     throw std::logic_error("Provided JSON element is not a valid device");

    if (element.HasMember("value"))
      temperature_ = element["id"].GetDouble();

    if (element.HasMember("unit"))
      unit_ = element["unit"].GetString();

    type_ = DeviceType::TemperatureSensor;
  }

  std::string unit() const {
    return unit_;
  }

  double temperature() const {
    return temperature();
  }

  //
  // Operator overloads
  //
  TemperatureSensor& operator=(const TemperatureSensor& rhs) = default;
  TemperatureSensor& operator=(TemperatureSensor&& rhs) = default;

private:
  double temperature_;  /**< Recent value of temperature */
  std::string unit_;    /**< Physical unit of the temperature */
};

/**
 * @brief Simple Device Factory
 *
 * Creates instances of Devices based on retrieved REST object.
 *
 * @param connection is forwarded to the device to allow updates
 * @param object is an JSON object from which the device will be initialized
 *
 * @return instance of appropriate device
 */
Device simpleDeviceFactory(const rapidjson::Value& object) {
  if (std::string("com.fibaro.temperatureSensor") == object["type"].GetString())
    return TemperatureSensor(object);
  else
    return Device(object);
}

} // end namespace hc_client
