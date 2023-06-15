# Randomised Treap

In this program, the performance of a randomised treap is compared with a baseline model, the
dynamic array.
We compare the insert, delete, and search operations of the two data structures in five
performance tests (see Experimental Results).
The dynamic array is implemented such that search and delete operations use a Linear Search
implementation. Both data structures were implemented from scratch in C++.

Each element contains a key and a unique ID. Both are integers. In Experiment 0, 1024 elements
were generated with unique keys from 0 to 1023 inclusive. In Experiments 1-4, keys are from 0 to 10
million.
There are three kinds of supported operations in the treap and dynamic array: Insertion, Deletion,
and Search. Experiments 2-4 required operations of two or more kinds to be performed.
Each insertion operation object contains an element to be inserted, and each deletion and search
contains an integer key of an arbitrary element.
To create a sequence of operations, arrays of each operation were created and filled with the
required number of operations. A sequence of integers corresponding to each type of operation was
created and shuffled. This was then used to decide which operation should be performed next by the
data structure under analysis.

## Running instructions

```
make all
./treap.exe
```
