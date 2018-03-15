#include <memory>
#include <fstream>
#include <map>

#include "context-clear.hpp"
#include "context-helib.hpp"
#include "context-tfhe.hpp"


using namespace Sheep;

typedef std::chrono::duration<double, std::micro> DurationT;


template <typename PlaintextT>
std::unique_ptr<BaseContext<PlaintextT> >
make_context(std::string context_type, std::string context_params="") {
	if (context_type == "HElib_F2") {
	  auto ctx =  std::make_unique<HElib::ContextHElib_F2<PlaintextT> >();
	  if (context_params.length() > 0)
	    ctx->read_params_from_file(context_params);
	  return ctx;
	} else if (context_type == "HElib_Fp") {
	  auto ctx =  std::make_unique<HElib::ContextHElib_Fp<PlaintextT> >();
	  if (context_params.length() > 0)
	    ctx->read_params_from_file(context_params);
	  return ctx;
	} else if (context_type == "TFHE") {
	  auto ctx =  std::make_unique<TFHE::ContextTFHE<PlaintextT> >();
	  return ctx;
	} else {
	  return std::make_unique<Clear::ContextClear<PlaintextT> >();
	}
}



template<typename T>
std::vector<T> read_inputs_file(std::string filename) {
  std::vector<T> inputs_list;
  std::ifstream inputstream(filename);	  
  if (inputstream.bad()) {
    std::cout<<"Empty or non-existent input file"<<std::endl;
  }
  
  /// loop over all lines in the input file 
  std::string line;
  while (std::getline(inputstream, line) ) {
    /// remove comments (lines starting with #) and empty lines
    int found= line.find_first_not_of(" \t");
    if( found != std::string::npos) {   
      if ( line[found] == '#') 
	continue;
      
      /// split up by whitespace
      std::string buffer;
      std::vector<std::string> tokens;
      std::stringstream ss(line);
      while (ss >> buffer) tokens.push_back(buffer);
      
      if (tokens.size() == 2) {   /// assume we have param_name param_value
	long input_val_long = stol(tokens[1]);
	T input_val = (T)(input_val_long);
	inputs_list.push_back(input_val);
      }	      
    }    
  } // end of loop over lines
  return inputs_list;
}

template <typename PlaintextT>
void print_outputs(std::vector<PlaintextT> test_results, std::vector<PlaintextT> control_results, std::vector<DurationT>& durations) {
  std::cout<<std::endl<<"==============="<<std::endl;
  std::cout<<"=== RESULTS ==="<<std::endl<<std::endl;
  std::cout<<"== Processing times: =="<<std::endl;
  std::cout<<"setup: "<<durations[0].count()<<std::endl;
  std::cout<<"encryption: "<<durations[1].count()<<std::endl;
  std::cout<<"circuit_evaluation: "<<durations[2].count()<<std::endl;  
  std::cout<<"decryption: "<<durations[3].count()<<std::endl;    
  std::cout<<std::endl;
  
  if (test_results.size() != control_results.size()) {
    std::cout<<"Outputs have different size - something went wrong!"<<std::endl;
    return;
  }
  std::cout<<"== Output values (test vs clear) =="<<std::endl;
  auto test_iter = test_results.begin();
  auto ctrl_iter = control_results.begin();
  while (test_iter != test_results.end()) {
    std::cout<<"  test context : "<<std::to_string(*test_iter);
    std::cout<<"  clear context : "<<std::to_string(*ctrl_iter)<<std::endl;
    test_iter++;
    ctrl_iter++;
  }
  std::cout<<endl<<"==== END RESULTS ==="<<std::endl;
  
}

template <typename PlaintextT>
bool benchmark_run(std::string context_name, std::string parameter_file,
		   Circuit C,
		   std::string input_filename)
{

        std::vector<DurationT> durations;
	typedef std::chrono::duration<double, std::micro> microsecond;
	typedef std::chrono::high_resolution_clock high_res_clock;
	auto setup_start_time = high_res_clock::now();
	
	std::unique_ptr<BaseContext<PlaintextT> > test_ctx =
	  make_context<PlaintextT>(context_name, parameter_file);

	auto setup_end_time = high_res_clock::now();	
	durations.push_back(microsecond(setup_end_time - setup_start_time));	

	std::unique_ptr<BaseContext<PlaintextT> > clear_ctx = make_context<PlaintextT>("Clear");
	
	// read in inputs from input_filename
	std::vector<PlaintextT> inputs = read_inputs_file<PlaintextT>(input_filename);
	std::vector<PlaintextT> result_bench = test_ctx->eval_with_plaintexts(C, inputs, durations);
	std::vector<DurationT> dummy;
	std::vector<PlaintextT> result_clear = clear_ctx->eval_with_plaintexts(C, inputs, dummy);	


	print_outputs(result_bench, result_clear, durations);
	
	return true;
}





int
main(int argc, const char** argv) {

  
  if (argc < 5) {

    std::cout<<"Usage: <path_to>/custom_example circuit_file context_name input_type inputs_file [params_file]"<<std::endl;
    return 0;
  }

  /// read the circuit
  std::ifstream input_circuit(argv[1]);
  Circuit C;
  input_circuit >> C;
  std::cout << C;


  /// read the other input args
  std::string context_name = argv[2];  
  std::string input_type = argv[3];
  std::string inputs_file = argv[4];
  std::string parameter_file = "";
  if (argc == 6)
    parameter_file = argv[5];

  std::cout<<"Benchmark: circuit: " <<argv[1]<<" context "<<context_name
	   <<" input_type "<<input_type<<" inputs_file "<<inputs_file
	   <<" parameter_file "<<parameter_file<<std::endl;

  /// run the benchmark
  bool isOK = false;
  if (input_type == "bool") {
    isOK = benchmark_run<bool>(context_name, parameter_file, C, inputs_file);
  } else if (input_type == "uint8_t") {
    isOK = benchmark_run<uint8_t>(context_name, parameter_file,C, inputs_file);    
  } else if (input_type == "int8_t") {
    isOK = benchmark_run<int8_t>(context_name, parameter_file, C, inputs_file);
  }  else if (input_type == "uint16_t") {
    isOK = benchmark_run<uint16_t>(context_name, parameter_file, C, inputs_file);
  } else if (input_type == "int16_t") {
    isOK = benchmark_run<int16_t>(context_name, parameter_file, C, inputs_file);
  } else if (input_type == "uint32_t") {
    isOK = benchmark_run<int32_t>(context_name, parameter_file, C, inputs_file);
  } else if (input_type == "int32_t") {
    isOK = benchmark_run<int32_t>(context_name, parameter_file, C, inputs_file);
  }
    
  return isOK;
}
