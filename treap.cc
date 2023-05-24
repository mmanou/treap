/* ******************************************************************************************** *
 * COMP90077 Advanced Algorithms and Data Structures: Assignment 2
 *  Title: An Experimental Study on Treaps
 *  Author: Michael Manoussakis (834867)
 *  Date: 18/05/2023
 *
 *  To compile, run:
 *    g++ -std=c++14 -O3 -o treap.exe treap.cc
 * ******************************************************************************************** */

#include <algorithm>
#include <cassert>
#include <chrono>
#include <climits>
#include <ctime>
#include <iostream>
#include <iterator>
#include <random>
#include <tuple>
#include <vector>

#define KEY_MAX 10000000
#define PRIORITY_MAX INT_MAX
#define NOT_FOUND -1
#define DELETED -1

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

typedef chrono::system_clock csc;

typedef tuple<int, int> element;  // {I_ELEMID: int, I_ELEMKEY: int}

typedef tuple<int, element> insertion_op;  // {I_OPTYPE: OPTYPE_INSERTION, I_OPELEM: element},
typedef tuple<int, int> deletion_op;       // {I_OPTYPE: OPTYPE_DELETION,  I_OPKEY:  int}
typedef tuple<int, int> search_op;         // {I_OPTYPE: OPTYPE_SEARCH,    I_OPKEY   int}

/* ******************************************************************************************** *
 *   TIMER
 * ******************************************************************************************** */
/* @param start csc::time_point start = csc::now();
 * @param end csc::time_point end = csc::now(); */
void print_time(csc::time_point start, csc::time_point end, string activity) {
    time_t t = csc::to_time_t(end);
    chrono::duration<double> elapsed_seconds = end - start;
    cout << "Finished " << activity << " at " << ctime(&t)
         << "Elapsed time: " << elapsed_seconds.count() << "s\n\n";
}

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
RandIntGenerator rng;  // Global Random Int Generator for (id, key, priority), and update sequences

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
        insertion_op ins = {OPTYPE_INSERTION, elem};

        return ins;
    }

    deletion_op gen_deletion() {
        int del_id = rng.rand_id(id_next - 1);
        deletion_op del;
        if (key_list[del_id - 1] != DELETED) {
            del = {OPTYPE_DELETION, key_list[del_id - 1]};
        } else {
            del = {OPTYPE_DELETION, rng.rand_key()};
        }
        return del;
    }

    search_op gen_search() {
        search_op sch = {OPTYPE_SEARCH, rng.rand_key()};
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
        insertion_op ins = {OPTYPE_INSERTION, elem};

        return ins;
    }
};

/* ******************************************************************************************** *
 *   DATA STRUCTURES FOR ANALYSIS: RANDOMISED TREAP
 * ******************************************************************************************** */

struct treap_node {
    element elem;
    int priority;
    treap_node* left;
    treap_node* right;

    treap_node(element e, int p) : elem(e), priority(p), left(NULL), right(NULL) {}

    int get_key() { return get<I_ELEMKEY>(elem); }

    int get_id() { return get<I_ELEMID>(elem); }
};

class RandomisedTreap {
   private:
    treap_node* head;

    // Core helper function for insertion operation
    treap_node* insert_node(treap_node* head, treap_node* n) {
        if (head == NULL) {
            return n;
        }
        // perform bst insert
        if (n->get_key() <= head->get_key()) {  // TODO: Can use id to break ties for '==' case
            if (head->left == NULL) {           // insert here
                head->left = n;
            } else {  // recurse left
                head->left = insert_node(head->left, n);
            }
        } else {
            if (head->right == NULL) {  // insert here
                head->right = n;
            } else {  // recurse left
                head->right = insert_node(head->right, n);
            }
        }

        // TODO: Could move this into the bst insert logic. Might be slightly faster
        //  if we inserted: fix the heap condition with rotations, then return the new head
        if (head->left != NULL && head->left->priority < head->priority) {
            return rotate_right(head);
        } else if (head->right != NULL && head->right->priority < head->priority) {
            return rotate_left(head);
        }

        // no rotations: return the original head
        return head;
    }

    // Core helper function for search operation
    treap_node* search_node(treap_node* head, int key) {
        if (head->get_key() == key) {
            return head;
        }
        if (key < head->get_key() && head->left != NULL) {  // go left
            return search_node(head->left, key);
        }
        if (head->get_key() < key && head->right != NULL) {  // go right
            return search_node(head->right, key);
        }
        return NULL;
    }

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

