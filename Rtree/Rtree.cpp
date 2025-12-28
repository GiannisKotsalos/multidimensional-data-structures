#include "Rtree.h"
#include <iostream>
#include <string>

RTree::RTree() : root(new Node(true)) {}

// Converts Movie object data into numerical coordinates for the R-Tree
void getCoords(const Movie* m, double coords[K]) {
    coords[0] = m->budget;
    coords[1] = m->revenue;
    coords[2] = (double)m->runtime;
    coords[3] = m->popularity;
    coords[4] = m->vote_average;
}

// Iterates through the movie list and inserts each into the index
void RTree::build(const vector<Movie>& movies) {
    cout << "\n--- Building R-Tree Index ---" << endl;
    for (const auto& m : movies) {
        insert(const_cast<Movie*>(&m));
    }
    cout << "R-Tree construction complete. Indexed " << movies.size() << " movies." << endl;
}

// Simple insertion: Wraps a movie in a leaf node and adds it to the root
void RTree::insert(Movie* movie) {
    double coords[K];
    getCoords(movie, coords);
    
    // Create a new leaf node for the movie
    Node* leaf = new Node(true);
    leaf->moviePtr = movie;
    leaf->mbr.expand(coords);

    // Add leaf to root and expand root's bounding box to include it
    root->children.push_back(leaf);
    root->mbr.expand(coords);
}

// Interactive terminal menu to define search ranges
void RTree::executeSearchPhase() {
    cout << "\n--- Executing Multi-Dimensional Query ---" << endl;
    Rect query;
    string attrNames[K] = {"Budget", "Revenue", "Runtime", "Popularity", "Vote Average"};

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
            // If user skips a filter, set range to 0 -> Huge Number to include everything
            query.min_coords[i] = 0;
            query.max_coords[i] = 1e15; 
        }
    }

    vector<Movie*> Candidates = search(query);
    cout << "\nQuery found " << Candidates.size() << " candidates." << endl;
    
    // Display only the first 5 Candidates to keep the terminal clean
    int displayCount = (Candidates.size() > 5) ? 5 : (int)Candidates.size();
    for (int i = 0; i < displayCount; ++i) {
        cout << " - " << Candidates[i]->title 
                  << " (Pop: " << Candidates[i]->popularity 
                  << ", Rating: " << Candidates[i]->vote_average << ")" << endl;
    }
}

// Public wrapper for the recursive search
vector<Movie*> RTree::search(const Rect& query) {
    vector<Movie*> Candidates;
    searchInternal(root, query, Candidates);
    return Candidates;
}

// The core search logic: pruning branches that don't overlap with the user's query
void RTree::searchInternal(Node* node, const Rect& query, vector<Movie*>& Candidates) {
    // Optimization: If this node's box doesn't overlap the query, stop looking here
    if (!node->mbr.overlaps(query)) return;

    // If we reached a leaf node, add the movie to Candidates
    if (node->isLeaf && node->moviePtr) {
        Candidates.push_back(node->moviePtr);
    } else {
        // If it's an internal node, search all of its children recursively
        for (Node* child : node->children) {
            searchInternal(child, query, Candidates);
        }
    }
}
