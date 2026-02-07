#ifndef QUADTREE_H
#define QUADTREE_H

#include <vector>
#include <array>
#include <cmath>
#include <iostream>
#include "../Movie.h" 

// --- ISOLATION DEFINITIONS ---
// We define our own constants and structs to avoid conflicts with R-Tree

// Dimensions (Budget, Revenue, Runtime, Release Year, VoteAvg)
const int Q_K = 5; 
const int Q_BUCKET_SIZE = 40;
const int NUM_CHILDREN = 32; // 2^5

// Unique name "QuadRect" so it doesn't clash with "Rect" in other files
struct QuadRect {
    std::array<double, Q_K> min_coords;
    std::array<double, Q_K> max_coords;

    QuadRect() {
        for(int i=0; i<Q_K; i++) {
            min_coords[i] = 1e15;
            max_coords[i] = -1e15;
        }
    }
    
    // Check if point is inside
    bool contains(const std::array<double, Q_K>& p) const {
        for (int i = 0; i < Q_K; ++i) {
            if (p[i] < min_coords[i] || p[i] > max_coords[i]) return false;
        }
        return true;
    }

    // Check intersection with another QuadRect
    bool intersects(const QuadRect& other) const {
        for (int i = 0; i < Q_K; ++i) {
            if (min_coords[i] > other.max_coords[i] || max_coords[i] < other.min_coords[i]) 
                return false;
        }
        return true;
    }
};

struct QNode {
    bool isLeaf;
    QuadRect boundary;
    std::vector<Movie*> data;
    QNode* children[NUM_CHILDREN];

    QNode(QuadRect bound) : isLeaf(true), boundary(bound) {
        for (int i = 0; i < NUM_CHILDREN; ++i) children[i] = nullptr;
    }

    ~QNode() {
        for (int i = 0; i < NUM_CHILDREN; ++i) if (children[i]) delete children[i];
    }
};

class Quadtree {
public:
    Quadtree();
    ~Quadtree();

    void build(const std::vector<Movie>& movies);

    // Dynamic operations (matching R-Tree / KD-Tree API)
    void insert(Movie* movie);
    bool remove(int movieId);
    bool update(int movieId, Movie* newMovieData);

    // Search now takes our unique "QuadRect"
    std::vector<Movie*> search(const QuadRect& query);

private:
    QNode* root;

    void insert(QNode* node, Movie* m);
    bool removeInternal(QNode* node, int movieId);
    void split(QNode* node);
    int getIndex(const QuadRect& boundary, const std::array<double, Q_K>& point);
    QuadRect getSubBoundary(const QuadRect& original, int index);
    void searchRecursive(QNode* node, const QuadRect& query, std::vector<Movie*>& results);
};

#endif
