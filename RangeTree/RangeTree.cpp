#include "RangeTree.h"
#include <algorithm>

RangeTree::RangeTree() : root(nullptr) {}

RangeTree::~RangeTree() {
    deleteTree(root, 0);
}

void RangeTree::deleteTree(RangeNode* node, int dim) {
    if (!node) return;
    deleteTree(node->left, dim);
    deleteTree(node->right, dim);
    if (dim < RT_INDEX_DIMS - 1) {
        deleteTree(node->nextDimTree, dim + 1);
    }
    delete node;
}

double RangeTree::getVal(const Movie* m, int dim) {
    switch(dim) {
        case 0: return m->budget;
        case 1: return m->revenue;
        case 2: return (double)m->runtime;
        case 3: return (double)m->release_year;
        case 4: return m->vote_average;
        default: return 0.0;
    }
}

void RangeTree::build(const vector<Movie>& movies) {
    vector<Movie*> moviePtrs;
    moviePtrs.reserve(movies.size());
    for (const auto& m : movies) moviePtrs.push_back(const_cast<Movie*>(&m));

    cout << "\n--- Building Range Tree (2D Canonical Index) ---" << endl;
    root = buildRecursive(moviePtrs, 0);
    cout << "Range Tree construction complete." << endl;
}

RangeNode* RangeTree::buildRecursive(vector<Movie*>& movies, int dim) {
    if (movies.empty()) return nullptr;

    // 1. Sort by current dimension
    sort(movies.begin(), movies.end(), [this, dim](Movie* a, Movie* b) {
        return getVal(a, dim) < getVal(b, dim);
    });

    // 2. Build BST
    int mid = movies.size() / 2;
    RangeNode* node = new RangeNode(movies[mid]);

    vector<Movie*> leftSubset(movies.begin(), movies.begin() + mid);
    vector<Movie*> rightSubset(movies.begin() + mid + 1, movies.end());

    node->left = buildRecursive(leftSubset, dim);
    node->right = buildRecursive(rightSubset, dim);

    // 3. Build Associative Tree (Only for Dimension 0 -> 1)
    if (dim < RT_INDEX_DIMS - 1) {
        // Must sort copy of ALL movies in this subtree by the *next* dimension
        vector<Movie*> nextDimMovies = movies; 
        node->nextDimTree = buildRecursive(nextDimMovies, dim + 1);
    }

    return node;
}

// --- Dynamic Operations Implementation ---

void RangeTree::insert(Movie* m) {
    if (!m) return;
    root = insertRecursive(root, m, 0);
}

RangeNode* RangeTree::insertRecursive(RangeNode* node, Movie* m, int dim) {
    // 1. Base Case: Create new node
    if (!node) {
        RangeNode* newNode = new RangeNode(m);
        // If we are in the primary dimension (0), we must create the associative tree (1)
        if (dim < RT_INDEX_DIMS - 1) {
            newNode->nextDimTree = insertRecursive(nullptr, m, dim + 1);
        }
        return newNode;
    }

    // 2. Maintain Associative Structure (If Dim 0)
    // Every node in the primary tree effectively "contains" the point in its range,
    // so we must insert the point into the associative tree of the current node.
    if (dim < RT_INDEX_DIMS - 1) {
        node->nextDimTree = insertRecursive(node->nextDimTree, m, dim + 1);
    }

    // 3. Standard BST Insert
    double mVal = getVal(m, dim);
    double nodeVal = getVal(node->movie, dim);

    if (mVal < nodeVal) {
        node->left = insertRecursive(node->left, m, dim);
    } else {
        node->right = insertRecursive(node->right, m, dim);
    }

    return node;
}

bool RangeTree::remove(int id) {
    bool deleted = false;
    root = removeRecursiveById(root, id, 0, deleted);
    return deleted;
}

// Recursively removes node by ID.
// Note: We search by ID (O(N)) instead of Value because update operations might 
// have changed the movie values in-place, rendering the BST structure invalid for search.
RangeNode* RangeTree::removeRecursiveById(RangeNode* node, int id, int dim, bool &deleted) {
    if (!node) return nullptr;

    // 1. Maintain Associative Structure (If Dim 0)
    // We must remove the movie from the associative tree of every ancestor it belongs to.
    if (dim < RT_INDEX_DIMS - 1 && node->nextDimTree) {
        bool subDeleted = false; // dummy
        node->nextDimTree = removeRecursiveById(node->nextDimTree, id, dim + 1, subDeleted);
    }

    // 2. Check if THIS is the node to delete
    if (node->movie->id == id) {
        deleted = true;

        // Case A: Leaf or Single Child
        if (!node->left) {
            RangeNode* temp = node->right;
            // Clean up associative tree if it exists
            if (dim < RT_INDEX_DIMS - 1 && node->nextDimTree) deleteTree(node->nextDimTree, dim + 1);
            delete node;
            return temp;
        } 
        else if (!node->right) {
            RangeNode* temp = node->left;
            if (dim < RT_INDEX_DIMS - 1 && node->nextDimTree) deleteTree(node->nextDimTree, dim + 1);
            delete node;
            return temp;
        }

        // Case B: Two Children
        // Get inorder successor (smallest in the right subtree)
        RangeNode* temp = minValueNode(node->right);

        // Copy the successor's data to this node
        node->movie = temp->movie;
        
        // Delete the inorder successor
        bool dummy = false;
        node->right = removeRecursiveById(node->right, temp->movie->id, dim, dummy);
    } 
    else {
        // 3. Search Children (Traverse All, as BST property might be invalid for ID search)
        // We must attempt both sides because we are searching by ID, not Value.
        node->left = removeRecursiveById(node->left, id, dim, deleted);
        node->right = removeRecursiveById(node->right, id, dim, deleted);
    }

    return node;
}

