#ifndef RTREE_H
#define RTREE_H

#include <vector>
#include <algorithm>
#include <limits>
#include <iostream>
#include "../Movie.h" //Include Movie struct

// K defines the number of dimensions we are indexing (Budget, Revenue, etc.)
const int K = 5; 

// Rect represents a K-dimensional box (Minimum Bounding Rectangle)
struct Rect {
    double min_coords[K]; // Minimum values for each dimension
    double max_coords[K]; // Maximum values for each dimension

    // Initialize with infinity so any real value will expand the box
    Rect() {
        for (int i = 0; i < K; ++i) {
            min_coords[i] = numeric_limits<double>::max();
            max_coords[i] = -numeric_limits<double>::max();
        }
    }
    // Grows the rectangle to include a new set of coordinates
    void expand(const double coords[K]) {
        for (int i = 0; i < K; ++i) {
            min_coords[i] = min(min_coords[i], coords[i]);
            max_coords[i] = max(max_coords[i], coords[i]);
        }
    }

    // Checks if this rectangle overlaps with another (used for searching)
    bool overlaps(const Rect& other) const {
        for (int i = 0; i < K; ++i) {
            if (min_coords[i] > other.max_coords[i] || max_coords[i] < other.min_coords[i])
                return false;
        }
        return true;
    }

    // Calculates the "volume" of the K-dimensional box
    double area() const {
        double a = 1.0;
        for (int i = 0; i < K; ++i) a *= (max_coords[i] - min_coords[i]);
        return a;
    }
};

// Represents a node in the R-Tree
struct Node {
    bool isLeaf;               // True if this node points to a Movie, False if it points to other Nodes
    Rect mbr;                  // The bounding box that encloses all children
    vector<Node*> children;    // Pointers to child nodes
    Movie* moviePtr;           // Pointer to the actual data (only used in leaf nodes)

    Node(bool leaf) : isLeaf(leaf), moviePtr(nullptr) {}
};

class RTree {
public:
    RTree();
    ~RTree(); // Added destructor to prevent memory leaks
    
    void build(const vector<Movie>& movies);    // Main entry point to index the dataset
    void insert(Movie* movie);                  // Adds a single movie to the tree
    bool remove(int movieId);                   // Removes a movie by ID
    bool update(int movieId, Movie* newMovieData); // Updates a movie record

    vector<Movie*> search(const Rect& query);   // Finds movies within a specific range

private:
    Node* root;
    
    // Recursive helper for the search operation
    void searchInternal(Node* node, const Rect& query, vector<Movie*>& results);
    
    // Recursive helper for remove operation
    bool removeInternal(Node* node, int movieId);
    
    // Helper to destroy tree
    void destroyRecursive(Node* node);
    
    // Helper to recalculate MBR after deletion
    void recalculateMBR(Node* node);
};

#endif