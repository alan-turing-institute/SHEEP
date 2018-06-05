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
using namespace SHEEP;

SheepServer::SheepServer(utility::string_t url) : m_listener(url)//, m_context(0)
{
    m_job_config = {};  /// zero all fields
    /// generic methods to listen for any request - the URL will be parsed within these functions.
    m_listener.support(methods::GET, std::bind(&SheepServer::handle_get, this, std::placeholders::_1));
    m_listener.support(methods::PUT, std::bind(&SheepServer::handle_put, this, std::placeholders::_1));
    m_listener.support(methods::POST, std::bind(&SheepServer::handle_post, this, std::placeholders::_1));
   
}

template <typename PlaintextT>
BaseContext<PlaintextT>*
SheepServer::make_context(std::string context_type) {
  auto ctx =  new ContextHElib_F2<PlaintextT>();
  return ctx;
  //  m_context = dynamic_cast<ContextHElib_F2<PlaintextT>* >(ctx);
  /*
  if (context_type == "HElib_F2") {
    ///    auto ctx =  std::make_unique<ContextHElib_F2<PlaintextT> >();
    auto ctx =  new ContextHElib_F2<PlaintextT>();    
    return ctx;
  } else if (context_type == "HElib_Fp") {
    //    auto ctx =  std::make_unique<ContextHElib_Fp<PlaintextT> >();
    auto ctx = new ContextHElib_Fp<PlaintextT>();    
    return ctx;
  } else if (context_type == "TFHE") {
    auto ctx =  std::make_unique<ContextTFHE<PlaintextT> >();
    return ctx;
    //	} else if (context_type == "SEAL") {
    //auto ctx =  std::make_unique<ContextSeal<PlaintextT> >();
    // return ctx;
  } else if (context_type == "Clear") {
    return std::make_unique<ContextClear<PlaintextT> >();
  } else {
    throw std::runtime_error("Unknown context requested");
  }
  */
}


void SheepServer::handle_get_context(http_request message) {
  /// list of available contexts?
  json::value result = json::value::object();
  json::value context_list = json::value::array();
  int index = 0;
  for (auto contextIter = available_contexts.begin();
       contextIter != available_contexts.end();
       ++contextIter) {
    context_list[index] = json::value::string(*contextIter);
    index++;
  }
  result["available_contexts"] = context_list;
  
  message.reply(status_codes::OK, result);
}

void SheepServer::handle_get_input_type(http_request message) {
  /// list of available contexts?
  json::value result = json::value::object();
  json::value type_list = json::value::array();
  int index = 0;
  for (auto typeIter = available_input_types.begin();
       typeIter != available_input_types.end();
       ++typeIter) {
    type_list[index] = json::value::string(*typeIter);
    index++;
  }
  result["available_input_types"] = type_list;
  
  message.reply(status_codes::OK, result);
}


void SheepServer::handle_post_context(http_request message) {
  /// set which context to use
  cout<<" in handle post context" <<endl;
  message.extract_json().then([=](pplx::task<json::value> jvalue) {
      try {
	json::value val = jvalue.get();
	auto context = val["context_name"].as_string();
	std::cout<<" context is "<<context<<std::endl;
	m_job_config.context = context;
	std::cout<<" job context is "<<m_job_config.context<<std::endl;
	
      } catch(json::json_exception) {
	  message.reply(status_codes::InternalError,("Unrecognized context request"));
      }      
    });
  message.reply(status_codes::OK);
}

void SheepServer::handle_post_input_type(http_request message) {
  /// set which input_type to use
  cout<<" in handle post input type" <<endl;
  message.extract_json().then([=](pplx::task<json::value> jvalue) {
      try {
	json::value val = jvalue.get();
	auto input_type = val["input_type"].as_string();
	std::cout<<" input_type is "<<input_type<<std::endl;
	m_job_config.input_type = input_type;
      } catch(json::json_exception) {
	  message.reply(status_codes::InternalError,("Unrecognized context request"));
      }      
    });
  message.reply(status_codes::OK);
}



void SheepServer::handle_get_job(http_request message) {
  /// is the sheep job fully configured?
  bool configured = m_job_config.isConfigured();
  message.reply(status_codes::OK);
}


void SheepServer::handle_post_job(http_request message) {
  /// reset the job config struct, we have a new job.
  m_job_config = {};
  message.reply(status_codes::OK);
}


void SheepServer::handle_get_parameters(http_request message) {
  
  /// if the job_config has some parameters set, then return them.
  /// Otherwise, create a new context, and get the default parameters.
  /// return a dict of parameters
  std::map<std::string, long&> param_map;
  if (m_job_config.input_type == "bool") {
    auto context = make_context<bool>(m_job_config.context);
    param_map = context->get_parameters();
  }
  json::value result = json::value::object();
  json::value param_list = json::value::array();
  int index = 0;
  for ( auto map_iter = param_map.begin(); map_iter != param_map.end(); ++map_iter) {
    //  std::cout<<"Parameter "<<map_iter->first<<" = "<<map_iter->second<<std::endl;
    json::value param = json::value::object();
    param[map_iter->first] = json::value::number((int64_t)map_iter->second);
    param_list[index] = param;
    index++;
  }
  
  //  for (auto contextIter = available_contexts.begin();
  //       contextIter != available_contexts.end();
  //     ++contextIter) {
  //  context_list[index] = json::value::string(*contextIter);
  //  index++;
  
  result["parameters"] = param_list;

 
  
  //  std::map<std::string, long&> parameters = m_context->get_parameters();
  
  // for ( auto map_iter = parameters.begin(); map_iter != parameters.end(); ++map_iter) {
  //  std::cout<<"Parameter "<<map_iter->first<<" = "<<map_iter->second<<std::endl;
  // }
  
  message.reply(status_codes::OK, result);  
}

void SheepServer::handle_put_parameters(http_request message) {
  /// put parameter name:value into job_config
  message.reply(status_codes::OK);
}



void SheepServer::handle_get(http_request message)
{
  auto path = message.relative_uri().path();
  if (path == "context/") return handle_get_context(message);  
  else if (path == "parameters/") return handle_get_parameters(message);
  else if (path == "input_type/") return handle_get_input_type(message);  
  message.reply(status_codes::OK);
};

void SheepServer::handle_post(http_request message)
{
  cout<<" in handle post " <<endl;

  auto path = message.relative_uri().path();
  cout<<" path is "<<path<<endl;
  if (path == "context/") return handle_post_context(message);  
  else if (path == "input_type/") return handle_post_input_type(message);
  //  else if (0 != path.compare("parameters")) cout<<"PARAMETERS IN PATH"<<std::endl;
  
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

