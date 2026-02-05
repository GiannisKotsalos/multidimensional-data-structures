#include "Rtree.h"
#include <iostream>
#include <string>

RTree::RTree() : root(new Node(true)) {}

// Converts Movie object data into numerical coordinates for the R-Tree
void getCoords(const Movie* m, double coords[K]) {
    coords[0] = m->budget;
    coords[1] = m->revenue;
    coords[2] = (double)m->runtime;
    coords[3] = (double)m->release_year;
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
