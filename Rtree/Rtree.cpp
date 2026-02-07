#include "Rtree.h"
#include <iostream>
#include <string>

RTree::RTree() : root(new Node(false)) { 
    // Initialize root as an internal node (container)
}

RTree::~RTree() {
    destroyRecursive(root);
}

void RTree::destroyRecursive(Node* node) {
    if (!node) return;
    for (Node* child : node->children) {
        destroyRecursive(child);
    }
    delete node;
}

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
    
    // Clear existing tree if any
    if (root) {
        destroyRecursive(root);
        root = new Node(false);
    }

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
    // Note: In a full R-Tree, we would traverse to find the best leaf and split if necessary.
    // For this implementation, we append to root's children (Flat R-Tree / BVH style).
    root->children.push_back(leaf);
    root->mbr.expand(coords);
}

// ==================== Delete ====================

// Helper to recalculate a node's MBR based on its children
void RTree::recalculateMBR(Node* node) {
    if (node->isLeaf) return;
    
    // Reset MBR
    node->mbr = Rect(); 
    
    for (Node* child : node->children) {
        // Expand to include child's MBR
        for (int i = 0; i < K; ++i) {
            node->mbr.min_coords[i] = min(node->mbr.min_coords[i], child->mbr.min_coords[i]);
            node->mbr.max_coords[i] = max(node->mbr.max_coords[i], child->mbr.max_coords[i]);
        }
    }
}

bool RTree::removeInternal(Node* node, int movieId) {
    if (node->isLeaf) return false;

    for (auto it = node->children.begin(); it != node->children.end(); ++it) {
        Node* child = *it;
        
        // If child is a leaf node containing the movie
        if (child->isLeaf && child->moviePtr && child->moviePtr->id == movieId) {
            delete child; // Free memory
            node->children.erase(it); // Remove from vector
            recalculateMBR(node); // Update this node's bounds
            return true;
        }
        
        // If child is an internal node, recurse
        if (!child->isLeaf) {
            if (removeInternal(child, movieId)) {
                recalculateMBR(node); // Update bounds on the way up
                return true;
            }
        }
    }
    return false;
}

bool RTree::remove(int movieId) {
    return removeInternal(root, movieId);
}

// ==================== Update ====================

bool RTree::update(int movieId, Movie* newMovieData) {
    // R-Tree update is typically Delete + Insert because moving 
    // a point in space might require it to move to a different branch.
    
    // 1. Remove old entry
    if (!remove(movieId)) {
        return false; 
    }
    
    // 2. Insert new entry
    insert(newMovieData);
    return true;
}

// ==================== Search ====================

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