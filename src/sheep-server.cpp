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
    m_job_config = {};  /// zero all fields
    m_listener.support(methods::GET, std::bind(&SheepServer::handle_get, this, std::placeholders::_1));
    m_listener.support(methods::PUT, std::bind(&SheepServer::handle_put, this, std::placeholders::_1));
    m_listener.support(methods::POST, std::bind(&SheepServer::handle_post, this, std::placeholders::_1));
   
}

template <typename PlaintextT>
std::unique_ptr<BaseContext<PlaintextT> >
SheepServer::make_context(std::string context_type, std::string context_params="") {
  if (context_type == "HElib_F2") {
    auto ctx =  std::make_unique<ContextHElib_F2<PlaintextT> >();
    if (context_params.length() > 0)
      ctx->read_params_from_file(context_params);
    return ctx;
  } else if (context_type == "HElib_Fp") {
    auto ctx =  std::make_unique<ContextHElib_Fp<PlaintextT> >();
    if (context_params.length() > 0)
      ctx->read_params_from_file(context_params);
    return ctx;
  } else if (context_type == "TFHE") {
    auto ctx =  std::make_unique<ContextTFHE<PlaintextT> >();
    if (context_params.length() > 0)
      ctx->read_params_from_file(context_params);
    return ctx;
    //	} else if (context_type == "SEAL") {
    //auto ctx =  std::make_unique<ContextSeal<PlaintextT> >();
    // if (context_params.length() > 0)
    //  ctx->read_params_from_file(context_params);
    // return ctx;
  } else if (context_type == "Clear") {
    return std::make_unique<ContextClear<PlaintextT> >();
  } else {
    throw std::runtime_error("Unknown context requested");
  }
}


void SheepServer::handle_get_job(http_request message) {
  /// is the sheep job fully configured?
  bool configured = m_job_config.iConfigured();
  message.reply(status_codes::OK);
}


void SheepServer::handle_post_job(http_request message) {
  /// reset the job config struct, we have a new job.
  m_job_config = {};
  message.reply(status_codes::OK);
}


void SheepServer::handle_get_parameters(http_request message) {
  
  /// if the context is present, 
  /// return a dict of parameters
  message.reply(status_codes::OK);  
}

void SheepServer::handle_put_parameters(http_request message) {
  /// put parameter name:value into job_config
  message.reply(status_codes::OK);
}



void SheepServer::handle_get(http_request message)
{
  auto path = message.relative_uri().path();
  if (0 != path.compare("parameters")) return handle_get_parameters(message);
  ucout <<  message.to_string() << endl;
  message.reply(status_codes::OK);
};

void SheepServer::handle_post(http_request message)
{
  auto path = message.relative_uri().path();
  if (0 != path.compare("parameters")) cout<<"PARAMETERS IN PATH"<<std::endl;
    ucout <<  message.to_string() << endl;
	message.reply(status_codes::OK);
};


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

