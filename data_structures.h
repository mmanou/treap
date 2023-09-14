#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include <algorithm>
#include <iostream>
#include <iterator>
#include <vector>

#include "data_generator.h"
#include "rand_int_generator.h"

#define NOT_FOUND -1

using namespace std;

/* ******************************************************************************************** *
 *   RANDOMISED TREAP
 * ******************************************************************************************** */

struct treap_node {
    element elem;
    int priority;
    treap_node* left;
    treap_node* right;

    treap_node(element e, int p) : elem(e), priority(p), left(NULL), right(NULL) {}

    int get_key() { return elem.KEY; }

    int get_id() { return elem.ID; }
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
    treap_node* search_node(treap_node* head, const int key) {
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

    treap_node* search_parent(treap_node* parent, treap_node* node, const int key) {
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
    void delete_node(treap_node* parent, const int key) {
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
        return max(get_height_and_depths_e0(node->left, total_depths, depth + 1),
                   get_height_and_depths_e0(node->right, total_depths, depth + 1));
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

    bool heap_condition_satisfied(const int parent_prio, treap_node* node) {
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
    void delet(const int key) {
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
    element* search(const int key) {
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

    int* get_all_node_depths(const int num_nodes) {
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
 *   DYNAMIC ARRAY
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
            if (list[i].KEY == key) {
                return i;
            }
        }
        return NOT_FOUND;
    }

    void print() {
        for (int i = 0; i < count; i++) {
            cout << '(' << list[i].ID << ", " << list[i].KEY << ")\n";
        }
    }
};

#endif  // DATA_STRUCTURES_H
