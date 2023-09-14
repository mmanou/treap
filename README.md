# Randomised Treap

In this program, the performance of a **randomised treap** is compared with a baseline model, the
**dynamic array**.

We compare the *insert*, *delete*, and *search operations* of the two data structures in five
performance tests.
The dynamic array is implemented such that search and delete operations use Linear Search.
Both data structures were implemented from scratch in C++.

Each *element* is made up of two integers:
- a *key*
- a unique *ID*

There are three kinds of supported operations in the treap and dynamic array: *Insertion*, *Deletion*,
and *Search*. Experiments 2-4 required operations of two or more kinds to be performed.
To create a sequence of operations, arrays of each operation were created and filled with the
required number of operations. A sequence of integers corresponding to each type of operation was
created and shuffled. This was then used to decide which operation should be performed next by the
data structure under analysis.

Each insertion operation object contains an element to be inserted, while each deletion and search
contains an integer key of an arbitrary element.

## Experiments

In Experiment 0, 1024 elements were generated and inserted with unique keys from 0 to 1023 inclusive.
In Experiments 1-4, keys are from 0 to 10 million.

- **Experiment 0**: *Treap Height and Average Depths of Nodes*.

- **Experiment 1**: *Time vs Number of Insertions*.

- **Experiment 2**: *Time vs Deletion Percentage* (with decreasing Insertion percentage).

- **Experiment 3**: *Time vs Search Percentage* (with decreasing Insertion percentage).

- **Experiment 4**: *Time vs Length of Mixed-Operation Sequence* (5% Deletion, 5% Search, 90% Insertion).

## Running instructions

```
make all
./treap.exe               // run all experiments
./treap.exe <exp_number>  // run specific experiment
```
