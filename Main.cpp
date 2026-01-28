#include <iostream>
#include <vector>
#include "Movie.h"
#include "FileReader.h"
#include "Rtree/Rtree.h" 

using namespace std;

int main() {
    string filename = "data_movies_clean.csv";

    cout << "--- Movie Data System ---\n";
    
    cout << "Loading movies...\n";
    // 1. Load Data
    vector<Movie> movies = loadMoviesFromCSV(filename);

    if (movies.empty()) {
        cerr << "No data loaded. Exiting." << endl;
        return 1;
    }
    
    cout << "Loaded " << movies.size() << " movies successfully.\n";

    // 2. Choose Algorithm (Example: R-Tree)
    cout << "Initializing R-Tree index..." << endl;
    RTree tree;
    tree.build(movies);
    tree.executeSearchPhase();
    
    // You can easily add more algorithms here later:
    // BTree btree;
    // btree.build(movies);

    return 0;
}