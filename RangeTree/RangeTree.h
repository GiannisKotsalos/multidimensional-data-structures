#ifndef RANGETREE_H
#define RANGETREE_H

#include "../Movie.h"
#include <vector>
#include <iostream>

// Dimensions: 0=Budget, 1=Revenue (Canonical). 
// Remaining dimensions are checked linearly.
#define RT_INDEX_DIMS 2 
#define RT_TOTAL_DIMS 5

struct RangeNode {
    Movie* movie;
    RangeNode *left, *right;
    RangeNode *nextDimTree; // Pointer to the root of the tree for the next dimension

    RangeNode(Movie* m) : movie(m), left(nullptr), right(nullptr), nextDimTree(nullptr) {}
};

class RangeTree {
public:
    RangeTree();
    ~RangeTree();

    void build(const vector<Movie>& movies);
    
    // --- New Dynamic Operations ---
    void insert(Movie* m);
    bool remove(int id);
    bool update(int id, Movie* m);

    vector<Movie*> search(const double minBounds[RT_TOTAL_DIMS], const double maxBounds[RT_TOTAL_DIMS]);

private:
    RangeNode* root;

    // Build
    RangeNode* buildRecursive(vector<Movie*>& movies, int dim);
    
    
    void deleteTree(RangeNode* node, int dim);
    double getVal(const Movie* m, int dim);
    
    // Search Helpers
    void searchBudgetTree(RangeNode* node, const double minBounds[], const double maxBounds[], vector<Movie*>& results);
    void searchRevenueTree(RangeNode* node, const double minBounds[], const double maxBounds[], vector<Movie*>& results);
    RangeNode* findSplitNode(RangeNode* node, double minVal, double maxVal, int dim);
    bool isMatchRest(Movie* m, const double minBounds[], const double maxBounds[]);

    // New Dynamic Implementation Helpers
    RangeNode* insertRecursive(RangeNode* node, Movie* m, int dim);
    RangeNode* removeRecursiveById(RangeNode* node, int id, int dim, bool &deleted);
    RangeNode* minValueNode(RangeNode* node);
};

#endif