    treap_node* search_parent(treap_node* parent, treap_node* node, int key) {
        if (parent != NULL && node->get_key() == key) {
            node->priority = INT_MAX;  // mark for deletion
            return parent;
        }
        if (key < node->get_key() && node->left != NULL) {  // go left
            return search_parent(node, node->left, key);
        }
        if (node->get_key() < key && node->right != NULL) {  // go right
            return search_parent(node, node->right, key);
        }
        return NULL;
    }

    bool is_leaf_node(treap_node* node) {
        if (node == NULL) {
            cout << "Unexpected check of node being a leaf";
            return false;
        }
        if (node->left == NULL && node->right == NULL) return true;
        return false;
    }

    bool only_has_left_child(treap_node* node) {
        if (node->left != NULL && node->right == NULL) {
            return true;
        }
        return false;
    }

    bool only_has_right_child(treap_node* node) {
        if (node->left == NULL && node->right != NULL) {
            return true;
        }
        return false;
    }

    bool left_smaller_than_right(treap_node* node) {
        // NOTE: Assumes that left and right both exist
        assert(("ERROR: Expected left and right to both exist",
                node->left != NULL && node->right != NULL));

        if ((node->left->priority < node->right->priority) ||  // priority
            (node->left->priority == node->right->priority &&  // tiebreak using key
             node->left->get_key() < node->right->get_key())) {
            return true;
        }
        return false;
    }

    // Core helper function for deletion operation
    void delete_node(treap_node* parent, int key) {
        if (parent == NULL) {
            return;
        }

        if (key < parent->get_key() ||
            (key == parent->get_key() && parent->left != NULL &&
             key == parent->left->get_key())) {  // Target node is left child
            assert(("ERROR: Expected left child to exist", parent->left != NULL));
            assert(
                ("ERROR: Expected left child to be the target", parent->left->get_key() == key));

            if (is_leaf_node(parent->left)) {  // is leaf => delete
                delete (parent->left);
                parent->left = NULL;
                return;
            }
            if (only_has_right_child(parent->left)) {
                parent->left = rotate_left(parent->left);
            } else if (only_has_left_child(parent->left)) {
                parent->left = rotate_right(parent->left);
            } else if (left_smaller_than_right(parent->left)) {
                parent->left = rotate_right(parent->left);
            } else {  // right smaller than left
                parent->left = rotate_left(parent->left);
            }
            delete_node(parent->left, key);
        } else {  // Target node is right child
            assert(("ERROR: Expected right child to exist", parent->right != NULL));
            assert(
                ("ERROR: Expected right child to be the target", parent->right->get_key() == key));

            if (is_leaf_node(parent->right)) {  // is leaf => delete
                delete (parent->right);
                parent->right = NULL;
                return;
            }
            if (only_has_right_child(parent->right)) {
                parent->right = rotate_left(parent->right);
            } else if (only_has_left_child(parent->right)) {
                parent->right = rotate_right(parent->right);
            } else if (left_smaller_than_right(parent->right)) {
                parent->right = rotate_right(parent->right);
            } else {  // right smaller than left
                parent->right = rotate_left(parent->right);
            }
            delete_node(parent->right, key);
        }
    }

    // Core helper function for height
    int get_height(treap_node* node, int depth) {
        if (node == NULL) {
            return depth;
        }
        return max(get_height(node->left, depth + 1), get_height(node->right, depth + 1));
    }

    // Core helper function for heigh and node depth
    int get_height_and_depths_e0(treap_node* node, int* total_depths, int depth) {
        if (node == NULL) {
            return depth;
        }
        total_depths[node->get_key()] += depth;
        return max(get_height_and_depths_e0(node->left, total_depths, depth + 1), get_height_and_depths_e0(node->right, total_depths, depth + 1));
    }

    // Core helper function for node depth
    int get_all_node_depths(treap_node* node, int curr_id, int curr_depth, int* depth_array) {
        if (node == NULL) {
            return curr_id;
        }
        depth_array[curr_id] = curr_depth;
        curr_id = get_all_node_depths(node->left, curr_id + 1, curr_depth + 1, depth_array);
        curr_id = get_all_node_depths(node->right, curr_id, curr_depth + 1,
                                      depth_array);  // TODO: Double-check increment of curr_id
        return curr_id;
    }

