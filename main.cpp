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
         << (1 == session.getDevices().size() ? "device." : "devices.")
         << endl;

    cout << "Temperature sensors: " << endl;
    session.printDevicesValues();
  }
  catch (const exception& e) {
    cerr << e.what() << "\nExitting." << endl;

    RestClient::disable();
    return 1;
  }

  thread background_thread([&session]() {
    blocker.lock();
    cout << "Type \"c\" to start automatic data update or anything else to stop" << endl;
    char c;
    cin >> c;
    if (c != 'c') {
      RestClient::disable();
      terminate();
    }

    cout << "Initializing data update (press CTRL+C to exit)" << endl;
    int last = session.initializeRefresh();
    cout << "Awaiting response from the server" << endl;

    blocker.unlock();

    while (true) {
      try {
        session.refreshStates(last);
      }
      catch (const exception& e) {
        cout << e.what() << "\n";
        cout << "Did not receive status for 30 seconds. Retrying." << endl;
        continue;
      }

      blocker.lock();
      session.printDevicesValues();
      blocker.unlock();

      this_thread::sleep_for(1s);
    }
  });

  // Do other things

  background_thread.join();
  RestClient::disable();
}

