/* ******************************************************************************************** *
 * COMP90077 Advanced Algorithms and Data Structures: Assignment 2
 *  Title: An Experimental Study on Treaps
 *  Author: Michael Manoussakis
 *  Date: 03/07/2023
 * ******************************************************************************************** */

#include <cassert>
#include <chrono>
#include <ctime>
#include <iterator>
#include <vector>

#include "experiments.h"

#define ALL_EXPERIMENTS -1

using namespace std;

typedef chrono::system_clock csc;

/* ******************************************************************************************** *
 *   TESTS
 * ******************************************************************************************** */

void sanity_test_1() {
    csc::time_point start = csc::now();  // Start timer

    DataGenerator dg;

    cout << "Initialise element\n";
    element t = dg.gen_element();

    cout << "Initialise insertion\n";
    insertion_op e1 = dg.gen_insertion();
    cout << "Initialise deletion\n";
    deletion_op e2 = dg.gen_deletion();
    cout << "Initialise search\n";
    search_op e3 = dg.gen_search();

    csc::time_point end = csc::now();  // Stop timer
    print_time(start, end, "Sanity Test 1");
}

void sanity_test_2() {

    csc::time_point start = csc::now();  // Start timer

    cout << "Initialise Data Structures\n";
    DataGenerator dg;
    DynamicArray dyn_array;
    RandomisedTreap r_treap;

    cout << "Create 10 insertions\n";
    insertion_op insert1k[10] = {};
    for (int i = 0; i < 10; i++) {
        insert1k[i] = dg.gen_insertion();
    }

    cout << "10 insertions into DynamicArray\n";
    for (int i = 0; i < 10; i++) {
        dyn_array.insert(insert1k[i].ELEM);
    }

    cout << "10 insertions into RandomisedTreap\n";
    for (int i = 0; i < 10; i++) {
        r_treap.insert(insert1k[i].ELEM);
    }

    cout << "print DynamicArray\n";
    dyn_array.print();

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

    csc::time_point end = csc::now();  // Stop timer
    print_time(start, end, "Sanity Test");
}

/* ******************************************************************************************** *
 *   MAIN
 * ******************************************************************************************** */

int main(int argc, char** argv) {
    if (argc > 2) {
        cout << "Too many arguments.";
        return 1;
    }

    int experiment_num = ALL_EXPERIMENTS;

    if (argc > 1) {
        experiment_num = atoi(argv[1]);

        if (experiment_num < 0 || experiment_num > 4) {
            cout << "Invalid experiment number. Expected 0-4.";
            return 1;
        }
    }

    cout << "==Sanity Test==\n";
    sanity_test_1();
    sanity_test_2();

    switch (experiment_num) {
        case ALL_EXPERIMENTS:
            experiment0();
            experiment1();
            experiment2();
            experiment3();
            experiment4();
            break;
        case 0:
            experiment0();
            break;
        case 1:
            experiment1();
            break;
        case 2:
            experiment2();
            break;
        case 3:
            experiment3();
            break;
        case 4:
            experiment4();
            break;
    }
    return 0;
}
