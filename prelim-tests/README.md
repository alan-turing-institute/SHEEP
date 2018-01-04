This are some preliminary ideas for the bechmarks and language

## Benchmarks (three levels)

  1. Low-level operations (for 1, 16, and 32 bit numbers)
    - Add
    - Mult
    - Mult-by-constant
    - Comparison
    - Multiplexor
    
  2. Mid-level operations (for several bitwidths)
    - Private Information Retrieval
    - Basic linear algebra: Inner product, Matrix multiplication, RELU, Approx. of Sigmoid
    - Basic statistics: Mean, STD^2
    - Automata evaluation
    
  3. High-level operations
    - Model evaluation (logistic, linear, NN)
    - Hypothesis testing (Chi-Square)

## Language

[Cingulata](https://github.com/CEA-LIST/Cingulata) (which was made open source very recently) transforms C programs into circuits and then manipulates such circuits with the ABC framework. 
