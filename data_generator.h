#ifndef DATA_GENERATOR_H
#define DATA_GENERATOR_H

#include "rand_int_generator.h"

#define DELETED -1

/* ******************************************************************************************** *
 *   DATA GENERATION
 * ******************************************************************************************** */

class DataGenerator {
   private:
    int id_next = 1;
    int* key_list;

   public:
    DataGenerator() {
        key_list = (int*)malloc(KEY_MAX * sizeof(int));  // NOTE: Might need more than KEY_MAX
        for (int i = 0; i < KEY_MAX; i++) {
            key_list[i] = DELETED;
        }
    }
    ~DataGenerator() { free(key_list); }

    element gen_element() {
        int key = rng.rand_key();
        element elem = {id_next, key};
        key_list[id_next - 1] = key;
        id_next++;
        return elem;
    }

    insertion_op gen_insertion() {
        element elem = gen_element();
        insertion_op ins = {elem};

        return ins;
    }

    deletion_op gen_deletion() {
        int del_id = rng.rand_id(id_next - 1);
        deletion_op del;
        if (key_list[del_id - 1] != DELETED) {
            del = {key_list[del_id - 1]};
        } else {
            del = {rng.rand_key()};
        }
        return del;
    }

    search_op gen_search() {
        search_op sch = {rng.rand_key()};
        return sch;
    }

    // For experiment 0 only
    element gen_specific_element(int key) {
        element elem = {id_next, key};
        key_list[id_next - 1] = key;
        id_next++;
        return elem;
    }
    // For experiment 0 only
    insertion_op gen_specific_insertion(int key) {
        element elem = gen_specific_element(key);
        insertion_op ins = {elem};

        return ins;
    }
};

#endif