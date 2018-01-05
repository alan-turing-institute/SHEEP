-- Circuits with labelled gates

-- 'a' is the type of the label (probably String)
data Circuit a = Input a Bool
             | Xor a (Circuit a) (Circuit a)
             | And a (Circuit a) (Circuit a)
             | Id a (Circuit a)
       deriving (Show)

runCircuit :: Circuit a -> Bool
runCircuit (Input _ b) = b
runCircuit (Xor _ left right) = (runCircuit left) /= (runCircuit right)
runCircuit (And _ left right) = (runCircuit left) && (runCircuit right)
runCircuit (Id _ c) = runCircuit c

myCircuit i1 i2 = 
  let and1 = And "and1" i1 i2
      xor1 = Xor "xor1" i1 i2
  in Xor "result" and1 xor1

main = do 
  print c
  print $ runCircuit c
    where c = myCircuit (Input "input1" True) (Input "input2" True)
