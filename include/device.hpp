#pragma once

#include <iostream>
#include <memory>
#include <numeric>
#include <stdexcept>

#include <boost/circular_buffer.hpp>

#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>

namespace hc_client {

/**
 * @brief The DeviceType enum
 * Enumerates all known types of devices.
 */
enum class DeviceType { UnknownDeviceType = 0, TemperatureSensorType };

/**
 * @brief The Device class
 * A base class for the devices
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
   * @brief Construction of a device from a JSON element
   * @param element is a parsed JSON object
   */
  Device(const rapidjson::Value& element) {
     if (!element.IsObject())
      throw std::logic_error("Provided JSON element is not a valid object");

     if (element.HasMember("id") && element["id"].IsNumber())
       id_ = element["id"].GetInt();
     else
       id_ = -1;

     if (element.HasMember("name") && element["name"].IsString())
       name_ = element["name"].GetString();
     else
       name_ = std::string("name_error");

     if (element.HasMember("enabled") && element["enabled"].IsBool())
       enabled_ = element["enabled"].GetBool();
     else
       enabled_ = false;

     type_ = DeviceType::UnknownDeviceType;
  }

  //
  // Device specific methods
  //
  /**
   * @brief Update state of the device
   */
  virtual void updateState(double) {
    std::cout << "The generic device state cannot be updated" << std::endl;
  }

  /**
   * @brief Type
   * @return the type of the device
   */
  DeviceType type() const {
    return type_;
  }

  /**
   * @brief Name
   * @return the name of the device
   */
  std::string name() const {
    return name_;
  }

  /**
   * @brief Id
   * @return the numerical identificator of the device
   */
  int id() const {
    return id_;
  }

  /**
   * @brief Enabled
   * @return true if the device is enabled
   */
  bool enabled() const {
    return enabled_;
  }

  //
  // Operator overloads
  //
  Device& operator=(const Device& rhs) = default;
  Device& operator=(Device&& rhs) = default;

  friend std::ostream &operator<<(std::ostream& out, const Device& device) {
    out << "[" << device.id_ << "]: " << device.name_ << " is " <<
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
   * @brief Construction of a temperature sensor from a JSON element
   * @param element is a parsed JSON element
   */
  TemperatureSensor(const rapidjson::Value& element) :
    Device(element),
    recent_temperatures_(MAX_BUFFER_SIZE)
  {
    if (!element.IsObject())
     throw std::logic_error("Provided JSON element is not a valid device");

    if (element.HasMember("properties") && element["properties"].IsObject()) {
      const auto& properties = element["properties"].GetObject();

      // TODO: Check why double values are recognized as strings
      if (properties.HasMember("value") && properties["value"].IsString())
        recent_temperatures_.push_back(std::stod(properties["value"].GetString()));
      else if (properties.HasMember("value") && properties["value"].IsDouble())
        recent_temperatures_.push_back(properties["value"].GetDouble());

      if (properties.HasMember("unit") && properties["unit"].IsString())
        unit_ = properties["unit"].GetString();
      else
        unit_ = std::string("unit_error");
    }

    type_ = DeviceType::TemperatureSensorType;
  }

  //
  // TemperatureSensor specific methods
  //
  /**
   * @brief Update temperature sensor with given temperature
   * @param most recent temperature (note that units are not checked)
   */
  virtual void updateState(double temperature) {
    recent_temperatures_.push_back(temperature);
  }

  /**
   * @brief Unit
   * @return the physical unit of temperature values
   */
  std::string unit() const {
    return unit_;
  }

  /**
   * @brief Temperature
   * @return most recent temperature reading or NaN if not available
   */
  double temperature() const {
    if (recent_temperatures_.empty())
      return NAN;
    else
      return recent_temperatures_.back();
  }

  /**
   * @brief Average Temperature
   * @return Mean value of recently registered temperatures or NaN if not available
   */
  double averageTemperature() const {
    if (recent_temperatures_.empty())
      return NAN;

    return std::accumulate(recent_temperatures_.begin(),
                           recent_temperatures_.end(), 0.0) /
                               recent_temperatures_.size();
  }

  //
  // Operator overloads
  //
  TemperatureSensor& operator=(const TemperatureSensor& rhs) = default;
  TemperatureSensor& operator=(TemperatureSensor&& rhs) = default;

private:

  /**
   * @brief Size of circular buffer for storing most recent temperature values
   */
  static const size_t MAX_BUFFER_SIZE = 24;

  /**
   * @brief Buffer for storing most recent temperature values
   */
  boost::circular_buffer<double> recent_temperatures_;

  std::string unit_;    /**< Physical unit of temperature values */
};


using DevicePtr = std::shared_ptr<Device>;

/**
 * @brief Simple device factory
 * Creates instances of Devices based on retrieved JSON object.
 * @param object is an JSON object from which the device will be initialized
 * @return instance of appropriate device
 */
DevicePtr simpleDeviceFactory(const rapidjson::Value& object) {
  if (!object.HasMember("type") || !object["type"].IsString())
    throw std::logic_error("Could not instantiate device from object without type");

  if (std::string("com.fibaro.temperatureSensor") == object["type"].GetString())
    return DevicePtr(new TemperatureSensor(object));
  else
    return DevicePtr(new Device(object));
}

} // end namespace hc_client
