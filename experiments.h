#ifndef EXPERIMENTS_H
#define EXPERIMENTS_H

#include <cassert>
#include <iostream>
#include <vector>
#include <chrono>

#include "data_structures.h"

typedef chrono::system_clock csc;

void print_time(csc::time_point start, csc::time_point end, string activity);
void experiment0();
void experiment1();
void experiment2();
void experiment3();
void experiment4();

#endif  // EXPERIMENTS_H
