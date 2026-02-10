#ifndef KDTREE_H
#define KDTREE_H

#include <iostream>
#include <array>
#include <cmath>
#include <vector>
#include <limits>
#include <algorithm>
#include <string>
#include <unordered_map>
#include "../Movie.h"

using namespace std;

// Number of dimensions for KD-Tree (matches R-Tree)
const int KD_DIMS = 5;

// Represents one movie as a point in 5D space (plus its stable ID).
struct KDPoint {
    array<double, KD_DIMS> coordinates; 
    int id; 
    
    KDPoint();
    KDPoint(const array<double, KD_DIMS>& coords, int id);
    
    double operator[](size_t index) const;
    double& operator[](size_t index);
    
    bool operator<(const KDPoint& other) const;
    bool operator==(const KDPoint& other) const;
};

// Converts a Movie to the 5D  by the KD-Tree.
// coord[0]: Budget
// coord[1]: Revenue  
// coord[2]: Runtime
// coord[3]: Release Year
// coord[4]: Vote Average
KDPoint movieToKDPoint(const Movie& movie);

// Binary node: left/right split by one dimension that rotates with depth.
struct KDNode {
    KDPoint point; 
    KDNode* left;          
    KDNode* right;            

    KDNode(const KDPoint& pt);
};

// KD-Tree core: build once (balanced), allow dynamic edits, and run range queries.
class KDTree {
private:
    KDNode* root;

    // Insert point by walking down the tree and splitting on depth % KD_DIMS.
    KDNode* insertRecursive(KDNode* node, const KDPoint& point, int depth);
    // Find minimum along a dimension inside a subtree (used by delete).
    KDNode* findMinRecursive(KDNode* node, int dim, int depth) const;
    // Delete a point while preserving KD-Tree invariants.
    KDNode* deleteRecursive(KDNode* node, const KDPoint& point, int depth, bool& removed);
    // Find a point by ID (used by update: delete old -> insert new).
    bool findPointById(KDNode* node, int id, KDPoint& out) const;
    // Balanced build: choose median on current dimension with nth_element.
    KDNode* buildTreeRecursive(vector<KDPoint>& points, size_t begin, size_t end, int depth);
    
    // Range query: collect points inside an axis-aligned bounding box.
    void rangeQueryRecursive(KDNode* node, 
                            const array<double, KD_DIMS>& minBounds,
                            const array<double, KD_DIMS>& maxBounds,
                            vector<KDPoint>& results,
                            int depth) const;
    
    // Recursively free memory (post-order traversal).
    void destroyRecursive(KDNode* node);

public:
    KDTree();
    ~KDTree();

    // Dynamic operations ( supported in KD-Tree UI).
    void insert(const KDPoint& point);
    bool deleteNode(const KDPoint& point);
    bool updatePoint(int id, const KDPoint& newPt);

    // Build a balanced KD-Tree from the full dataset (O(n log n)).
    void build(const vector<Movie>& movies);
    
    // Range query returning KDPoints (raw coordinates + ID).
    vector<KDPoint> rangeQueryPoints(const array<double, KD_DIMS>& minBounds,
                                     const array<double, KD_DIMS>& maxBounds) const;

    // Range query returning Movie* (for LSH and printing).
    vector<Movie*> search(const array<double, KD_DIMS>& minBounds,
                          const array<double, KD_DIMS>& maxBounds,
                          const unordered_map<int, Movie*>& movieById) const;
};

#endif
