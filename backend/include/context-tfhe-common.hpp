#ifndef CONTEXT_TFHE_COMMON_HPP
#define CONTEXT_TFHE_COMMON_HPP

namespace SHEEP {

struct CiphertextTFHE {
  std::shared_ptr<LweSample> sample;

 public:
  CiphertextTFHE(std::shared_ptr<TFheGateBootstrappingParameterSet> params)
      : sample(new_gate_bootstrapping_ciphertext(params.get()),
               [](LweSample *p) { delete_gate_bootstrapping_ciphertext(p); }) {}

  // Conversion operator to allow passing of Ciphertext objects
  // to functions expecting a C pointer.
  operator LweSample *() { return sample.get(); }
};

template <size_t N>
struct CiphertextArrayTFHE {
  std::shared_ptr<LweSample> sample;

 public:
  CiphertextArrayTFHE(std::shared_ptr<TFheGateBootstrappingParameterSet> params)
      : sample(new_gate_bootstrapping_ciphertext_array(N, params.get()),
               [](LweSample *p) {
                 delete_gate_bootstrapping_ciphertext_array(N, p);
               }) {}

  size_t size() const { return N; };

  // Conversion operator to allow passing of Ciphertext objects
  // to functions expecting a C pointer.
  operator LweSample *() { return sample.get(); }

  LweSample *operator[](size_t i) {
    if (i >= N)
      throw std::runtime_error("CiphertextArrayTFHE index out of range");
    return sample.get() + i;
  }
};

template <typename WireValueT>
class ContextTFHE;

}  // namespace SHEEP

#endif  // CONTEXT_TFHE_COMMON_HPP
