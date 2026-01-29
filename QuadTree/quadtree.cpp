#include "quadtree.h"
#include <algorithm>

using namespace std;

// --- ISOLATION HELPER ---
// Manually extracts coordinates so we don't rely on Movie.h having a helper
array<double, Q_K> getMovieCoords(const Movie* m) {
    return {
        m->budget, 
        m->revenue, 
        static_cast<double>(m->runtime), 
        m->popularity, 
        m->vote_average
    };
}

Quadtree::Quadtree() : root(nullptr) {}

Quadtree::~Quadtree() {
    if (root) delete root;
}

// 1. Build
void Quadtree::build(const vector<Movie>& movies) {
    if (movies.empty()) return;

    cout << "\n--- Building Quadtree (Hyperoctree) Index ---" << endl;

    QuadRect globalBounds;
    // Initialize with first movie
    array<double, Q_K> firstPt = getMovieCoords(&movies[0]);
    globalBounds.min_coords = firstPt;
    globalBounds.max_coords = firstPt;

    // Find World Bounds
    for (const auto& m : movies) {
        array<double, Q_K> p = getMovieCoords(&m);
        for (int i = 0; i < Q_K; ++i) {
            if (p[i] < globalBounds.min_coords[i]) globalBounds.min_coords[i] = p[i];
            if (p[i] > globalBounds.max_coords[i]) globalBounds.max_coords[i] = p[i];
        }
    }

    // Add buffer
    for (int i = 0; i < Q_K; ++i) globalBounds.max_coords[i] += 0.001; 

    root = new QNode(globalBounds);

    for (const auto& m : movies) {
        insert(root, const_cast<Movie*>(&m));
    }
    cout << "Quadtree construction complete." << endl;
}

// 2. Insert
void Quadtree::insert(QNode* node, Movie* m) {
    if (node->isLeaf) {
        node->data.push_back(m);
        if (node->data.size() > Q_BUCKET_SIZE) split(node);
        return;
    }

    array<double, Q_K> p = getMovieCoords(m);
    int idx = getIndex(node->boundary, p);

    if (node->children[idx] == nullptr) {
        QuadRect subBound = getSubBoundary(node->boundary, idx);
        node->children[idx] = new QNode(subBound);
    }
    insert(node->children[idx], m);
}

// 3. Split
void Quadtree::split(QNode* node) {
    node->isLeaf = false;
    for (Movie* m : node->data) {
        array<double, Q_K> p = getMovieCoords(m);
        int idx = getIndex(node->boundary, p);

        if (node->children[idx] == nullptr) {
            QuadRect subBound = getSubBoundary(node->boundary, idx);
            node->children[idx] = new QNode(subBound);
        }
        insert(node->children[idx], m);
    }
    node->data.clear();
}

// 4. Geometry Helpers
int Quadtree::getIndex(const QuadRect& boundary, const array<double, Q_K>& point) {
    int index = 0;
    for (int i = 0; i < Q_K; ++i) {
        double mid = (boundary.min_coords[i] + boundary.max_coords[i]) / 2.0;
        if (point[i] >= mid) index |= (1 << i);
    }
    return index;
}

QuadRect Quadtree::getSubBoundary(const QuadRect& original, int index) {
    QuadRect sub;
    for (int i = 0; i < Q_K; ++i) {
        double mid = (original.min_coords[i] + original.max_coords[i]) / 2.0;
        if ((index >> i) & 1) {
            sub.min_coords[i] = mid;
            sub.max_coords[i] = original.max_coords[i];
        } else {
            sub.min_coords[i] = original.min_coords[i];
            sub.max_coords[i] = mid;
        }
    }
    return sub;
}

// 5. Search
vector<Movie*> Quadtree::search(const QuadRect& query) {
    vector<Movie*> results;
    if (root) searchRecursive(root, query, results);
    return results;
}

void Quadtree::searchRecursive(QNode* node, const QuadRect& query, vector<Movie*>& results) {
    if (!node->boundary.intersects(query)) return;

    if (node->isLeaf) {
        for (Movie* m : node->data) {
            if (query.contains(getMovieCoords(m))) {
                results.push_back(m);
            }
        }
    } else {
        for (int i = 0; i < NUM_CHILDREN; ++i) {
            if (node->children[i]) searchRecursive(node->children[i], query, results);
        }
    }
}