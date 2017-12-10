#pragma once

#include <iostream>
#include <memory>
#include <list>
#include <algorithm>
#include <stdexcept>
#include <thread>

#include <restclient-cpp/connection.h>
#include <restclient-cpp/restclient.h>

#include "device.hpp"

namespace hc_client {

using RestConnectionPtr = std::shared_ptr<RestClient::Connection>;
using DevicePtr = std::shared_ptr<Device>;

/**
 * @brief The Session class
 *
 * A session represents a connection to a Home Center. During connection the
 * user has to login to the data base and retrieve data on existing devices.
 * It is then possible to check and refresh status for each device.
 *
 * The Session object is non-copyable but is movable.
 */
class Session {
public:

  //
  // Constructors
  //
  Session() = delete;
  Session(const Session& rhs) = delete;
  Session(Session&& rhs) = default;

  Session(const std::string& ip_address, int timeout_seconds) {
    connection_ = RestConnectionPtr(new RestClient::Connection(ip_address));
    connection_->SetTimeout(timeout_seconds);
  }

  //
  // Session specific methods
  //
  /**
   * @brief Login to the Home Center service
   */
  void login() {
    std::cout << "Login: ";
    std::cin >> login_;

    std::cout << "Password: ";
    std::cin >> password_;

    connection_->SetBasicAuth(login_, password_);

    // TODO: This authentication check is way too expensive
    auto response = connection_->get("/api/devices");

    if (response.code != 200)
      throw std::runtime_error("Could not authenticate to the server.");

    std::cout << "Access granted" << std::endl;
  }

  /**
   * @brief Retrieve information about existing devices
   */
  void retrieveDevicesInfo() {
    auto response = connection_->get("/api/devices");

    rapidjson::Document document;
    document.Parse(response.body.c_str());

    if (!document.IsArray())
      throw std::logic_error("Could not parse JSON while retrieving devices");

    for (const auto& element : document.GetArray())
      if (element.HasMember("type")) {
        devices_.push_back(simpleDeviceFactory(element));
      }
  }

  /**
   * @brief Initialize states refreshing service
   * @return "last" parameter
   */
  int initializeRefresh() {
    auto response = connection_->get("/api/refreshStates");

    if (response.code != 200)
      throw std::runtime_error("Could not initialize refresh service.");

    rapidjson::Document document;
    document.Parse(response.body.c_str());

    if (!document.IsObject() || !document.HasMember("last"))
      throw std::runtime_error("Could not parse JSON while initializing refresh");

    int last = document["last"].GetInt();

    // TODO: Find out what about the rest of the items
    //std::string status = document["status"].GetString();
    //int64_t timestamp = document["timestamp"].GetInt64();
    //std::string date = document["date"].GetString();
    //std::vector<std::string> logs;
    //for (auto& log : document["logs"].GetArray())
    //  logs.push_back(log.GetString());

    return last;
  }

  /**
   * @brief Refreshes states of known devices
   * @param last is (...)
   */
  void refreshStates(int last) {
    auto response =
        connection_->get("/api/refreshStates?last=" + std::to_string(last));

    if (response.code != 200)
      throw std::runtime_error("Could not refresh devices.");

    rapidjson::Document document;
    document.Parse(response.body.c_str());
    if (!document.IsObject() || !document.HasMember("changes") ||
        !document["changes"].IsArray())
      throw std::runtime_error("Could not parse JSON while refreshing devices");

    for (auto& element : document["changes"].GetArray()) {
      int id = element["id"].GetInt();
      double value = element["value"].GetDouble();

      auto dev_it = std::find_if(devices_.begin(), devices_.end(),
                             [this, id] (auto& d) { return id == d->id(); });

      (*dev_it)->updateState(value);
    }
  }

  /**
   * @brief Print values of the devices
   */
  void printDevicesValues() const {
    for (auto dev_ptr : devices_) {
      switch (dev_ptr->type())
      {
      case DeviceType::TemperatureSensorType:
      {
        const TemperatureSensor *temp_sens =
            dynamic_cast<const TemperatureSensor *>(dev_ptr.get());

        if (temp_sens != nullptr)
          std::cout << "[" << temp_sens->id() << "]: " << temp_sens->name()
                    << " - " << temp_sens->temperature() << " "
                    << temp_sens->unit() << std::endl;
        break;
      }
      case DeviceType::UnknownDeviceType:
        break;
      }
    }
  }

  /**
   * @brief Devices list
   * @return a reference to the list of retrieved devices
   */
  const std::list<DevicePtr>& getDevices() const {
    return devices_;
  }

  //
  // Operator overloads
  //
  Session& operator=(const Session& rhs) = delete;
  Session& operator=(Session&& rhs) = default;

private:
  RestConnectionPtr connection_;      /**< Instance of REST connection */
  std::list<DevicePtr> devices_;   /**< List of all devices available */
  std::string address_;            /**< The address of the Home Center service */
  std::string login_;              /**< The login of the user */
  std::string password_;           /**< The password of the user */
};

} // end namespace hc_client
