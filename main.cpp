#include <iostream>
#include <string>
#include <algorithm>

#include <restclient-cpp/connection.h>

#include "include/device.hpp"
#include "include/session.hpp"

using namespace std;
using namespace rapidjson;
using namespace hc_client;

int main(int argc, char* argv[])
{
  string address;
  if (2 == argc)
    address = string(argv[1]);
  else {
    cout << "Usage: hc_client [address] \n\n"
            "  Example: hc_client styx.fibaro.com:7777 \n\n"
            "  If address is omitted, runs demo with address styx.fibaro.com:7777\n"
            << endl;

    address = string("http://styx.fibaro.com:7777");
  }

  RestClient::init();

  Session session(address);

  try {
    session.login();
    session.retrieveDevicesInfo();

    cout << "Temperature sensors:" << endl;
    for_each(session.getDevices().begin(),
             session.getDevices().end(),
             [](auto& device) {
      if (device.type() == DeviceType::TemperatureSensor) cout << device << endl;
    });

    session.updateDevicesStates();
  }
  catch (const exception& e) {
    cerr << e.what() << "\nExitting." << endl;

    RestClient::disable();
    return 1;
  }

  RestClient::disable();
}