bool RangeTree::update(int id, Movie* m) {
    // To handle updates robustly (especially if 'm' was modified in-place),
    // we first remove the old entry by ID, then insert the new one.
    bool removed = remove(id);
    if (removed) {
        insert(m);
        return true;
    }
    return false;
}

RangeNode* RangeTree::minValueNode(RangeNode* node) {
    RangeNode* current = node;
    while (current && current->left != nullptr)
        current = current->left;
    return current;
}

// --- Search Implementation ---

vector<Movie*> RangeTree::search(const double minBounds[RT_TOTAL_DIMS], const double maxBounds[RT_TOTAL_DIMS]) {
    vector<Movie*> results;
    // Start the process at the Main Tree (Budget - Dim 0)
    searchBudgetTree(root, minBounds, maxBounds, results);
    return results;
}

RangeNode* RangeTree::findSplitNode(RangeNode* node, double minVal, double maxVal, int dim) {
    while (node) {
        double val = getVal(node->movie, dim);
        // If node value is greater than max, go left (target range is to the left)
        if (val > maxVal) {
            node = node->left;
        } 
        // If node value is less than min, go right (target range is to the right)
        else if (val < minVal) {
            node = node->right;
        } 
        // Otherwise, we found the split! node is between min and max (inclusive)
        else {
            break;
        }
    }
    return node;
}

// PRIMARY DIMENSION SEARCH (BUDGET)
// Finds "Canonical Nodes" and delegates to Associative Tree (Revenue)
void RangeTree::searchBudgetTree(RangeNode* node, const double minBounds[], const double maxBounds[], vector<Movie*>& results) {
    // 1. Find Split Node for Budget
    RangeNode* split = findSplitNode(node, minBounds[0], maxBounds[0], 0);
    if (!split) return;

    // 2. Check the split node itself
    // It matches Budget (by definition of split), so check Revenue + Rest
    double splitRevenue = getVal(split->movie, 1);
    if (splitRevenue >= minBounds[1] && splitRevenue <= maxBounds[1]) {
        if (isMatchRest(split->movie, minBounds, maxBounds)) {
            results.push_back(split->movie);
        }
    }

    // 3. Traverse Left Path (from split->left down to minVal)
    RangeNode* curr = split->left;
    while (curr) {
        double val = getVal(curr->movie, 0);
        if (val >= minBounds[0]) {
            // MATCH: The current node is >= min. 
            // Therefore, its RIGHT subtree is completely inside [min, split].
            // We verify the Right Child's ASSOCIATIVE tree (Revenue).
            if (curr->right) {
                searchRevenueTree(curr->right->nextDimTree, minBounds, maxBounds, results);
            }
            
            // Check the current node itself
            double rev = getVal(curr->movie, 1);
            if (rev >= minBounds[1] && rev <= maxBounds[1]) {
                if (isMatchRest(curr->movie, minBounds, maxBounds)) results.push_back(curr->movie);
            }
            
            // Move Left to find smaller values that might still be >= min
            curr = curr->left;
        } else {
            // Too small, move right
            curr = curr->right;
        }
    }

    // 4. Traverse Right Path (from split->right down to maxVal)
    curr = split->right;
    while (curr) {
        double val = getVal(curr->movie, 0);
        if (val <= maxBounds[0]) {
            // MATCH: The current node is <= max.
            // Therefore, its LEFT subtree is completely inside [split, max].
            // We verify the Left Child's ASSOCIATIVE tree (Revenue).
            if (curr->left) {
                searchRevenueTree(curr->left->nextDimTree, minBounds, maxBounds, results);
            }

            // Check the current node itself
            double rev = getVal(curr->movie, 1);
            if (rev >= minBounds[1] && rev <= maxBounds[1]) {
                if (isMatchRest(curr->movie, minBounds, maxBounds)) results.push_back(curr->movie);
            }

            // Move Right to find larger values that might still be <= max
            curr = curr->right;
        } else {
            // Too big, move left
            curr = curr->left;
        }
    }
}

// SECONDARY DIMENSION SEARCH (REVENUE)
// Standard 1D Range Search on the associative tree
void RangeTree::searchRevenueTree(RangeNode* node, const double minBounds[], const double maxBounds[], vector<Movie*>& results) {
    if (!node) return;

    double val = getVal(node->movie, 1); // Dimension 1: Revenue
    double minV = minBounds[1];
    double maxV = maxBounds[1];

    if (val >= minV) {
        searchRevenueTree(node->left, minBounds, maxBounds, results);
    }
    if (val <= maxV) {
        searchRevenueTree(node->right, minBounds, maxBounds, results);
    }

    // If matches Revenue...
    if (val >= minV && val <= maxV) {
        // ... Check the remaining linear dimensions (Runtime, etc)
        if (isMatchRest(node->movie, minBounds, maxBounds)) {
            results.push_back(node->movie);
        }
    }
}

// LINEAR FILTER (Dimensions 2, 3, 4)
bool RangeTree::isMatchRest(Movie* m, const double minBounds[], const double maxBounds[]) {
    // We already checked 0 (Budget) and 1 (Revenue) via the trees.
    // Now check 2 (Runtime), 3 (Release Year), 4 (Vote).
    for (int i = 2; i < RT_TOTAL_DIMS; i++) {
        double v = getVal(m, i);
        if (v < minBounds[i] || v > maxBounds[i]) return false;
    }
    return true;
}