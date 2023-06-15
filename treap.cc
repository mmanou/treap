/* ******************************************************************************************** *
 * COMP90077 Advanced Algorithms and Data Structures: Assignment 2
 *  Title: An Experimental Study on Treaps
 *  Author: Michael Manoussakis
 *  Date: 18/05/2023
 *
 *  Compilation command:
 *    g++ -std=c++14 -O3 -o treap.exe treap.cc
 *  Running instructions:
 *    ./treap.exe
 *
 * ******************************************************************************************** */

#include <cassert>
#include <chrono>
#include <ctime>
#include <iterator>
#include <vector>

#include "experiments.h"

using namespace std;

typedef chrono::system_clock csc;

/* ******************************************************************************************** *
 *   TESTS
 * ******************************************************************************************** */

void sanity_test() {
    DataGenerator dg;

    cout << "Initialise element\n";
    element t = dg.gen_element();

    cout << "Initialise insertion\n";
    insertion_op e1 = dg.gen_insertion();
    cout << "Initialise deletion\n";
    deletion_op e2 = dg.gen_deletion();
    cout << "Initialise search\n";
    search_op e3 = dg.gen_search();
}

/* ******************************************************************************************** *
 * * MAIN
 * ******************************************************************************************** */

int main(int argc, char** argv) {
    // Start timer
    csc::time_point start = csc::now();

    sanity_test();

    // Initialise Data Structures
    cout << "Initialise DataGenerator\n";
    DataGenerator dg;
    cout << "Initialise DynamicArray\n";
    DynamicArray dyn_array;
    cout << "Initialise RandomisedTreap\n";
    RandomisedTreap r_treap;

    cout << "Create 10 insertions\n";
    insertion_op insert1k[10] = {};
    for (int i = 0; i < 10; i++) {
        insert1k[i] = dg.gen_insertion();
    }

    // cout << "1000 insertions into DynamicArray\n";
    // for (int i = 0; i < 1000; i++) {
    //     dyn_array.insert(get<I_OPELEM>(insert1k[i]));
    // }

    cout << "10 insertions into RandomisedTreap\n";
    for (int i = 0; i < 10; i++) {
        r_treap.insert(insert1k[i].ELEM);
    }

    // cout << "print DynamicArray\n";
    // dyn_array.print();

    cout << "print RandomisedTreap\n";
    r_treap.print();

    cout << "5 searches in RandomisedTreap\n";
    for (int i = 0; i < 5; i++) {
        search_op s = dg.gen_search();
        cout << "searching key=" << s.KEY;
        element* e = r_treap.search(s.KEY);
        if (e == NULL) {
            cout << "=> not found\n";
        } else {
            cout << "=> found elem=(" << e->ID << ", " << e->KEY << ")\n";
        }
    }

    cout << "5 deletions from RandomisedTreap\n";
    for (int i = 0; i < 5; i++) {
        deletion_op d = dg.gen_deletion();
        cout << "deleting key=" << d.KEY << '\n';
        r_treap.delet(d.KEY);
    }

    cout << "print RandomisedTreap\n";
    r_treap.print();

    cout << "Treap height = " << r_treap.get_height() << "\n";
    int* depths = r_treap.get_all_node_depths(10);
    cout << "Avg node depth = [";
    for (int i = 0; i < 10; i++) {
        cout << depths[i] << ",";
    }
    cout << "]\n";
    free(depths);

    csc::time_point end = csc::now();
    time_t time1 = chrono::system_clock::to_time_t(end);

    chrono::duration<double> elapsed_seconds = end - start;
    cout << "Finished at " << ctime(&time1);
    cout << "Elapsed time: " << elapsed_seconds.count() << "s\n";

    experiment0();
    experiment1();
    experiment2();
    experiment3();
    experiment4();

    return 0;
}
