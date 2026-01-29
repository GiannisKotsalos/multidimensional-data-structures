#include "KDTree.h"
#include <iostream>

// ==================== KDPoint Implementation ====================

KDPoint::KDPoint() : id(-1) {
    for (int i = 0; i < KD_DIMS; i++) {
        coordinates[i] = 0.0;
    }
}

KDPoint::KDPoint(const array<double, KD_DIMS>& coords, int id) 
    : coordinates(coords), id(id) {}

double KDPoint::operator[](size_t index) const { 
    return coordinates[index]; 
}

double& KDPoint::operator[](size_t index) { 
    return coordinates[index]; 
}

bool KDPoint::operator<(const KDPoint& other) const {
    return id < other.id;
}

bool KDPoint::operator==(const KDPoint& other) const {
    if (id != other.id) return false;
    for (int i = 0; i < KD_DIMS; i++) {
        if (coordinates[i] != other.coordinates[i]) return false;
    }
    return true;
}

// Convert Movie to KDPoint
KDPoint movieToKDPoint(const Movie& movie) {
    array<double, KD_DIMS> coords;
    coords[0] = movie.budget;
    coords[1] = movie.revenue;
    coords[2] = static_cast<double>(movie.runtime);
    coords[3] = movie.popularity;
    coords[4] = movie.vote_average;
    
    return KDPoint(coords, movie.id);
}

// ==================== KDNode Implementation ====================

KDNode::KDNode(const KDPoint& pt) : point(pt), left(nullptr), right(nullptr) {}

// ==================== KDTree Implementation ====================

KDTree::KDTree() : root(nullptr) {}

KDTree::~KDTree() {
    destroyRecursive(root);
}

void KDTree::destroyRecursive(KDNode* node) {
    if (node != nullptr) {
        destroyRecursive(node->left);
        destroyRecursive(node->right);
        delete node;
    }
}

size_t KDTree::sizeRecursive(KDNode* node) const {
    if (node == nullptr) return 0;
    return 1 + sizeRecursive(node->left) + sizeRecursive(node->right);
}

size_t KDTree::size() const {
    return sizeRecursive(root);
}

bool KDTree::empty() const {
    return root == nullptr;
}

// ==================== Insert ====================

KDNode* KDTree::insertRecursive(KDNode* node, const KDPoint& point, int depth) {
    if (node == nullptr) return new KDNode(point);

    int cd = depth % KD_DIMS;

    if (point[cd] < node->point[cd])
        node->left = insertRecursive(node->left, point, depth + 1);
    else
        node->right = insertRecursive(node->right, point, depth + 1);

    return node;
}

void KDTree::insert(const KDPoint& point) {
    root = insertRecursive(root, point, 0);
}

// ==================== Search ====================

bool KDTree::searchRecursive(KDNode* node, const KDPoint& point, int depth) const {
    if (node == nullptr) return false;
    if (node->point == point) return true;

    int cd = depth % KD_DIMS;

    if (point[cd] < node->point[cd])
        return searchRecursive(node->left, point, depth + 1);
    else
        return searchRecursive(node->right, point, depth + 1);
}

bool KDTree::search(const KDPoint& point) const {
    return searchRecursive(root, point, 0);
}

// ==================== Find Minimum ====================

KDNode* KDTree::findMinRecursive(KDNode* node, int dim, int depth) const {
    if (node == nullptr) return nullptr;

    int cd = depth % KD_DIMS;

    if (cd == dim) {
        if (node->left == nullptr) return node;
        return findMinRecursive(node->left, dim, depth + 1);
    }

    KDNode* minNode = node;
    KDNode* leftMin = findMinRecursive(node->left, dim, depth + 1);
    KDNode* rightMin = findMinRecursive(node->right, dim, depth + 1);

    if (leftMin != nullptr && leftMin->point[dim] < minNode->point[dim])
        minNode = leftMin;
    if (rightMin != nullptr && rightMin->point[dim] < minNode->point[dim])
        minNode = rightMin;

    return minNode;
}

// ==================== Delete ====================

