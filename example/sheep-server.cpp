#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <fstream>
#include <random>
#include <sys/time.h>

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

SheepServer::SheepServer(utility::string_t url) : m_listener(url)
{
    m_listener.support(methods::GET, std::bind(&SheepServer::handle_get, this, std::placeholders::_1));
    m_listener.support(methods::PUT, std::bind(&SheepServer::handle_put, this, std::placeholders::_1));
    m_listener.support(methods::POST, std::bind(&SheepServer::handle_post, this, std::placeholders::_1));
    m_listener.support(methods::DEL, std::bind(&SheepServer::handle_delete, this, std::placeholders::_1));
   
}

void SheepServer::handle_get(http_request message)
{
    ucout <<  message.to_string() << endl;
    message.reply(status_codes::OK);
};

void SheepServer::handle_post(http_request message)
{
    ucout <<  message.to_string() << endl;
	message.reply(status_codes::OK);
};

void SheepServer::handle_delete(http_request message)
{
	ucout << message.to_string() << endl;
	message.reply(status_codes::OK);
}

void SheepServer::handle_put(http_request message)
{
	ucout << message.to_string() << endl;

	auto paths = uri::split_path(uri::decode(message.relative_uri().path()));
	auto query = uri::split_query(uri::decode(message.relative_uri().query()));
	auto queryItr = query.find(U("request"));
	utility::string_t request = queryItr->second;
	ucout << request << endl;

	if (request == U("leave"))
	{
		Data data;
		data.job = U("Devs");
		People p1;
		p1.age = 10;
		p1.name = U("Franck");
		data.peoples.push_back(p1);
		People p2;
		p2.age = 20;
		p2.name = U("Joe");
		data.peoples.push_back(p2);

		utility::string_t response = data.AsJSON().serialize();
		ucout << response << endl;

		message.reply(status_codes::OK, data.AsJSON());
		return;
	}

	message.reply(status_codes::OK);
};

std::unique_ptr<SheepServer> g_http;

void on_initialize(const string_t& address)
{
    // Build our listener's URI from the configured address and the hard-coded path "SheepServer/Action"

    uri_builder uri(address);
    uri.append_path(U("SheepServer/Action/"));

    auto addr = uri.to_uri().to_string();
	g_http = std::unique_ptr<SheepServer>(new SheepServer(addr));
	g_http->open().wait();
    
    ucout << utility::string_t(U("Listening for requests at: ")) << addr << std::endl;

    return;
}

void on_shutdown()
{
	g_http->close().wait();
    return;
}

int main(int argc, const char** argv)
{
    utility::string_t port = U("34568");
    if(argc == 2)
    {
        port = argv[1];
    }

    utility::string_t address = U("http://localhost:");
    address.append(port);

    on_initialize(address);
    std::cout << "Press ENTER to exit." << std::endl;

    std::string line;
    std::getline(std::cin, line);

    on_shutdown();
    return 0;
}