    int find_depth_of_key_node(treap_node* node, const int key, int depth) {
        if (node == NULL) {
            return NOT_FOUND;
        }
        if (node->get_key() == key) {
            return depth;
        }
        return max(find_depth_of_key_node(node->left, key, depth + 1),
                   find_depth_of_key_node(node->right, key, depth + 1));
    }

    void print(treap_node* head, int depth) {
        for (int i = 0; i < depth; i++) {
            cout << "_";
        }
        if (head == NULL) {
            cout << "*EMPTY*\n";
            return;
        }
        cout << '(' << head->get_id() << ", " << head->get_key() << ", " << head->priority
             << ")\n";
        print(head->left, depth + 1);
        print(head->right, depth + 1);
    }

    bool heap_condition_satisfied(int parent_prio, treap_node* node) {
        if (node == NULL) {
            return true;
        }
        if (node->priority < parent_prio) {
            cout << "Failed heap condition: prio=" << node->priority
                 << " parent_prio=" << parent_prio << '\n';

            return false;
        }
        return heap_condition_satisfied(node->priority, node->left) &&
               heap_condition_satisfied(node->priority, node->right);
    }

    bool bst_condition_satisfied(treap_node* node) {
        if (node == NULL) {
            return true;
        }
        if (node->left != NULL && node->get_key() < node->left->get_key()) {
            return false;
        }
        if (node->right != NULL && node->get_key() > node->right->get_key()) {
            return false;
        }

        bool satisfied =
            (bst_condition_satisfied(node->left) && bst_condition_satisfied(node->right));

        if (!satisfied) {
            cout << "Failed bst condition\n";
        }
        return satisfied;
    }

    // Deallocate all memory
    void dealloc_head(treap_node* head) {
        if (head == NULL) return;
        if (head->left != NULL) {
            dealloc_head(head->left);
        }
        if (head->right != NULL) {
            dealloc_head(head->right);
        }
        delete (head);
    }

   public:
    RandomisedTreap() : head(NULL) {}
    ~RandomisedTreap() { dealloc_head(head); }

    // Perform insertion operation
    void insert(element e) {
        treap_node* n = new treap_node(e, rng.rand_priority());
        head = insert_node(head, n);
    }

    // Perform deletion operation
    void delet(int key) {
        if (head == NULL) {
            return;
        }
        if (head->get_key() == key) {
            cout << "Head deletion\n";
            head->priority = INT_MAX;

            if (is_leaf_node(head)) {  // is leaf => delete
                delete (head);
                head = NULL;
                return;
            } else if (only_has_right_child(head)) {
                head = rotate_left(head);
                delete (head->left);
                head->left = NULL;
                return;
            } else if (only_has_left_child(head)) {
                head = rotate_right(head);
                delete (head->right);
                head->right = NULL;
                return;
            } else if (left_smaller_than_right(head)) {
                head = rotate_right(head);
                delete_node(head, key);
                return;
            } else {  // right smaller than left
                head = rotate_left(head);
                delete_node(head, key);
                return;
            }
            return;
        }
        treap_node* parent = search_parent(NULL, head, key);
        if (parent == NULL) {
            return;
        }
        delete_node(parent, key);
        //  DELETE:
        // if (!heap_condition_satisfied(INT_MIN, parent)) {
        //     // print(parent, 0);
        // }
    }

    // Perform search operation
    element* search(int key) {
        treap_node* node = search_node(head, key);
        if (node == NULL) {
            return NULL;
        }
        return &node->elem;
    }

    int find_depth_of_key(const int key) { return find_depth_of_key_node(head, key, 0); }

    int get_height() { return get_height(head, 0); }

    int get_height_and_depths_e0(int* total_depths) {
        return get_height_and_depths_e0(head, total_depths, 0);
    }

    int* get_all_node_depths(int num_nodes) {
        if (head == NULL) {
            return NULL;
        }

        int* depths = (int*)calloc(num_nodes, sizeof(int));
        get_all_node_depths(head, 0, 0, depths);

        return depths;
    }

    bool heap_condition_satisfied() {
        if (head == NULL) {
            return true;
        }
        return heap_condition_satisfied(INT_MIN, head);
    }

    bool bst_condition_satisfied() {
        if (head == NULL) {
            return true;
        }
        return bst_condition_satisfied(head);
    }

