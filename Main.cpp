#include <iostream>
#include <vector>
#include <string>
#include "Movie.h"
#include "FileReader.h"
#include "Rtree/Rtree.h" 

using namespace std;

// Function to handle user input and create a search query
Rect getUserQuery() {
    Rect query;
    string attrNames[K] = {"Budget", "Revenue", "Runtime", "Popularity", "Vote Average"};

    cout << "\n--- Define Your Search Range ---\n";
    for (int i = 0; i < K; ++i) {
        char choice;
        cout << "Filter by " << attrNames[i] << "? (y/n): ";
        cin >> choice;

        if (choice == 'y' || choice == 'Y') {
            cout << "  Enter min " << attrNames[i] << ": ";
            cin >> query.min_coords[i];
            cout << "  Enter max " << attrNames[i] << ": ";
            cin >> query.max_coords[i];
        } else {
            // Set default range to include everything if skipped
            query.min_coords[i] = 0;
            query.max_coords[i] = 1e15; 
        }
    }
    return query;
}

int main() {
    vector<Movie> movies = loadMoviesFromCSV("data_movies_clean.csv");
    if (movies.empty()) return 1;

    // 1. Build the R-Tree
    RTree tree;
    tree.build(movies);

    // 2. Get query from user once
    Rect myQuery = getUserQuery();

    // 3. Run search on R-Tree
    vector<Movie*> results = tree.search(myQuery);

    // 4. Run search on other algorithms (later)
    // vector<Movie*> btreeResults = myBTree.search(myQuery);

    // Display Results
    cout << "\nFound " << results.size() << " movies matching your criteria." << endl;
    for (size_t i = 0; i < min(results.size(), (size_t)5); ++i) {
        cout << " - " << results[i]->title << " (" << results[i]->vote_average << "/10)\n";
    }

    return 0;
}