KDNode* KDTree::deleteRecursive(KDNode* node, const KDPoint& point, int depth) {
    if (node == nullptr) return nullptr;

    int cd = depth % KD_DIMS;

    if (node->point == point) {
        if (node->right != nullptr) {
            KDNode* minNode = findMinRecursive(node->right, cd, depth + 1);
            node->point = minNode->point;
            node->right = deleteRecursive(node->right, minNode->point, depth + 1);
        }
        else if (node->left != nullptr) {
            KDNode* minNode = findMinRecursive(node->left, cd, depth + 1);
            node->point = minNode->point;
            node->right = node->left;
            node->left = nullptr;
            node->right = deleteRecursive(node->right, minNode->point, depth + 1);
        }
        else {
            delete node;
            return nullptr;
        }
        return node;
    }

    if (point[cd] < node->point[cd])
        node->left = deleteRecursive(node->left, point, depth + 1);
    else
        node->right = deleteRecursive(node->right, point, depth + 1);

    return node;
}

bool KDTree::remove(const KDPoint& point) {
    if (root == nullptr) return false;
    size_t sizeBefore = size();
    root = deleteRecursive(root, point, 0);
    return size() < sizeBefore;
}

// ==================== Build Balanced Tree ====================

KDNode* KDTree::buildTreeRecursive(vector<KDPoint>& points, size_t begin, size_t end, int depth) {
    if (begin >= end) return nullptr;

    size_t mid = begin + (end - begin) / 2;
    int cd = depth % KD_DIMS;

    nth_element(points.begin() + begin, points.begin() + mid, points.begin() + end,
                [cd](const KDPoint& a, const KDPoint& b) {
                    if (a[cd] == b[cd]) return a.id < b.id;
                    return a[cd] < b[cd];
                });

    KDNode* node = new KDNode(points[mid]);
    node->left = buildTreeRecursive(points, begin, mid, depth + 1);
    node->right = buildTreeRecursive(points, mid + 1, end, depth + 1);
    return node;
}

void KDTree::build(const vector<Movie>& movies) {
    cout << "\n--- Building KD-Tree Index ---" << endl;
    
    // Destroy existing tree
    destroyRecursive(root);
    root = nullptr;
    
    // Convert movies to points
    vector<KDPoint> points;
    points.reserve(movies.size());
    for (const auto& movie : movies) {
        points.push_back(movieToKDPoint(movie));
    }
    
    // Build balanced tree
    root = buildTreeRecursive(points, 0, points.size(), 0);
    
    cout << "KD-Tree construction complete. Indexed " << movies.size() << " movies." << endl;
}

// ==================== Range Query ====================

void KDTree::rangeQueryRecursive(KDNode* node, 
                                  const array<double, KD_DIMS>& minBounds,
                                  const array<double, KD_DIMS>& maxBounds,
                                  vector<KDPoint>& results,
                                  int depth) const {
    if (node == nullptr) return;

    int cd = depth % KD_DIMS;
    bool inRange = true;

    // Check if current point is within the range
    for (int i = 0; i < KD_DIMS; i++) {
        if (node->point[i] < minBounds[i] || node->point[i] > maxBounds[i]) {
            inRange = false;
            break;
        }
    }

    if (inRange) {
        results.push_back(node->point);
    }

    // Check if we need to search left subtree
    if (minBounds[cd] <= node->point[cd]) {
        rangeQueryRecursive(node->left, minBounds, maxBounds, results, depth + 1);
    }

    // Check if we need to search right subtree
    if (maxBounds[cd] >= node->point[cd]) {
        rangeQueryRecursive(node->right, minBounds, maxBounds, results, depth + 1);
    }
}

vector<KDPoint> KDTree::rangeQueryPoints(const array<double, KD_DIMS>& minBounds,
                                          const array<double, KD_DIMS>& maxBounds) const {
    vector<KDPoint> results;
    rangeQueryRecursive(root, minBounds, maxBounds, results, 0);
    return results;
}

vector<Movie*> KDTree::search(const array<double, KD_DIMS>& minBounds,
                               const array<double, KD_DIMS>& maxBounds,
                               const unordered_map<int, Movie*>& movieById) const {
    vector<KDPoint> points = rangeQueryPoints(minBounds, maxBounds);
    vector<Movie*> results;
    results.reserve(points.size());
    
    for (const auto& point : points) {
        auto it = movieById.find(point.id);
        if (it != movieById.end()) {
            results.push_back(it->second);
        }
    }
    return results;
}
