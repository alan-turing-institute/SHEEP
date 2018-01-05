-- simplest implementation, where a circuit outputs a single Bool

data Circuit = Input Bool
             | Xor Circuit Circuit
             | And Circuit Circuit
             | Id Circuit
       deriving (Show)

runCircuit :: Circuit -> Bool
runCircuit (Input b) = b
runCircuit (Xor left right) = (runCircuit left) /= (runCircuit right)
runCircuit (And left right) = (runCircuit left) && (runCircuit right)
runCircuit (Id c) = runCircuit c

myCircuit i1 i2 = 
  let and1 = And i1 i2
      xor1 = Xor i1 i2  
  in Xor and1 xor1

main = do 
  print c
  print $ runCircuit c
    where c = myCircuit (Input True) (Input False)