    void print() { print(head, 0); }
};

/* ******************************************************************************************** *
 *   DATA STRUCTURES FOR ANALYSIS: DYNAMIC ARRAY
 * ******************************************************************************************** */

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
    ~DynamicArray() { free(list); }

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

        if (count < (capacity / 4)) {
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

    void print() {
        for (int i = 0; i < count; i++) {
            cout << '(' << get<I_ELEMID>(list[i]) << ", " << get<I_ELEMKEY>(list[i]) << ")\n";
        }
    }
};

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
 *   EXPERIMENTS
 * ******************************************************************************************** */

void experiment0_phase(int* total_depths) {
    const int E0_COUNT = 1024;
    const int KEY_511 = 511;
    // Initialise Data Structures
    DataGenerator dg;
    RandomisedTreap r_treap;

    // Generate test data
    cout << "Create 1024 insertions\n";
    vector<insertion_op> insertions;
    // insertion_op* insertions = (insertion_op*)malloc(E0_COUNT * sizeof(insertion_op));
    for (int i = 0; i < E0_COUNT; i++) {
        insertions.push_back(dg.gen_specific_insertion(i));
    }

    // shuffle insertions
    random_device rd;
    mt19937 engine{rd()};
    std::shuffle(insertions.begin(), insertions.end(), engine);

    assert(("Expected 1024 insertions", insertions.size() == E0_COUNT));

    cout << "Insert 1024 elements into RandomisedTreap\n";
    for (int i = 0; i < E0_COUNT; i++) {
        r_treap.insert(get<I_OPELEM>(insertions[i]));
    }

    // Print results
    cout << "Treap_height=" << r_treap.get_height() << "\n";
    // int* depths = r_treap.get_all_node_depths(E0_COUNT); // DELETE:
    cout << "KEY 512 Depth=" << r_treap.find_depth_of_key(KEY_511);
    r_treap.get_height_and_depths_e0(total_depths);

    // DELETE:
    // cout << "Node_depths=";
    // for (int i = 0; i < E0_COUNT; i++) {
    //     cout << depths[i] << ",";
    // }
    // cout << "\n";

    // free(depths);

}

void experiment0() {
    const int E0_COUNT = 1024;
    const int NUM_TRIALS = 100;
    int* total_depths = (int *)calloc(E0_COUNT, sizeof(int));

    for (int i=0; i<NUM_TRIALS; i++) {
        experiment0_phase(total_depths);
    }

    cout << "average_depths=[";
    for (int i=0; i<E0_COUNT; i++) {
        cout << (total_depths[i] / NUM_TRIALS) << ", ";
    }
    cout << "]\n";

    free(total_depths);
}


void experiment1_phase(const int num_insertions) {
    // Initialise Data Structures
    DataGenerator dg;
    DynamicArray dyn_array;
    RandomisedTreap r_treap;

    // Generate insertions
    insertion_op* insertions = (insertion_op*)malloc(num_insertions * sizeof(insertion_op));
    for (int i = 0; i < num_insertions; i++) {
        insertions[i] = dg.gen_insertion();
    }

    // Start test on DynamicArray
    cout << num_insertions << " insertions into DynamicArray\n";
    csc::time_point start_da = csc::now();  // Start timer
    for (int i = 0; i < num_insertions; i++) {
        dyn_array.insert(get<I_OPELEM>(insertions[i]));
    }
    csc::time_point end_da = csc::now();  // Stop timer
    print_time(start_da, end_da, "insertions into DynamicArray");

    // Start test on RandomisedTreap
    cout << num_insertions << " insertions into RandomisedTreap\n";
    csc::time_point start_rt = csc::now();  // Start timer
    for (int i = 0; i < num_insertions; i++) {
        r_treap.insert(get<I_OPELEM>(insertions[i]));
    }
    csc::time_point end_rt = csc::now();  // Stop timer
    print_time(start_rt, end_rt, "insertions into RandomisedTreap");

    free(insertions);
}

void experiment1() {
    cout << "==Experiment 1==\n"
         << "> Num insertions (L) = 100000\n";
    experiment1_phase(100000);
    cout << "> END 0.1M\n\n";

    cout << "> Num insertions (L) = 200000\n";
    experiment1_phase(200000);
    cout << "> END 0.2M\n\n";

    cout << "> Num insertions (L) = 500000\n";
    experiment1_phase(500000);
    cout << "> END 0.5M\n\n";

    cout << "> Num insertions (L) = 800000\n";
    experiment1_phase(800000);
    cout << "> END 0.8M\n\n";

    cout << "> Num insertions (L) = 1000000\n";
    experiment1_phase(1000000);
    cout << "> END 1M\n\n";
}

