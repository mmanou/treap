/* ******************************************************************************************** *
 * COMP90077 Advanced Algorithms and Data Structures: Assignment 2
 *  Title: An Experimental Study on Treaps
 *  Author: Michael Manoussakis (834867)
 *  Date: 18/05/2023
 * ******************************************************************************************** */

#include <chrono>
#include <climits>
#include <ctime>
#include <iostream>
#include <random>
#include <tuple>

#define KEY_MAX 10000000
#define PRIORITY_MAX INT_MAX
#define NOT_FOUND -1

// Operation Indexes
#define I_OPTYPE 0  // applies to insertion_op, deletion_op, search_op
#define I_OPKEY 1   // applies to deletion_op, search_op
#define I_OPELEM 1  // applies to insertion_op

// Element Indexes
#define I_ELEMID 0
#define I_ELEMKEY 1

// Operation Labels
#define OPTYPE_INSERTION 1
#define OPTYPE_DELETION 2
#define OPTYPE_SEARCH 3

using namespace std;

typedef tuple<int, int> element;  // {I_ELEMID: int, I_ELEMKEY: int}

typedef tuple<int, element> insertion_op;  // {I_OPTYPE: OPTYPE_INSERTION, I_OPELEM: element},
typedef tuple<int, int> deletion_op;       // {I_OPTYPE: OPTYPE_DELETION,  I_OPKEY:  int}
typedef tuple<int, int> search_op;         // {I_OPTYPE: OPTYPE_SEARCH,    I_OPKEY   int}

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
};
RandIntGenerator rng;  // Global Random Int Generator for id, key

/* ******************************************************************************************** *
 *   DATA GENERATION
 * ******************************************************************************************** */

class DataGenerator {
   private:
    int id_next = 1;
    bool* exists;

   public:
    DataGenerator() { exists = (bool*)calloc(KEY_MAX, sizeof(bool)); }
    ~DataGenerator() { free(exists); }

    element gen_element() {
        element elem = {id_next++, rng.rand_key()};
        return elem;
    }

    insertion_op gen_insertion() {
        element elem = gen_element();
        insertion_op ins = {OPTYPE_INSERTION, elem};

        return ins;
    }

    deletion_op gen_deletion() {
        int del_id = rng.rand_id(id_next - 1);
        deletion_op del;
        if (exists[del_id]) {
            del = {OPTYPE_DELETION, rng.rand_key()};  // TODO: fix
        } else {
            del = {OPTYPE_DELETION, rng.rand_key()};
        }
        return del;
    }

    search_op gen_search() {
        search_op sch = {OPTYPE_SEARCH, rng.rand_key()};
        return sch;
    }
};

/* ******************************************************************************************** *
 *   DATA STRUCTURES FOR ANALYSIS
 * ******************************************************************************************** */

struct treap_node {
    element* elem;
    int priority;
    treap_node* left;
    treap_node* right;

    treap_node(element* e, int p, treap_node* l, treap_node* r)
        : elem(e), priority(p), left(l), right(r) {}
    treap_node(element* e, int p) : elem(e), priority(p), left(NULL), right(NULL) {}

    int get_key() { return get<I_ELEMKEY>(*elem); }

    // int get_id() { return get<I_ELEMID>(*elem); } DELETE: currently unused in treap
};

class RandomisedTreap {
   private:
    treap_node* head;

    treap_node* insert_node(treap_node* head, treap_node* n) {
        if (head == NULL) {
            return n;
        }
        // perform bst insert
        if (n->get_key() <= head->get_key()) {
            if (head->left == NULL) {  // insert here
                head->left = n;
            } else {  // recurse left
                insert_node(head->left, n);
            }
        } else {
            if (head->right == NULL) {  // insert here
                head->right = n;
            } else {  // recurse left
                insert_node(head->right, n);
            }
        }

        // if we inserted: fix the heap condition with rotations, then return the new head
        if (head->left != NULL && head->left->priority < head->priority) {
            return rotate_right(head);
        } else if (head->right != NULL && head->right->priority < head->priority) {
            return rotate_left(head);
        }

        // no rotations: return the original head
        return head;
    }

