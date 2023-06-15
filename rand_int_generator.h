#ifndef RAND_INT_GENERATOR_H
#define RAND_INT_GENERATOR_H

#include <algorithm>
#include <cassert>
#include <climits>
#include <iostream>
#include <iterator>
#include <random>
#include <vector>

// Operation Indexes
#define I_OPTYPE 0  // applies to insertion_op, deletion_op, search_op
#define I_OPKEY 1   // applies to deletion_op, search_op
#define I_OPELEM 1  // applies to insertion_op

// Operation Labels
#define OPTYPE_INSERTION 1
#define OPTYPE_DELETION 2
#define OPTYPE_SEARCH 3

#define KEY_MAX 10000000
#define PRIORITY_MAX INT_MAX

#define DELETED -1

using namespace std;

/* ******************************************************************************************** *
 *   ELEMENTS AND OPERATIONS
 * ******************************************************************************************** */

struct element {
    int ID;
    int KEY;
};

struct insertion_op {
    element ELEM;
};

struct deletion_op {
    int KEY;
};

struct search_op {
    int KEY;
};

/* ******************************************************************************************** *
 *   RANDOM NUMBER GENERATION
 * ******************************************************************************************** */

class RandIntGenerator {
   private:
    random_device rd;
    mt19937 engine{rd()};
    uniform_int_distribution<> id_dist;
    uniform_int_distribution<> key_dist;
    uniform_int_distribution<> prio_dist;

   public:
    RandIntGenerator() : id_dist(1, 9), key_dist(0, KEY_MAX), prio_dist(0, PRIORITY_MAX) {}

    int rand_id() { return id_dist(engine); }

    int rand_id(int max) {
        uniform_int_distribution<> custom_dist(1, max);
        return custom_dist(engine);
    }

    int rand_key() { return key_dist(engine); }

    int rand_priority() { return prio_dist(engine); }

    /* @param type{int} OPTYPE_INSERTION, OPTYPE_DELETION, or OPTYPE_SEARCH*/
    vector<int> rand_update_sequence2(int num_updates, int type1, int count1, int type2,
                                      int count2) {
        assert(("Invalid input for type1: expected an OPTYPE",
                type1 == OPTYPE_INSERTION || type1 == OPTYPE_DELETION || type1 == OPTYPE_SEARCH));
        assert(("Invalid input for type2: expected an OPTYPE",
                type2 == OPTYPE_INSERTION || type2 == OPTYPE_DELETION || type2 == OPTYPE_SEARCH));

        assert(("Incorrect input: Expected count1 + count2 == num_updates",
                count1 + count2 == num_updates));

        vector<int> vec;

        for (int i = 0; i < count1; i++) {
            vec.push_back(type1);
        }
        for (int i = 0; i < count2; i++) {
            vec.push_back(type2);
        }

        std::shuffle(vec.begin(), vec.end(), engine);

        cout << "shuffled vector: length=" << vec.size() << " vec=[";
        for (int i = 0; i < 10; i++) {
            cout << vec[i] << ',';
        }
        cout << "...";
        for (int i = num_updates - 10; i < num_updates; i++) {
            cout << ',' << vec[i];
        }
        cout << "]\n";

        return vec;
    }

    /* @param type{int} OPTYPE_INSERTION, OPTYPE_DELETION, or OPTYPE_SEARCH*/
    vector<int> rand_update_sequence3(int num_updates, int type1, int count1, int type2,
                                      int count2, int type3, int count3) {
        assert(("Incorrect input: Expected count1 + count2 + count 3 == num_updates",
                count1 + count2 + count3 == num_updates));
        assert(("Invalid input for type1: expected an OPTYPE",
                type1 == OPTYPE_INSERTION || type1 == OPTYPE_DELETION || type1 == OPTYPE_SEARCH));
        assert(("Invalid input for type2: expected an OPTYPE",
                type2 == OPTYPE_INSERTION || type2 == OPTYPE_DELETION || type2 == OPTYPE_SEARCH));
        assert(("Invalid input for type3: expected an OPTYPE",
                type3 == OPTYPE_INSERTION || type3 == OPTYPE_DELETION || type3 == OPTYPE_SEARCH));

        vector<int> vec;

        for (int i = 0; i < count1; i++) {
            vec.push_back(type1);
        }
        for (int i = 0; i < count2; i++) {
            vec.push_back(type2);
        }
        for (int i = 0; i < count3; i++) {
            vec.push_back(type3);
        }

        std::shuffle(vec.begin(), vec.end(), engine);

        cout << "shuffled vector: length=" << vec.size() << " vec=[";
        for (int i = 0; i < 10; i++) {
            cout << vec[i] << ',';
        }
        cout << "...";
        for (int i = num_updates - 10; i < num_updates; i++) {
            cout << ',' << vec[i];
        }
        cout << "]\n";

        return vec;
    }
};

#endif