void experiment2_phase(const int num_insertions, const int num_deletions) {
    const int NUM_OPERATIONS = 1000000;
    assert(("Expected num_insertions + num_deletions == NUM_OPERATIONS",
            num_insertions + num_deletions == NUM_OPERATIONS));

    // Initialise Data Structures
    DataGenerator dg;
    DynamicArray dyn_array;
    RandomisedTreap r_treap;

    // Generate update sequence
    insertion_op* insertions = (insertion_op*)malloc(num_insertions * sizeof(insertion_op));
    for (int i = 0; i < num_insertions; i++) {
        insertions[i] = dg.gen_insertion();
    }
    deletion_op* deletions = (deletion_op*)malloc(num_deletions * sizeof(deletion_op));
    for (int i = 0; i < num_deletions; i++) {
        deletions[i] = dg.gen_deletion();
    }
    vector<int> updates = rng.rand_update_sequence2(
        NUM_OPERATIONS, OPTYPE_INSERTION, num_insertions, OPTYPE_DELETION, num_deletions);

    // Start test on DynamicArray
    int next_insertion = 0;
    int next_deletion = 0;
    cout << NUM_OPERATIONS << " insertions, deletions on DynamicArray\n";
    csc::time_point start_da = csc::now();  // Start timer
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        if (updates[i] == OPTYPE_INSERTION) {
            dyn_array.insert(get<I_OPELEM>(insertions[next_insertion++]));
        } else {  // OPTYPE_DELETION
            dyn_array.delet(get<I_OPKEY>(deletions[next_deletion++]));
        }
    }
    csc::time_point end_da = csc::now();  // Stop timer
    print_time(start_da, end_da, "insertions, deletions on DynamicArray");

    assert(("Insertions not all completed", next_insertion == num_insertions));
    assert(("Deletions not all completed", next_deletion == num_deletions));

    // Start test on RandomisedTreap
    next_insertion = 0;
    next_deletion = 0;
    cout << NUM_OPERATIONS << " insertions, deletions on RandomisedTreap\n";
    csc::time_point start_rt = csc::now();  // Start timer
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        if (updates[i] == OPTYPE_INSERTION) {
            r_treap.insert(get<I_OPELEM>(insertions[next_insertion++]));
        } else {  // OPTYPE_DELETION
            r_treap.delet(get<I_OPKEY>(deletions[next_deletion++]));
        }
    }
    csc::time_point end_rt = csc::now();  // Stop timer
    print_time(start_rt, end_rt, "insertions, deletions on RandomisedTreap");

    assert(("Insertions not all completed", next_insertion == num_insertions));
    assert(("Deletions not all completed", next_deletion == num_deletions));
    // assert(("Heap condition was not satisfied", r_treap.heap_condition_satisfied()));
    assert(("BST condition was not satisfied", r_treap.bst_condition_satisfied()));

    free(insertions);
    free(deletions);
}

void experiment2() {
    cout << "==Experiment 2==\n"
         << "> Deletion Probability = 0.1%\n";
    experiment2_phase(999000, 1000);
    cout << "> END 0.1%\n\n";

    cout << "> Deletion Probability = 0.5%\n";
    experiment2_phase(995000, 5000);
    cout << "> END 0.5%\n\n";

    cout << "> Deletion Probability = 1%\n";
    experiment2_phase(990000, 10000);
    cout << "> END 1%\n\n";

    cout << "> Deletion Probability = 5%\n";
    experiment2_phase(950000, 50000);
    cout << "> END 5%\n\n";

    cout << "> Deletion Probability = 10%\n";
    experiment2_phase(900000, 100000);
    cout << "> END 10%\n\n";
}

