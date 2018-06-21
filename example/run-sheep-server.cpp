#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <fstream>
#include <random>
#include <sys/time.h>

#define _TURN_OFF_PLATFORM_STRING

#include "cpprest/json.h"
#include "cpprest/http_listener.h"
#include "cpprest/uri.h"
#include "cpprest/asyncrt_utils.h"
#include "sheep-server.hpp"

using namespace std;
using namespace web;
using namespace utility;
using namespace http;
using namespace web::http::experimental::listener;

std::unique_ptr<SheepServer> g_http;

void on_initialize(const string_t& address)
{
    // Build our listener's URI from the configured address and the hard-coded path "SheepServer/"

    uri_builder uri(address);
    uri.append_path("SheepServer/");

    auto addr = uri.to_uri().to_string();
	g_http = std::unique_ptr<SheepServer>(new SheepServer(addr));
	g_http->open().wait();

    cout << utility::string_t("Listening for requests at: ") << addr << std::endl;

    return;
}

void on_shutdown()
{
	g_http->close().wait();
    return;
}

int main(int argc, const char** argv)
{
    utility::string_t port = ("34568");
    if(argc == 2)
    {
        port = argv[1];
    }

    utility::string_t address = ("http://0.0.0.0:");
    address.append(port);

    on_initialize(address);

    std::cout << "Press Ctrl+C to exit." << std::endl;

    while(true) {
    }
    on_shutdown();
    return 0;
}
