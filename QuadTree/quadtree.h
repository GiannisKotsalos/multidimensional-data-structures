#ifndef QUADTREE_H
#define QUADTREE_H

#include <vector>
#include <array>
#include <cmath>
#include <iostream>
#include "../Movie.h" 

// --- ISOLATION DEFINITIONS ---
// This file contains a 5D "quadtree" (technically a hyperoctree) that indexes movies
// by a fixed set of numeric attributes.  It is intentionally self‑contained so that
// it does not clash with other spatial index implementations (e.g. R-Tree, KD-Tree).

// Dimensions stored in each point:
//  [0] Budget
//  [1] Revenue
//  [2] Runtime
//  [3] Release Year
//  [4] Vote Average
const int Q_K = 5; 

// Maximum number of movies stored in a leaf before it is split into children.
const int Q_BUCKET_SIZE = 40;

// Each dimension is split into 2 halves -> in 5D we get 2^5 = 32 children.
const int NUM_CHILDREN = 32; // 2^5

// Unique name "QuadRect" so it doesn't clash with "Rect" in other files.
// Represents an axis-aligned bounding box in 5D space.
struct QuadRect {
    std::array<double, Q_K> min_coords;
    std::array<double, Q_K> max_coords;

    // Initialize the rectangle to an "empty" state that will expand when we
    // take mins / maxes over actual data.
    QuadRect() {
        for(int i=0; i<Q_K; i++) {
            min_coords[i] = 1e15;
            max_coords[i] = -1e15;
        }
    }
    
    // Check whether a point lies inside this rectangle (inclusive on bounds).
    bool contains(const std::array<double, Q_K>& p) const {
        for (int i = 0; i < Q_K; ++i) {
            if (p[i] < min_coords[i] || p[i] > max_coords[i]) return false;
        }
        return true;
    }

    // Check whether this rectangle intersects another one in all dimensions.
    bool intersects(const QuadRect& other) const {
        for (int i = 0; i < Q_K; ++i) {
            if (min_coords[i] > other.max_coords[i] || max_coords[i] < other.min_coords[i]) 
                return false;
        }
        return true;
    }
};

// Single node of the quadtree / hyperoctree.
//  - Internal nodes own up to NUM_CHILDREN pointers.
//  - Leaf nodes store a small bucket of Movie* in `data`.
struct QNode {
    bool isLeaf;
    QuadRect boundary;
    std::vector<Movie*> data;      // Only used when the node is a leaf.
    QNode* children[NUM_CHILDREN];

    // Create a leaf node covering the given bounding box.
    QNode(QuadRect bound) : isLeaf(true), boundary(bound) {
        for (int i = 0; i < NUM_CHILDREN; ++i) children[i] = nullptr;
    }

    // Recursively delete the entire subtree rooted at this node.
    ~QNode() {
        for (int i = 0; i < NUM_CHILDREN; ++i) if (children[i]) delete children[i];
    }
};

class Quadtree {
public:
    // Construct an empty quadtree (no root node yet).
    Quadtree();

    // Recursively destroys the root node and its children.
    ~Quadtree();

    // Build a fresh quadtree from a static list of movies.
    // Any existing tree is discarded and replaced.
    void build(const std::vector<Movie>& movies);

    // Dynamic operations (API roughly matches the R-Tree / KD-Tree):
    void insert(Movie* movie);
    bool remove(int movieId);
    bool update(int movieId, Movie* newMovieData);

    // Range query over the tree. Returns pointers to all movies whose
    // coordinates fall inside the `query` hyper-rectangle.
    std::vector<Movie*> search(const QuadRect& query);

private:
    // Root of the tree. nullptr means the index is currently empty.
    QNode* root;

    // Recursive helpers used by the public API.
    void insert(QNode* node, Movie* m);
    bool removeInternal(QNode* node, int movieId);
    void split(QNode* node);

    // Compute which of the 2^K children a point belongs to for a given boundary.
    int getIndex(const QuadRect& boundary, const std::array<double, Q_K>& point);

    // Return the sub‑boundary corresponding to a specific child index.
    QuadRect getSubBoundary(const QuadRect& original, int index);

    // Depth‑first traversal used to answer range queries.
    void searchRecursive(QNode* node, const QuadRect& query, std::vector<Movie*>& results);
};

#endif
