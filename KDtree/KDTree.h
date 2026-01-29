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

// Point structure for KD-Tree
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

// Convert Movie to KDPoint
// coord[0]: Budget
// coord[1]: Revenue  
// coord[2]: Runtime
// coord[3]: Popularity
// coord[4]: Vote Average
KDPoint movieToKDPoint(const Movie& movie);

// Node structure for KD-Tree
struct KDNode {
    KDPoint point; 
    KDNode* left;          
    KDNode* right;            

    KDNode(const KDPoint& pt);
};

// KD-Tree class
class KDTree {
private:
    KDNode* root;

    KDNode* insertRecursive(KDNode* node, const KDPoint& point, int depth);
    bool searchRecursive(KDNode* node, const KDPoint& point, int depth) const;
    KDNode* findMinRecursive(KDNode* node, int dim, int depth) const;
    KDNode* buildTreeRecursive(vector<KDPoint>& points, size_t begin, size_t end, int depth);
    KDNode* deleteRecursive(KDNode* node, const KDPoint& point, int depth);
    
    void rangeQueryRecursive(KDNode* node, 
                            const array<double, KD_DIMS>& minBounds,
                            const array<double, KD_DIMS>& maxBounds,
                            vector<KDPoint>& results,
                            int depth) const;
    
    void destroyRecursive(KDNode* node);
    size_t sizeRecursive(KDNode* node) const;

public:
    KDTree();
    ~KDTree();

    void insert(const KDPoint& point);
    bool search(const KDPoint& point) const;
    bool remove(const KDPoint& point);
    
    size_t size() const;
    bool empty() const;

    // Build balanced tree from vector of points
    void build(const vector<Movie>& movies);
    
    // Range query returning points
    vector<KDPoint> rangeQueryPoints(const array<double, KD_DIMS>& minBounds,
                                     const array<double, KD_DIMS>& maxBounds) const;

    // Range query returning Movie pointers (for LSH phase)
    vector<Movie*> search(const array<double, KD_DIMS>& minBounds,
                          const array<double, KD_DIMS>& maxBounds,
                          const unordered_map<int, Movie*>& movieById) const;
};

#endif
