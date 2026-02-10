#include "quadtree.h"
#include <algorithm>

using namespace std;

// --- ISOLATION HELPER ---
// Convert a Movie into the 5D point used by the quadtree.
// We do this here instead of relying on Movie.h providing a helper so that
// the quadtree implementation stays self‑contained.
array<double, Q_K> getMovieCoords(const Movie* m) {
    return {
        m->budget, 
        m->revenue, 
        static_cast<double>(m->runtime), 
        static_cast<double>(m->release_year), 
        m->vote_average
    };
}

// Start with an empty index (no root node yet).
Quadtree::Quadtree() : root(nullptr) {}

// Recursively delete the root and all its children.
Quadtree::~Quadtree() {
    if (root) delete root;
}

// 1. Build a full tree from a static list of movies.
//    Existing contents (if any) are discarded.
void Quadtree::build(const vector<Movie>& movies) {
    if (movies.empty()) return;

    cout << "\n--- Building Quadtree (Hyperoctree) Index ---" << endl;

    QuadRect globalBounds;
    // Initialize global bounding box with the first movie,
    // then expand it to cover all movies.
    array<double, Q_K> firstPt = getMovieCoords(&movies[0]);
    globalBounds.min_coords = firstPt;
    globalBounds.max_coords = firstPt;

    // Find world bounds: min / max across all dimensions.
    for (const auto& m : movies) {
        array<double, Q_K> p = getMovieCoords(&m);
        for (int i = 0; i < Q_K; ++i) {
            if (p[i] < globalBounds.min_coords[i]) globalBounds.min_coords[i] = p[i];
            if (p[i] > globalBounds.max_coords[i]) globalBounds.max_coords[i] = p[i];
        }
    }

    // Add a tiny buffer so that points on the max edge still fall inside.
    for (int i = 0; i < Q_K; ++i) globalBounds.max_coords[i] += 0.001; 

    // Create the root node covering the entire data space.
    root = new QNode(globalBounds);

    // Insert each movie into the tree.
    for (const auto& m : movies) {
        insert(root, const_cast<Movie*>(&m));
    }
    cout << "Quadtree construction complete." << endl;
}

// 2. Public Insert (handles empty tree; delegates to internal insert).
void Quadtree::insert(Movie* movie) {
    array<double, Q_K> p = getMovieCoords(movie);

    // If this is the first insertion, create a degenerate root that we expand
    // just enough to contain the point.
    if (root == nullptr) {
        QuadRect bounds;
        bounds.min_coords = p;
        bounds.max_coords = p;
        for (int i = 0; i < Q_K; ++i) bounds.max_coords[i] += 0.001;
        root = new QNode(bounds);
    }

    // Delegate to the recursive insertion logic.
    insert(root, movie);
}

void Quadtree::insert(QNode* node, Movie* m) {
    // If we are at a leaf, just push the movie into the bucket and
    // split the node if it grows too large.
    if (node->isLeaf) {
        node->data.push_back(m);
        if (node->data.size() > Q_BUCKET_SIZE) split(node);
        return;
    }

    // Otherwise, figure out which child hyper‑octant the point belongs to.
    array<double, Q_K> p = getMovieCoords(m);
    int idx = getIndex(node->boundary, p);

    // Lazily create the child if it does not exist yet.
    if (node->children[idx] == nullptr) {
        QuadRect subBound = getSubBoundary(node->boundary, idx);
        node->children[idx] = new QNode(subBound);
    }
    insert(node->children[idx], m);
}

// 3. Split a leaf node into an internal node + children and reinsert its data.
void Quadtree::split(QNode* node) {
    node->isLeaf = false;
    // Reinsert all existing points into the appropriate children.
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
// Compute the child index (0 .. 2^K-1) for a point within a given boundary.
// Each bit of the index encodes whether the point is in the lower (0) or upper (1)
// half of that dimension relative to the midpoint.
int Quadtree::getIndex(const QuadRect& boundary, const array<double, Q_K>& point) {
    int index = 0;
    for (int i = 0; i < Q_K; ++i) {
        double mid = (boundary.min_coords[i] + boundary.max_coords[i]) / 2.0;
        if (point[i] >= mid) index |= (1 << i);
    }
    return index;
}

// Given a parent boundary and a child index, compute the child's bounding box.
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

// ==================== Remove ====================
// Removal is implemented as a recursive search that erases the movie pointer
// from the leaf bucket that contains it. We deliberately avoid complex
// node‑merging logic for now; empty internal nodes are allowed to persist.
bool Quadtree::removeInternal(QNode* node, int movieId) {
    if (!node) return false;

    if (node->isLeaf) {
        for (auto it = node->data.begin(); it != node->data.end(); ++it) {
            if ((*it) && (*it)->id == movieId) {
                node->data.erase(it);
                return true;
            }
        }
        return false;
    }

    // Otherwise, recurse into each non‑null child until we find and remove it.
    for (int i = 0; i < NUM_CHILDREN; ++i) {
        if (node->children[i] && removeInternal(node->children[i], movieId)) {
            return true;
        }
    }
    return false;
}

bool Quadtree::remove(int movieId) {
    if (!root) return false;
    return removeInternal(root, movieId);
}

// ==================== Update ====================
// Update is implemented as "remove, then insert the new record". This keeps
// the tree structure consistent even if the movie's coordinates change.
bool Quadtree::update(int movieId, Movie* newMovieData) {
    // Same pattern as R-Tree: delete + insert (coordinates may change)
    if (!remove(movieId)) return false;
    insert(newMovieData);
    return true;
}

// 5. Search
// Range query over the tree. Only subtrees whose bounding box intersects
// the query hyper‑rectangle are visited.
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
        // Internal node: recurse into any children whose bounding boxes
        // intersect the query (checked earlier via `intersects`).
        for (int i = 0; i < NUM_CHILDREN; ++i) {
            if (node->children[i]) searchRecursive(node->children[i], query, results);
        }
    }
}
