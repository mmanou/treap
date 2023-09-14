#include "experiments.h"

using namespace std;

RandIntGenerator rng;

/* ******************************************************************************************** *
 *   TIMER
 * ******************************************************************************************** */

/* @param start csc::time_point start = csc::now();
 * @param end csc::time_point end = csc::now();
 */
void print_time(csc::time_point start, csc::time_point end, string activity) {
    const time_t t = csc::to_time_t(end);
    const chrono::duration<double> elapsed_seconds = end - start;
    cout << "Finished " << activity << " at " << ctime(&t)
         << "Elapsed time: " << elapsed_seconds.count() << "s\n\n";
}

/* ******************************************************************************************** *
 *   EXPERIMENT 0
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
        r_treap.insert(insertions[i].ELEM);
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
    int* total_depths = (int*)calloc(E0_COUNT, sizeof(int));

    for (int i = 0; i < NUM_TRIALS; i++) {
        experiment0_phase(total_depths);
    }

    cout << "average_depths=[";
    for (int i = 0; i < E0_COUNT; i++) {
        cout << (total_depths[i] / NUM_TRIALS) << ", ";
    }
    cout << "]\n";

    free(total_depths);
}

/* ******************************************************************************************** *
 *   EXPERIMENT 1
 * ******************************************************************************************** */

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
        dyn_array.insert(insertions[i].ELEM);
    }
    csc::time_point end_da = csc::now();  // Stop timer
    print_time(start_da, end_da, "insertions into DynamicArray");

    // Start test on RandomisedTreap
    cout << num_insertions << " insertions into RandomisedTreap\n";
    csc::time_point start_rt = csc::now();  // Start timer
    for (int i = 0; i < num_insertions; i++) {
        r_treap.insert(insertions[i].ELEM);
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

/* ******************************************************************************************** *
 *   EXPERIMENT 2
 * ******************************************************************************************** */

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
            dyn_array.insert(insertions[next_insertion++].ELEM);
        } else {  // OPTYPE_DELETION
            dyn_array.delet(deletions[next_deletion++].KEY);
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
            r_treap.insert(insertions[next_insertion++].ELEM);
        } else {  // OPTYPE_DELETION
            r_treap.delet(deletions[next_deletion++].KEY);
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
 *   EXPERIMENT 3
 * ******************************************************************************************** */

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
    const csc::time_point start_da = csc::now();  // Start timer
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        if (updates[i] == OPTYPE_INSERTION) {
            dyn_array.insert(insertions[next_insertion++].ELEM);
        } else {  // OPTYPE_SEARCH
            dyn_array.search(searches[next_search++].KEY);
        }
    }
    const csc::time_point end_da = csc::now();  // Stop timer
    print_time(start_da, end_da, "insertions, searches on DynamicArray");

    assert(("Insertions not all completed", next_insertion == num_insertions));
    assert(("Searches not all completed", next_search == num_searches));

    // Start test on RandomisedTreap
    next_insertion = 0;
    next_search = 0;
    cout << NUM_OPERATIONS << " insertions, searches on RandomisedTreap\n";
    const csc::time_point start_rt = csc::now();  // Start timer
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        if (updates[i] == OPTYPE_INSERTION) {
            r_treap.insert(insertions[next_insertion++].ELEM);
        } else {  // OPTYPE_SEARCH
            r_treap.search(searches[next_search++].KEY);
        }
    }
    const csc::time_point end_rt = csc::now();  // Stop timer
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

/* ******************************************************************************************** *
 *   EXPERIMENT 4
 * ******************************************************************************************** */

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
    const csc::time_point start_da = csc::now();  // Start timer
    for (int i = 0; i < num_operations; i++) {
        if (updates[i] == OPTYPE_INSERTION) {
            dyn_array.insert(insertions[next_insertion++].ELEM);
        } else if (updates[i] == OPTYPE_DELETION) {
            dyn_array.delet(deletions[next_deletion++].KEY);
        } else {  // OPTYPE_SEARCH
            dyn_array.search(searches[next_search++].KEY);
        }
    }
    const csc::time_point end_da = csc::now();  // Stop timer
    print_time(start_da, end_da, "insertions, deletions, searches on DynamicArray");

    assert(("Insertions not all completed", next_insertion == num_insertions));
    assert(("Deletions not all completed", next_deletion == num_deletions));
    assert(("Searches not all completed", next_search == num_searches));

    // Start test on RandomisedTreap
    next_insertion = 0;
    next_deletion = 0;
    next_search = 0;
    cout << num_operations << " insertions, deletions, searches on RandomisedTreap\n";
    const csc::time_point start_rt = csc::now();  // Start timer
    for (int i = 0; i < num_operations; i++) {
        if (updates[i] == OPTYPE_INSERTION) {
            r_treap.insert(insertions[next_insertion++].ELEM);
        } else if (updates[i] == OPTYPE_DELETION) {
            r_treap.delet(deletions[next_deletion++].KEY);
        } else {  // OPTYPE_SEARCH
            r_treap.search(searches[next_search++].KEY);
        }
    }
    const csc::time_point end_rt = csc::now();  // Stop timer
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
