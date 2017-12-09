#pragma once

#include <restclient-cpp/connection.h>
#include <restclient-cpp/restclient.h>

#include <iostream>
#include <memory>
#include <list>
#include <algorithm>
#include <stdexcept>

#include "device.hpp"

namespace hc_client {

using RestConnection = std::shared_ptr<RestClient::Connection>;

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

  Session(const std::string& ip_address) {
    connection_ = RestConnection(new RestClient::Connection(ip_address));
    connection_->SetTimeout(10);
  }

  //
  // Session specific methods
  //
  /**
   * @brief Login to Home Center and retrieve data about devices
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
      if (element.HasMember("type"))
        devices_.push_back(simpleDeviceFactory(element));

    std::cout << "Found " << devices_.size() << " " <<
                 (1 == devices_.size() ? "device." : "devices.") << std::endl;
  }

  /**
   * @brief Update devices information
   */
  void updateDevicesStates() {
    auto response = connection_->get("/api/refreshStates");

    rapidjson::Document document;
    document.Parse(response.body.c_str());

    if (!document.IsObject())
      throw std::runtime_error("Could not parse JSON while updating devices");

    std::string status = document["status"].GetString();
    int last = document["last"].GetInt();
    int64_t timestamp = document["timestamp"].GetInt64();
    std::string date = document["date"].GetString();

    std::vector<std::string> logs;
    for (auto& log : document["logs"].GetArray())
      logs.push_back(log.GetString());

//    std::cout << status << "\n" <<
//                 last << "\n" <<
//                 timestamp << "\n" <<
//                 date << std::endl;

//    for (auto& log : logs)
//      std::cout << log << std::endl;

    std::cout << "/api/refreshStates?last=" + std::to_string(last) << std::endl;

    response =
        connection_->get("/api/refreshStates?last=" + std::to_string(last));

    std::cout << "Code: " << response.code << std::endl;
    std::cout << "Body: " << response.body << std::endl;
  }

//  /**
//   * @brief Update devices
//   *
//   * Refresh information for all of devices in the list
//   */
//  void refreshAllDevices() {
//    for (auto& device : devices_)
//      device.checkForUpdates();
//  }

//  /**
//   * @brief Refresh information about a single device
//   *
//   * @param id is the numerical identificator of the device to be refreshed
//   */
//  void refreshDevice(int id) {
//    auto dev_it =
//        std::find_if(devices_.begin(), devices_.end(),
//                     [this, id](const auto& d) { return d.id() == id; });

//    dev_it->checkForUpdates();
//  }

  //
  // Getter methods
  //
  /**
   * @brief Get devices list
   * @return a reference to the list of retrieved devices
   */
  const std::list<Device>& getDevices() const {
    return devices_;
  }

  //
  // Operator overloads
  //
  Session& operator=(const Session& rhs) = delete;

  Session& operator=(Session&& rhs) = default;

private:
  RestConnection connection_;   /**< Instance of REST connection */
  std::list<Device> devices_;   /**< List of all devices available */
  std::string address_;         /**< The address of the Home Center service */
  std::string login_;           /**< The login of the user */
  std::string password_;        /**< The password of the user */
};

} // end namespace hc_client
