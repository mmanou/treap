/* ******************************************************************************************** *
 * COMP90077 Advanced Algorithms and Data Structures: Assignment 2
 *  Title: An Experimental Study on Treaps
 *  Author: Michael Manoussakis (834867)
 *  Date: 18/05/2023
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
        // assert(("Constructor capacity doesn't work how you think it does",
        //         vec.capacity() >= num_updates));

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

        vector<int> vec(num_updates);
        assert(("Constructor capacity doesn't work how you think it does",
                vec.capacity() >= num_updates));

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
        key_list = (int*)malloc(KEY_MAX * sizeof(int));  // TODO: Might need more than KEY_MAX
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

    // DELETE: unused old function
    // treap_node* find_parent(treap_node* head, int key) {
    //     // NOTE: Assumes that head has already been checked, and is not the node.
    //     // However, we cannot check again, because it might have a duplicate key.
    //     if (key < head->get_key()) {
    //         if (head->left == NULL) {
    //             return NULL;
    //         }
    //         if (head->left->get_key() == key) {
    //             return head;
    //         }
    //         return find_parent(head->left, key);
    //     }
    //     if (head->get_key() < key) {
    //         if (head->right == NULL) {
    //             return NULL;
    //         }
    //         if (head->right->get_key() == key) {
    //             return head;
    //         }
    //         return find_parent(head->right, key);
    //     }
    //     return NULL;
    // }

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
        //     print(parent, 0);
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

    int get_height() { return get_height(head, 0); }

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

void experiment0() {
    cout << "==Experiment 0==\n";
    const int E0_COUNT = 1024;
    // Initialise Data Structures
    DataGenerator dyn_array;
    RandomisedTreap r_treap;

    // Generate test data
    cout << "Create 1024 insertions\n";
    insertion_op* insertions = (insertion_op*)malloc(E0_COUNT * sizeof(insertion_op));
    for (int i = 0; i < E0_COUNT; i++) {
        insertions[i] = dyn_array.gen_insertion();
    }

    cout << "Insert 1024 elements into RandomisedTreap\n";
    for (int i = 0; i < E0_COUNT; i++) {
        r_treap.insert(get<I_OPELEM>(insertions[i]));
    }

    // Print results
    cout << "Treap_height=" << r_treap.get_height() << "\n";
    int* depths = r_treap.get_all_node_depths(E0_COUNT);
    cout << "Avg_node_depth=";
    for (int i = 0; i < E0_COUNT; i++) {
        cout << depths[i] << ",";
    }
    cout << "\n";

    free(insertions);
}

void experiment1() {
    cout << "==Experiment 1==\n";
    // Initialise Data Structures
    DataGenerator dg;
    DynamicArray dyn_array;
    RandomisedTreap r_treap;

    const int NUM_INSERTIONS = 1000000;

    // Generate insertions
    cout << "Create 1M insertions\n";
    insertion_op* insertions = (insertion_op*)malloc(NUM_INSERTIONS * sizeof(insertion_op));
    for (int i = 0; i < NUM_INSERTIONS; i++) {
        insertions[i] = dg.gen_insertion();
    }

    // Start test on DynamicArray
    cout << NUM_INSERTIONS << " insertions into DynamicArray\n";
    csc::time_point start_da = csc::now();  // Start timer
    for (int i = 0; i < NUM_INSERTIONS; i++) {
        dyn_array.insert(get<I_OPELEM>(insertions[i]));
    }
    csc::time_point end_da = csc::now();  // Stop timer
    print_time(start_da, end_da, "insertions into DynamicArray");

    // Start test on RandomisedTreap
    cout << NUM_INSERTIONS << " insertions into RandomisedTreap\n";
    csc::time_point start_rt = csc::now();  // Start timer
    for (int i = 0; i < NUM_INSERTIONS; i++) {
        r_treap.insert(get<I_OPELEM>(insertions[i]));
    }
    csc::time_point end_rt = csc::now();  // Stop timer
    print_time(start_rt, end_rt, "insertions into RandomisedTreap");

    free(insertions);
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
    // cout << NUM_OPERATIONS << " insertions, deletions on DynamicArray\n"; TODO: Uncomment
    // csc::time_point start_da = csc::now();  // Start timer
    // for (int i = 0; i < NUM_OPERATIONS; i++) {
    //     if (updates[i] == OPTYPE_INSERTION) {
    //         dyn_array.insert(get<I_OPELEM>(insertions[next_insertion++]));
    //     } else {  // OPTYPE_DELETION
    //         dyn_array.delet(get<I_OPKEY>(deletions[next_deletion++]));
    //     }
    // }
    // csc::time_point end_da = csc::now();  // Stop timer
    // print_time(start_da, end_da, "insertions, deletions on DynamicArray");

    // assert(("Insertions not all completed", next_insertion == num_insertions));
    // assert(("Deletions not all completed", next_deletion == num_deletions));

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

/* ******************************************************************************************** *
 *   MAIN
 * ******************************************************************************************** */

int main(int argc, char** argv) {
    // Start timer
    csc::time_point start_da = csc::now();
    time_t time0 = chrono::system_clock::to_time_t(start_da);

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

    csc::time_point end_da = csc::now();
    time_t time1 = chrono::system_clock::to_time_t(end_da);

    chrono::duration<double> elapsed_seconds = end_da - start_da;
    cout << "Finished at " << ctime(&time1);
    cout << "Elapsed time: " << elapsed_seconds.count() << "s\n";

    // experiment0();
    // experiment1();
    experiment2();

    return 0;
}