void experiment3_phase(const int num_insertions, const int num_searches) {
    const int NUM_OPERATIONS = 1000000;
    assert(("Expected num_insertions + num_searches == NUM_OPERATIONS",
            num_insertions + num_searches == NUM_OPERATIONS));

    // Initialise Data Structures
    DataGenerator dg;
    DynamicArray dyn_array;
    RandomisedTreap r_treap;

    // Generate update sequence
    insertion_op* insertions = (insertion_op*)malloc(num_insertions * sizeof(insertion_op));
    for (int i = 0; i < num_insertions; i++) {
        insertions[i] = dg.gen_insertion();
    }
    search_op* searches = (search_op*)malloc(num_searches * sizeof(search_op));
    for (int i = 0; i < num_searches; i++) {
        searches[i] = dg.gen_search();
    }
    vector<int> updates = rng.rand_update_sequence2(NUM_OPERATIONS, OPTYPE_INSERTION,
                                                    num_insertions, OPTYPE_SEARCH, num_searches);

    // Start test on DynamicArray
    int next_insertion = 0;
    int next_search = 0;
    cout << NUM_OPERATIONS << " insertions, searches on DynamicArray\n";
    csc::time_point start_da = csc::now();  // Start timer
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        if (updates[i] == OPTYPE_INSERTION) {
            dyn_array.insert(get<I_OPELEM>(insertions[next_insertion++]));
        } else {  // OPTYPE_SEARCH
            dyn_array.search(get<I_OPKEY>(searches[next_search++]));
        }
    }
    csc::time_point end_da = csc::now();  // Stop timer
    print_time(start_da, end_da, "insertions, searches on DynamicArray");

    assert(("Insertions not all completed", next_insertion == num_insertions));
    assert(("Searches not all completed", next_search == num_searches));

    // Start test on RandomisedTreap
    next_insertion = 0;
    next_search = 0;
    cout << NUM_OPERATIONS << " insertions, searches on RandomisedTreap\n";
    csc::time_point start_rt = csc::now();  // Start timer
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        if (updates[i] == OPTYPE_INSERTION) {
            r_treap.insert(get<I_OPELEM>(insertions[next_insertion++]));
        } else {  // OPTYPE_SEARCH
            r_treap.search(get<I_OPKEY>(searches[next_search++]));
        }
    }
    csc::time_point end_rt = csc::now();  // Stop timer
    print_time(start_rt, end_rt, "insertions, searches on RandomisedTreap");

    assert(("Insertions not all completed", next_insertion == num_insertions));
    assert(("Searches not all completed", next_search == num_searches));
    // assert(("Heap condition was not satisfied", r_treap.heap_condition_satisfied()));
    assert(("BST condition was not satisfied", r_treap.bst_condition_satisfied()));

    free(insertions);
    free(searches);
}

void experiment3() {
    cout << "==Experiment 3==\n"
         << "> Search Probability = 0.1%\n";
    experiment3_phase(999000, 1000);
    cout << "> END 0.1%\n\n";

    cout << "> Search Probability = 0.5%\n";
    experiment3_phase(995000, 5000);
    cout << "> END 0.5%\n\n";

    cout << "> Search Probability = 1%\n";
    experiment3_phase(990000, 10000);
    cout << "> END 1%\n\n";

    cout << "> Search Probability = 5%\n";
    experiment3_phase(950000, 50000);
    cout << "> END 5%\n\n";

    cout << "> Search Probability = 10%\n";
    experiment3_phase(900000, 100000);
    cout << "> END 10%\n\n";
}

