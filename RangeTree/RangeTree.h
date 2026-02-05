#ifndef RANGETREE_H
#define RANGETREE_H

#include <vector>
#include <algorithm>
#include <iostream>
#include "../Movie.h"

using namespace std;

// Dimensions: 0:Budget, 1:Revenue, 2:Runtime, 3:Popularity, 4:Vote
const int RT_TOTAL_DIMS = 5;

// We index the first 2 dimensions (Budget, Revenue) using trees.
// The rest are filtered linearly.
const int RT_INDEX_DIMS = 2; 

struct RangeNode {
    Movie* movie;
    RangeNode* left;
    RangeNode* right;
    RangeNode* nextDimTree; // Root of the associative tree (Dimension + 1)

    RangeNode(Movie* m) : movie(m), left(nullptr), right(nullptr), nextDimTree(nullptr) {}
};

class RangeTree {
public:
    RangeTree();
    ~RangeTree();

    void build(const vector<Movie>& movies);
    vector<Movie*> search(const double minBounds[RT_TOTAL_DIMS], const double maxBounds[RT_TOTAL_DIMS]);

private:
    RangeNode* root;

    double getVal(const Movie* m, int dim);
    RangeNode* buildRecursive(vector<Movie*>& movies, int dim);
    void deleteTree(RangeNode* node, int dim);

    // --- New Search Logic Helpers ---

    // 1. Find the node where paths to minVal and maxVal diverge
    RangeNode* findSplitNode(RangeNode* node, double minVal, double maxVal, int dim);

    // 2. Search the 1D tree (Dimension 0) and switch to next dimension on canonical nodes
    void searchBudgetTree(RangeNode* node, const double minBounds[], const double maxBounds[], vector<Movie*>& results);

    // 3. Search the Associative Tree (Dimension 1 - Revenue)
    void searchRevenueTree(RangeNode* node, const double minBounds[], const double maxBounds[], vector<Movie*>& results);

    // 4. Verify the remaining un-indexed dimensions (Runtime, etc.)
    bool isMatchRest(Movie* m, const double minBounds[], const double maxBounds[]);
};

#endif