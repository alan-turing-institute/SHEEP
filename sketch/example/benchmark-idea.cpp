
typedef std::chrono::duration<double, std::micro> DurationT;

template <typename PlaintextT>
class BaseContext { };

template <typename PlaintextT>
std::unique_ptr<BaseContext<PlaintextT> >
make_context(std::string context_type, std::string context_params)
{
	if (context_type == "TFHE") {
		return std::make_unique<ContextTFHE<PlaintextT> >();
	} else {
		//return std::make_unique<ContextHElib<PlaintextT> >();
		return std::make_unique<ContextClear<PlaintextT> >();
	}
}

template <typename PlaintextT>
bool benchmark_run(std::unique_ptr<BaseContext<PlaintextT> > ctx,
		   Circuit C,
		   std::string input_filename,
		   DurationT& duration)
{
	// read in inputs from input_filename
	std::vector<PlaintextT> inputs = read_inputs_from_file<PlaintextT>(input_filename);
	
	std::vector<PlaintextT> result = ctx->eval_with_plaintexts(C, inputs, duration);

	// e.g. compare with ContextClear ...
	return true;
}

int main(void) {
	std::string plaintext_type;
	std::string context_type;

	std::ifstream infile("filename");
	
	Circuit C;
	infile >> C;

	DurationT duration;
	if (plaintext_type == "int8_t") {
		benchmark_run(make_context<int8_t>(context_type, ""), C, inputs, duration);
	} else if (plaintext_type == "uint8_t") {
		benchmark_run(make_context<uint8_t>(context_type, ""), C, inputs, duration);
	};
}