void experiment4_phase(const int num_operations, const int num_insertions, const int num_deletions,
                       const int num_searches) {
    assert(("Expected num_insertions + num_deletions + num_searches == num_operations",
            (num_insertions + num_deletions + num_searches) == num_operations));

    // Initialise Data Structures
    DataGenerator dg;
    DynamicArray dyn_array;
    RandomisedTreap r_treap;

    // Generate update sequence
    insertion_op* insertions = (insertion_op*)malloc(num_insertions * sizeof(insertion_op));
    for (int i = 0; i < num_insertions; i++) {
        insertions[i] = dg.gen_insertion();
    }
    deletion_op* deletions = (deletion_op*)malloc(num_deletions * sizeof(deletion_op));
    for (int i = 0; i < num_deletions; i++) {
        deletions[i] = dg.gen_deletion();
    }
    search_op* searches = (search_op*)malloc(num_searches * sizeof(search_op));
    for (int i = 0; i < num_searches; i++) {
        searches[i] = dg.gen_search();
    }
    vector<int> updates =
        rng.rand_update_sequence3(num_operations, OPTYPE_INSERTION, num_insertions,
                                  OPTYPE_DELETION, num_deletions, OPTYPE_SEARCH, num_searches);

    // Start test on DynamicArray
    int next_insertion = 0;
    int next_deletion = 0;
    int next_search = 0;
    cout << num_operations << " insertions, deletions, searches on DynamicArray\n";
    csc::time_point start_da = csc::now();  // Start timer
    for (int i = 0; i < num_operations; i++) {
        if (updates[i] == OPTYPE_INSERTION) {
            dyn_array.insert(get<I_OPELEM>(insertions[next_insertion++]));
        } else if (updates[i] == OPTYPE_DELETION) {
            dyn_array.delet(get<I_OPKEY>(deletions[next_deletion++]));
        } else {  // OPTYPE_SEARCH
            dyn_array.search(get<I_OPKEY>(searches[next_search++]));
        }
    }
    csc::time_point end_da = csc::now();  // Stop timer
    print_time(start_da, end_da, "insertions, deletions, searches on DynamicArray");

    assert(("Insertions not all completed", next_insertion == num_insertions));
    assert(("Deletions not all completed", next_deletion == num_deletions));
    assert(("Searches not all completed", next_search == num_searches));

    // Start test on RandomisedTreap
    next_insertion = 0;
    next_deletion = 0;
    next_search = 0;
    cout << num_operations << " insertions, deletions, searches on RandomisedTreap\n";
    csc::time_point start_rt = csc::now();  // Start timer
    for (int i = 0; i < num_operations; i++) {
        if (updates[i] == OPTYPE_INSERTION) {
            r_treap.insert(get<I_OPELEM>(insertions[next_insertion++]));
        } else if (updates[i] == OPTYPE_DELETION) {
            r_treap.delet(get<I_OPKEY>(deletions[next_deletion++]));
        } else {  // OPTYPE_SEARCH
            r_treap.search(get<I_OPKEY>(searches[next_search++]));
        }
    }
    csc::time_point end_rt = csc::now();  // Stop timer
    print_time(start_rt, end_rt, "insertions, deletions, searches on RandomisedTreap");

    assert(("Insertions not all completed", next_insertion == num_insertions));
    assert(("Deletions not all completed", next_deletion == num_deletions));
    assert(("Searches not all completed", next_search == num_searches));
    assert(("BST condition was not satisfied", r_treap.bst_condition_satisfied()));
    free(insertions);
    free(deletions);
    free(searches);
}

void experiment4() {
    cout << "==Experiment 4==\n"
         << ">  Num. Mixed operations (L) = 100000\n";
    experiment4_phase(100000, 90000, 5000, 5000);
    cout << "> END L=0.1M\n\n";

    cout << "> Num. Mixed operations (L) = 200000\n";
    experiment4_phase(200000, 180000, 10000, 10000);
    cout << "> END L=0.2M\n\n";

    cout << "> Num. Mixed operations (L) = 500000\n";
    experiment4_phase(500000, 450000, 25000, 25000);
    cout << "> END L=0.5M\n\n";

    cout << "> Num. Mixed operations (L) = 800000\n";
    experiment4_phase(800000, 720000, 40000, 40000);
    cout << "> END L=0.8M\n\n";

    cout << "> Num. Mixed operations (L) = 1000000\n";
    experiment4_phase(1000000, 900000, 50000, 50000);
    cout << "> END L=1M\n\n";
}

/* ********************************************************************************************
 * * MAIN
 * ********************************************************************************************
 */

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
        r_treap.insert(get<I_OPELEM>(insert1k[i]));
    }

    // cout << "print DynamicArray\n";
    // dyn_array.print();

    cout << "print RandomisedTreap\n";
    r_treap.print();

    cout << "5 searches in RandomisedTreap\n";
    for (int i = 0; i < 5; i++) {
        search_op s = dg.gen_search();
        cout << "searching key=" << get<I_OPKEY>(s);
        element* e = r_treap.search(get<I_OPKEY>(s));
        if (e == NULL) {
            cout << "=> not found\n";
        } else {
            cout << "=> found elem=(" << get<I_ELEMID>(*e) << ", " << get<I_ELEMKEY>(*e) << ")\n";
        }
    }

    cout << "5 deletions from RandomisedTreap\n";
    for (int i = 0; i < 5; i++) {
        deletion_op d = dg.gen_deletion();
        cout << "deleting key=" << get<I_OPKEY>(d) << '\n';
        r_treap.delet(get<I_OPKEY>(d));
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
