## Implementation specifics

- DecisionTree:
    - tree is parsed based on indentation consisting of 2 spaces (or 1 "\t" character)
    - **incorrect input tree is detected** and printed out as an error (`Either` type in `parseDecisionTree` function)
    - **incorrect traverse sequence is detected** when trying to label data (`Maybe` type in `traverseTree` function)

- DataFile (type to represent data used for training or labeling):
    - **each line must have same number of features**, otherwise an error is thrown

- CART algorithm:
    - uses Gini Index to calculate inpurity of base set, then it tries all features and respective thresholds to find a split, which reduces the inpurity in new subsets
    - the algorithm stops if all data belong to the same class (might lead to overfitting)
    - **it is possible to add `maxDepth`** of the resulting tree as a parameter to `createTreeCART` function, however default implementation does not use it
    - the tresholds are calculated as an average of 2 consecutive raw threshold values
    - **pruning was not implemented**

- Main:
    - **invalid arguments or not provinding enough arguments is detected** and returned as an error

## Project structure

The basic file structure consists of 4 files:
- **flp-fun.hs** contains the main part of the program, parses the arguments, call the appropriate subtask functions, collects the result and prints them to stdout 

- **Datafile.hs** contains parsing training/classification data files and provides `DataValue` type

- **DecisionTree.hs** contains the `DecisionTree` type with functions to read from string (subtask1), print (subtask2) and traverse the tree (subtask1) 

- **Cart.hs** contains the implementation of basic CART algorithm to generate `DecisionTree` from training data (subtask2) using Gini Index to calculate the best split and thresholds 

## General notes

Run the program with
```
make
```

Clean the build files with
```
make clean
```