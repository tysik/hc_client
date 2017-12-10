#include <iostream>
#include <string>
#include <algorithm>
#include <thread>
#include <mutex>
#include <chrono>

#include <restclient-cpp/connection.h>

#include "include/device.hpp"
#include "include/session.hpp"

using namespace std;
using namespace rapidjson;
using namespace hc_client;

mutex blocker;

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
  int timeout = 30;
  Session session(address, timeout);

  try {
    session.login();

    cout << "Retrieving devices information" << endl;
    session.retrieveDevicesInfo();

    cout << "Found " << session.getDevices().size() << " "
         << (1 == session.getDevices().size() ? "device." : "devices. ")
         << "Would you like to list them? (y/n)" << endl;

    char input;
    cin >> input;
    if ('y' == input)
      session.printDevicesInfo();

    cout << "\nFound following temperature sensors: " << endl;
    session.printDevicesValues();
  }
  catch (const exception& e) {
    cerr << e.what() << "\nExitting." << endl;

    RestClient::disable();
    return 1;
  }

  cout << "Would you like to start refreshing the devices? (y/n)" << endl;

  char input;
  cin >> input;
  if ('y' == input)
    session.run();
  else {
    cout << "Retrieving a single update" << endl;
    int last = session.initializeRefresh();
    session.refreshStates(last);
  }

  // Do other things

  session.join();
  RestClient::disable();
}