    treap_node* search_node(treap_node* head, int key) {
        if (head->get_key() == key) {
            return head;
        }
        if (head->get_key() < key && head->left != NULL) {  // go right
            return search_node(head->left, key);
        }
        if (key < head->get_key() && head->right != NULL) {  // go right
            return search_node(head->right, key);
        }
        return NULL;
    }

    // TODO: Deletion
    //  treap_node* delete_node(treap_node* node) {
    //      if (node->left == NULL && node->right == NULL) {
    //          return NULL;
    //      }
    //  }

    treap_node* rotate_left(treap_node* head) {
        treap_node* temp = head->right;
        head->right = temp->left;
        temp->left = head;
        return temp;
    }

    treap_node* rotate_right(treap_node* head) {
        treap_node* temp = head->left;
        head->left = temp->right;
        temp->right = head;
        return temp;
    }
    // treap_node* rotate_leftright() {} // DELETE:
    // treap_node* rotate_rightleft() {} // DELETE:

   public:
    void insert(element* e) {
        treap_node n(e, rng.rand_priority());
        head = insert_node(head, &n);
    }

    // TODO: Deletion
    //  void delet(int key) {
    //      head = delete_node(head, key);

    //     treap_node* node = search_node(head, key);
    //     if (node == NULL) {
    //         return;
    //     }
    //     delete_node(node);
    // }

    element* search(int key) {
        treap_node* node = search_node(head, key);
        if (node == NULL) {
            return NULL;
        }
        return node->elem;
    }
};

class DynamicArray {
   private:
    int count = 0;
    int capacity = 1;
    element* list;

    void grow() {
        capacity *= 2;
        resize();
    }

    void shrink() {
        capacity /= 2;
        resize();
    }

    void resize() {
        element* new_list = (element*)malloc(capacity * sizeof(element));
        for (int i = 0; i < count; i++) {
            new_list[i] = list[i];
        }
        free(list);
        list = new_list;
    }

   public:
    DynamicArray() { list = (element*)malloc(1 * sizeof(element)); }

    void insert(element x) {
        if (count + 1 == capacity) {
            grow();
        }
        list[count++] = x;
    }

    void delet(int key) {
        // search for key
        int pos = search(key);
        if (pos == NOT_FOUND) {
            return;
        }

        // swap with last elem, decrease count
        element temp = list[pos];
        count -= 1;
        list[pos] = list[count];
        list[count] = temp;

        if (count < capacity / 4) {
            shrink();
        }
    }
    int search(int key) {
        for (int i = 0; i < count; i++) {
            if (get<I_ELEMKEY>(list[i]) == key) {
                return i;
            }
        }
        return NOT_FOUND;
    }
};

/* ******************************************************************************************** *
 *   MAIN
 * ******************************************************************************************** */

int main(int argc, char** argv) {
    // DELETE:
    // auto start = std::chrono::system_clock::now();
    // // Some computation here
    // auto end = std::chrono::system_clock::now();

    // std::chrono::duration<double> elapsed_seconds = end - start;
    // std::time_t end_time = std::chrono::system_clock::to_time_t(end);

    // std::cout << "finished computation at " << std::ctime(&end_time)
    //           << "elapsed time: " << elapsed_seconds.count() << "s" << std::endl;

    //

    // DELETE:
    // uniform_int_distribution<> id_dist(1, 9);
    // uniform_int_distribution<> key_dist(0, KEY_MAX);

    // cout << id_dist(engine) << '\n';
    // cout << key_dist(engine);

    cout << "Initialise DataGenerator\n";

    DataGenerator dg;

    cout << "Initialise element\n";
    element t = dg.gen_element();

    cout << "Initialise insertion\n";
    insertion_op e1 = dg.gen_insertion();
    cout << "Initialise deletion\n";
    deletion_op e2 = dg.gen_deletion();
    cout << "Initialise search\n";
    search_op e3 = dg.gen_search();

    return 0;
}
