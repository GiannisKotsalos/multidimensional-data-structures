// C++ Program to Implement KD Tree
#include <iostream>
#include <array>
#include <cmath>
#include <vector>
#include <queue>
#include <limits>
#include <algorithm>
#include <string>
#include <sstream>
#include <fstream>
#include <cctype>
#include <chrono>
#include <unordered_map>
using namespace std;

// Movie struct matching FileReader.cpp
struct Movie {
    int id;
    std::string title;
    bool adult;
    std::string original_language;
    std::vector<std::string> origin_country;
    std::string release_date;
    std::vector<std::string> genre_names;
    std::vector<std::string> production_company_names;
    double budget;
    double revenue;
    int runtime;
    double popularity;
    double vote_average;
    int vote_count;
};

// Template class for KDTree with K dimensions
template <size_t K>
struct Point {
    array<double, K> coordinates; 
    int id; 
    
    Point() : id(-1) {}
    Point(const array<double, K>& coords, int id) : coordinates(coords), id(id) {}
    
    // Access coordinates like an array
    double operator[](size_t index) const { return coordinates[index]; }
    double& operator[](size_t index) { return coordinates[index]; }
    
    // Comparison operator for ordering by id
    bool operator<(const Point& other) const {
        return id < other.id;
    }
    
    // Equality operator for comparing points
    bool operator==(const Point& other) const {
        if (id != other.id) return false;
        for (size_t i = 0; i < K; i++) {
            if (coordinates[i] != other.coordinates[i]) return false;
        }
        return true;
    }
};

// Helper function to extract year from release_date string (format: "YYYY-MM-DD" or "YYYY")
int extractYear(const string& release_date) {
    if (release_date.empty()) return 0;
    
    // Try to extract year (first 4 digits)
    string yearStr;
    for (char c : release_date) {
        if (isdigit(c)) {
            yearStr += c;
            if (yearStr.length() == 4) break;
        } else if (!yearStr.empty()) {
            break; // Non-digit after digits
        }
    }
    
    if (yearStr.length() == 4) {
        return stoi(yearStr);
    }
    return 0;
}

// Convert Movie to Point<5> using the specified dimensions
// coord[0]: Release Year (Extracted from release_date)
// coord[1]: Popularity (TMDB score)
// coord[2]: Vote Average (Rating)
// coord[3]: Runtime (Duration in minutes)
// coord[4]: Budget (Production budget in USD)
Point<5> movieToPoint(const Movie& movie) {
    array<double, 5> coords;
    coords[0] = static_cast<double>(extractYear(movie.release_date));  // Dimension 0: Release Year
    coords[1] = movie.popularity;                                       // Dimension 1: Popularity
    coords[2] = movie.vote_average;                                     // Dimension 2: Vote Average
    coords[3] = static_cast<double>(movie.runtime);                     // Dimension 3: Runtime (minutes)
    coords[4] = movie.budget;                                           // Dimension 4: Budget (USD)
    
    return Point<5>(coords, movie.id);
}

// Helper function to get movie by ID from vector
Movie* findMovieById(vector<Movie>& movies, int id) {
    for (auto& movie : movies) {
        if (movie.id == id) return &movie;
    }
    return nullptr;
}

// FileReader functions from FileReader.cpp
vector<string> parseCSVRow(const string &line) {
    vector<string> fields;
    string cur;
    bool inQuotes = false;

    for (char c : line) {
        if (c == '\"') {
            inQuotes = !inQuotes; 
        } 
        else if (c == ',' && !inQuotes) {
            fields.push_back(cur);
            cur.clear();
        }
        else {
            cur.push_back(c);
        }
    }
    fields.push_back(cur);
    return fields;
}

vector<string> parseList(const string &s) {
    vector<string> result;
    string temp;
    bool inWord = false;

    for (char c : s) {
        if (isalnum(c)) {
            temp.push_back(c);
            inWord = true;
        } else {
            if (inWord) {
                result.push_back(temp);
                temp.clear();
            }
            inWord = false;
        }
    }
    if (!temp.empty()) result.push_back(temp);

    return result;
}

vector<Movie> loadMoviesFromCSV(const string &filename) {
    vector<Movie> movies;
    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << "\n";
        return movies;
    }

    string line;
    getline(file, line); // skip header

    while (getline(file, line)) {
        if (line.empty()) continue;

        auto fields = parseCSVRow(line);
        if (fields.size() != 14) {
            cerr << "Skipping malformed row (expected 14 fields, got " << fields.size() << ")\n";
            continue;
        }

        Movie m;
        int i = 0;

        m.id = stoi(fields[i++]);
        m.title = fields[i++];
        m.adult = (fields[i++] == "TRUE" || fields[i-1] == "true");
        m.original_language = fields[i++];
        m.origin_country = parseList(fields[i++]);
        m.release_date = fields[i++];
        m.genre_names = parseList(fields[i++]);
        m.production_company_names = parseList(fields[i++]);
        m.budget = stod(fields[i++]);
        m.revenue = stod(fields[i++]);
        m.runtime = stoi(fields[i++]);
        m.popularity = stod(fields[i++]);
        m.vote_average = stod(fields[i++]);
        m.vote_count = stoi(fields[i++]);

        movies.push_back(m);
    }

    return movies;
}

template <size_t K>
class KDTree {
    
private:
    // Node structure representing each point in the KDTree
    struct Node {
        // Point in K dimensions with id
        Point<K> point; 
        // Pointer to left child
        Node* left;          
        // Pointer to right child
        Node* right;            

        // Constructor to initialize a Node
        Node(const Point<K>& pt) : point(pt), left(nullptr), right(nullptr) {}
    };

    Node* root; // Root of the KDTree

    // Recursive function to insert a point into the KDTree
    Node* insertRecursive(Node* node, const Point<K>& point, int depth) {
        // Base case: If node is null, create a new node
        if (node == nullptr) return new Node(point);

        // Calculate current dimension (cd)
        int cd = depth % K;

        // Compare point with current node and decide to go left or right
        if (point[cd] < node->point[cd])
            node->left = insertRecursive(node->left, point, depth + 1);
        else
            node->right = insertRecursive(node->right, point, depth + 1);

        return node;
    }

    // Recursive function to search for a point in the KDTree
    bool searchRecursive(Node* node, const Point<K>& point, int depth) const {
        // Base case: If node is null, the point is not found
        if (node == nullptr) return false;

        // If the current node matches the point, return true
        if (node->point == point) return true;

        // Calculate current dimension (cd)
        int cd = depth % K;

        // Compare point with current node and decide to go left or right
        if (point[cd] < node->point[cd])
            return searchRecursive(node->left, point, depth + 1);
        else
            return searchRecursive(node->right, point, depth + 1);
    }

    // Helper function to find minimum node in a subtree along a given dimension
    Node* findMinRecursive(Node* node, int dim, int depth) const {
        if (node == nullptr) return nullptr;

        int cd = depth % K;

        // If current dimension matches, search left subtree
        if (cd == dim) {
            if (node->left == nullptr) return node;
            return findMinRecursive(node->left, dim, depth + 1);
        }

        // Otherwise, search both subtrees and compare
        Node* minNode = node;
        Node* leftMin = findMinRecursive(node->left, dim, depth + 1);
        Node* rightMin = findMinRecursive(node->right, dim, depth + 1);

        if (leftMin != nullptr && leftMin->point[dim] < minNode->point[dim])
            minNode = leftMin;
        if (rightMin != nullptr && rightMin->point[dim] < minNode->point[dim])
            minNode = rightMin;

        return minNode;
    }

    // Helper function to find maximum node in a subtree along a given dimension
    Node* findMaxRecursive(Node* node, int dim, int depth) const {
        if (node == nullptr) return nullptr;

        int cd = depth % K;

        // If current dimension matches, search right subtree
        if (cd == dim) {
            if (node->right == nullptr) return node;
            return findMaxRecursive(node->right, dim, depth + 1);
        }

        // Otherwise, search both subtrees and compare
        Node* maxNode = node;
        Node* leftMax = findMaxRecursive(node->left, dim, depth + 1);
        Node* rightMax = findMaxRecursive(node->right, dim, depth + 1);

        if (leftMax != nullptr && leftMax->point[dim] > maxNode->point[dim])
            maxNode = leftMax;
        if (rightMax != nullptr && rightMax->point[dim] > maxNode->point[dim])
            maxNode = rightMax;

        return maxNode;
    }

    // Build a balanced KDTree from a range of points using nth_element
    Node* buildTreeRecursive(vector<Point<K>>& points, size_t begin, size_t end, int depth) {
        if (begin >= end) return nullptr;

        size_t mid = begin + (end - begin) / 2;
        int cd = depth % K;

        nth_element(points.begin() + begin, points.begin() + mid, points.begin() + end,
                    [cd](const Point<K>& a, const Point<K>& b) {
                        if (a[cd] == b[cd]) return a.id < b.id;
                        return a[cd] < b[cd];
                    });

        Node* node = new Node(points[mid]);
        node->left = buildTreeRecursive(points, begin, mid, depth + 1);
        node->right = buildTreeRecursive(points, mid + 1, end, depth + 1);
        return node;
    }

    // Recursive function to delete a point from the KDTree
    Node* deleteRecursive(Node* node, const Point<K>& point, int depth) {
        if (node == nullptr) return nullptr;

        int cd = depth % K;

        // If point matches current node
        if (node->point == point) {
            // If right child exists, find minimum in right subtree
            if (node->right != nullptr) {
                Node* minNode = findMinRecursive(node->right, cd, depth + 1);
                node->point = minNode->point;
                node->right = deleteRecursive(node->right, minNode->point, depth + 1);
            }
            // If left child exists, find minimum in left subtree
            else if (node->left != nullptr) {
                Node* maxNode = findMaxRecursive(node->left, cd, depth + 1);
                node->point = maxNode->point;
                node->left = deleteRecursive(node->left, maxNode->point, depth + 1);
            }
            // Node is a leaf
            else {
                delete node;
                return nullptr;
            }
            return node;
        }

        // Recursively search for the node to delete
        if (point[cd] < node->point[cd])
            node->left = deleteRecursive(node->left, point, depth + 1);
        else
            node->right = deleteRecursive(node->right, point, depth + 1);

        return node;
    }


    // Recursive function to print the KDTree
    void printRecursive(Node* node, int depth) const {
        // Base case: If node is null, return
        if (node == nullptr) return;

        // Print current node with indentation based on depth
        for (int i = 0; i < depth; i++) cout << "  ";
        cout << "ID:" << node->point.id << " (";
        for (size_t i = 0; i < K; i++) {
            cout << node->point[i];
            if (i < K - 1) cout << ", ";
        }
        cout << ")" << endl;

        // Recursively print left and right children
        printRecursive(node->left, depth + 1);
        printRecursive(node->right, depth + 1);
    }

    // Calculate Euclidean distance between two points
    double distance(const Point<K>& p1, const Point<K>& p2) const {
        double dist = 0.0;
        for (size_t i = 0; i < K; i++) {
            double diff = p1[i] - p2[i];
            dist += diff * diff;
        }
        return sqrt(dist);
    }

    // Recursive function for range query
    void rangeQueryRecursive(Node* node, 
                            const array<double, K>& minBounds,
                            const array<double, K>& maxBounds,
                            vector<Point<K>>& results,
                            int depth) const {
        if (node == nullptr) return;

        int cd = depth % K;
        bool inRange = true;

        // Check if current point is within the range
        for (size_t i = 0; i < K; i++) {
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

    // Recursive function for nearest neighbor search
    void nearestNeighborRecursive(Node* node,
                                  const Point<K>& queryPoint,
                                  Point<K>& bestPoint,
                                  double& bestDist,
                                  int depth) const {
        if (node == nullptr) return;

        int cd = depth % K;
        double dist = distance(node->point, queryPoint);

        // Update best if current node is closer
        if (dist < bestDist) {
            bestDist = dist;
            bestPoint = node->point;
        }

        // Determine which side to search first
        double diff = queryPoint[cd] - node->point[cd];
        Node* nearChild = (diff < 0) ? node->left : node->right;
        Node* farChild = (diff < 0) ? node->right : node->left;

        // Search near side first
        if (nearChild != nullptr) {
            nearestNeighborRecursive(nearChild, queryPoint, bestPoint, bestDist, depth + 1);
        }

        // Check if we need to search far side
        if (farChild != nullptr && abs(diff) < bestDist) {
            nearestNeighborRecursive(farChild, queryPoint, bestPoint, bestDist, depth + 1);
        }
    }

    // Structure to hold point with distance for k-NN
    struct PointWithDistance {
        Point<K> point;
        double dist;

        PointWithDistance(const Point<K>& p, double d) : point(p), dist(d) {}

        // For max-heap (we want to keep k smallest distances, so largest at top)
        bool operator<(const PointWithDistance& other) const {
            return dist > other.dist; // Max-heap: larger distances at top
        }
    };

    // Recursive function for k-nearest neighbors search
    void kNearestNeighborsRecursive(Node* node,
                                   const Point<K>& queryPoint,
                                   priority_queue<PointWithDistance>& maxHeap,
                                   size_t k,
                                   int depth) const {
        if (node == nullptr) return;

        int cd = depth % K;
        double dist = distance(node->point, queryPoint);

        // Add point to heap if we have less than k points or if it's closer than the farthest
        if (maxHeap.size() < k) {
            maxHeap.push(PointWithDistance(node->point, dist));
        } else if (dist < maxHeap.top().dist) {
            maxHeap.pop();
            maxHeap.push(PointWithDistance(node->point, dist));
        }

        // Determine which side to search first
        double diff = queryPoint[cd] - node->point[cd];
        Node* nearChild = (diff < 0) ? node->left : node->right;
        Node* farChild = (diff < 0) ? node->right : node->left;

        // Search near side first
        if (nearChild != nullptr) {
            kNearestNeighborsRecursive(nearChild, queryPoint, maxHeap, k, depth + 1);
        }

        // Check if we need to search far side
        double maxDist = (maxHeap.size() == k) ? maxHeap.top().dist : numeric_limits<double>::max();
        if (farChild != nullptr && abs(diff) < maxDist) {
            kNearestNeighborsRecursive(farChild, queryPoint, maxHeap, k, depth + 1);
        }
    }

public:
    // Constructor to initialize the KDTree with a null root
    KDTree() : root(nullptr) {}

    // Public function to insert a point into the KDTree
    void insert(const Point<K>& point) {
        root = insertRecursive(root, point, 0);
    }
    
    // Convenience function to insert from array and id
    void insert(const array<double, K>& coords, int id) {
        insert(Point<K>(coords, id));
    }

    // Public function to search for a point in the KDTree
    bool search(const Point<K>& point) const {
        return searchRecursive(root, point, 0);
    }
    
    // Convenience function to search from array and id
    bool search(const array<double, K>& coords, int id) const {
        return search(Point<K>(coords, id));
    }

    // Public function to delete a point from the KDTree
    bool remove(const Point<K>& point) {
        if (root == nullptr) return false;
        size_t sizeBefore = size();
        root = deleteRecursive(root, point, 0);
        return size() < sizeBefore; // Return true if a node was deleted
    }

    // Convenience function to delete from array and id
    bool remove(const array<double, K>& coords, int id) {
        return remove(Point<K>(coords, id));
    }

    // Public function to update a point in the KDTree
    // This deletes the old point and inserts the new one to maintain proper tree structure
    bool update(const Point<K>& oldPoint, const Point<K>& newPoint) {
        if (root == nullptr) return false;
        if (remove(oldPoint)) {
            insert(newPoint);
            return true;
        }
        return false;
    }

    // Convenience function to update from arrays and ids
    bool update(const array<double, K>& oldCoords, int oldId,
                const array<double, K>& newCoords, int newId) {
        return update(Point<K>(oldCoords, oldId), Point<K>(newCoords, newId));
    }

    // Helper function to count nodes (for size)
    size_t sizeRecursive(Node* node) const {
        if (node == nullptr) return 0;
        return 1 + sizeRecursive(node->left) + sizeRecursive(node->right);
    }

    // Get the number of points in the tree
    size_t size() const {
        return sizeRecursive(root);
    }

    // Check if tree is empty
    bool empty() const {
        return root == nullptr;
    }

    // Public function to print the KDTree
    void print() const {
        printRecursive(root, 0);
    }

    // Range query: Find all points within the given bounds
    vector<Point<K>> rangeQueryPoints(const array<double, K>& minBounds,
                                      const array<double, K>& maxBounds) const {
        vector<Point<K>> results;
        rangeQueryRecursive(root, minBounds, maxBounds, results, 0);
        return results;
    }

    // Range query: Return Movie pointers matching the bounds (for LSH phase)
    vector<Movie*> rangeQuery(const array<double, K>& minBounds,
                              const array<double, K>& maxBounds,
                              const unordered_map<int, Movie*>& movieById) const {
        vector<Point<K>> points = rangeQueryPoints(minBounds, maxBounds);
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

    // Nearest neighbor search: Find the closest point to the query point
    Point<K> nearestNeighbor(const Point<K>& queryPoint) const {
        if (root == nullptr) {
            throw runtime_error("Tree is empty");
        }
        Point<K> bestPoint = root->point;
        double bestDist = numeric_limits<double>::max();
        nearestNeighborRecursive(root, queryPoint, bestPoint, bestDist, 0);
        return bestPoint;
    }
    
    // Convenience function for nearest neighbor from array
    Point<K> nearestNeighbor(const array<double, K>& queryCoords) const {
        Point<K> queryPoint;
        queryPoint.coordinates = queryCoords;
        queryPoint.id = -1; // ID not needed for query
        return nearestNeighbor(queryPoint);
    }

    // k-Nearest neighbors search: Find k closest points to the query point
    vector<Point<K>> kNearestNeighbors(const Point<K>& queryPoint, size_t k) const {
        if (root == nullptr) {
            return vector<Point<K>>();
        }

        // Use max-heap to keep k smallest distances
        priority_queue<PointWithDistance> maxHeap;
        kNearestNeighborsRecursive(root, queryPoint, maxHeap, k, 0);

        // Extract results from heap
        vector<Point<K>> results;
        while (!maxHeap.empty()) {
            results.push_back(maxHeap.top().point);
            maxHeap.pop();
        }
        // Reverse to get points from closest to farthest
        reverse(results.begin(), results.end());
        return results;
    }
    
    // Convenience function for k-NN from array
    vector<Point<K>> kNearestNeighbors(const array<double, K>& queryCoords, size_t k) const {
        Point<K> queryPoint;
        queryPoint.coordinates = queryCoords;
        queryPoint.id = -1; // ID not needed for query
        return kNearestNeighbors(queryPoint, k);
    }

    // Destructor to free memory
    void destroyRecursive(Node* node) {
        if (node != nullptr) {
            destroyRecursive(node->left);
            destroyRecursive(node->right);
            delete node;
        }
    }

    // Build a balanced tree from points using nth_element
    void buildBalanced(const vector<Point<K>>& points) {
        destroyRecursive(root);
        vector<Point<K>> pointsCopy = points;
        root = buildTreeRecursive(pointsCopy, 0, pointsCopy.size(), 0);
    }

    ~KDTree() {
        destroyRecursive(root);
    }
};

int main() {
    // Load movies from CSV file
    string filename = "data_movies_clean.csv";
    vector<Movie> movies = loadMoviesFromCSV(filename);
    
    cout << "Loaded " << movies.size() << " movies from " << filename << endl;
    
    if (movies.empty()) {
        cerr << "No movies loaded. Exiting." << endl;
        return 1;
    }
    
    // Create a KDTree with 5 dimensions
    KDTree<5> tree;

    unordered_map<int, Movie*> movieById;
    movieById.reserve(movies.size());
    vector<Point<5>> moviePoints;
    moviePoints.reserve(movies.size());

    for (auto& movie : movies) {
        movieById[movie.id] = &movie;
        moviePoints.push_back(movieToPoint(movie));
    }

    cout << "\nBuilding balanced KDTree<5>..." << endl;
    auto buildStart = chrono::high_resolution_clock::now();
    tree.buildBalanced(moviePoints);
    auto buildEnd = chrono::high_resolution_clock::now();
    auto buildMs = chrono::duration_cast<chrono::milliseconds>(buildEnd - buildStart).count();
    cout << "Built " << movies.size() << " movies into the tree in " << buildMs << " ms." << endl;

    array<double, 5> benchMinBounds = {2000.0, 3.0, 0.0, 0.0, 0.0};
    array<double, 5> benchMaxBounds = {2020.0, 6.0, 10.0, 9999.0, 999999999999.0};
    auto searchStart = chrono::high_resolution_clock::now();
    auto benchCandidates = tree.rangeQuery(benchMinBounds, benchMaxBounds, movieById);
    auto searchEnd = chrono::high_resolution_clock::now();
    auto searchMs = chrono::duration_cast<chrono::milliseconds>(searchEnd - searchStart).count();
    cout << "Benchmark query returned " << benchCandidates.size()
         << " candidate(s) in " << searchMs << " ms." << endl;
    
    // Interactive menu for filtering candidates
    int choice;
    bool continueSearch = true;
    
    while (continueSearch) {
        cout << "\n========================================" << endl;
        cout << "   KD-Tree Candidate Search Menu" << endl;
        cout << "========================================" << endl;
        cout << "1. Filter by Release Year" << endl;
        cout << "2. Filter by Popularity" << endl;
        cout << "3. Filter by Vote Average" << endl;
        cout << "4. Filter by Runtime" << endl;
        cout << "5. Filter by Budget" << endl;
        cout << "6. Filter by Multiple Criteria" << endl;
        cout << "7. Exit" << endl;
        cout << "========================================" << endl;
        cout << "Enter your choice (1-7): ";
        cin >> choice;
        
        if (choice == 7) {
            continueSearch = false;
            cout << "Exiting..." << endl;
            break;
        }
        
        array<double, 5> minBounds = {0.0, 0.0, 0.0, 0.0, 0.0};
        array<double, 5> maxBounds = {9999.0, 9999.0, 10.0, 9999.0, 999999999999.0};
        
        switch (choice) {
            case 1: { // Filter by Release Year
                int minYear, maxYear;
                cout << "\n--- Filter by Release Year ---" << endl;
                cout << "Enter minimum year: ";
                cin >> minYear;
                cout << "Enter maximum year: ";
                cin >> maxYear;
                minBounds[0] = static_cast<double>(minYear);
                maxBounds[0] = static_cast<double>(maxYear);
                break;
            }
            case 2: { // Filter by Popularity
                double minPop, maxPop;
                cout << "\n--- Filter by Popularity ---" << endl;
                cout << "Enter minimum popularity: ";
                cin >> minPop;
                cout << "Enter maximum popularity: ";
                cin >> maxPop;
                minBounds[1] = minPop;
                maxBounds[1] = maxPop;
                break;
            }
            case 3: { // Filter by Vote Average
                double minVote, maxVote;
                cout << "\n--- Filter by Vote Average ---" << endl;
                cout << "Enter minimum vote average (0-10): ";
                cin >> minVote;
                cout << "Enter maximum vote average (0-10): ";
                cin >> maxVote;
                minBounds[2] = minVote;
                maxBounds[2] = maxVote;
                break;
            }
            case 4: { // Filter by Runtime
                int minRun, maxRun;
                cout << "\n--- Filter by Runtime ---" << endl;
                cout << "Enter minimum runtime (minutes): ";
                cin >> minRun;
                cout << "Enter maximum runtime (minutes): ";
                cin >> maxRun;
                minBounds[3] = static_cast<double>(minRun);
                maxBounds[3] = static_cast<double>(maxRun);
                break;
            }
            case 5: { // Filter by Budget
                double minBudget, maxBudget;
                cout << "\n--- Filter by Budget ---" << endl;
                cout << "Enter minimum budget (USD): ";
                cin >> minBudget;
                cout << "Enter maximum budget (USD): ";
                cin >> maxBudget;
                minBounds[4] = minBudget;
                maxBounds[4] = maxBudget;
                break;
            }
            case 6: { // Filter by Multiple Criteria
                cout << "\n--- Filter by Multiple Criteria ---" << endl;
                cout << "Leave blank (press Enter) to skip a filter" << endl;
                
                string input;
                
                // Year
                cout << "Minimum year (or Enter to skip): ";
                cin.ignore();
                getline(cin, input);
                if (!input.empty()) minBounds[0] = stod(input);
                
                cout << "Maximum year (or Enter to skip): ";
                getline(cin, input);
                if (!input.empty()) maxBounds[0] = stod(input);
                
                // Popularity
                cout << "Minimum popularity (or Enter to skip): ";
                getline(cin, input);
                if (!input.empty()) minBounds[1] = stod(input);
                
                cout << "Maximum popularity (or Enter to skip): ";
                getline(cin, input);
                if (!input.empty()) maxBounds[1] = stod(input);
                
                // Vote Average
                cout << "Minimum vote average (or Enter to skip): ";
                getline(cin, input);
                if (!input.empty()) minBounds[2] = stod(input);
                
                cout << "Maximum vote average (or Enter to skip): ";
                getline(cin, input);
                if (!input.empty()) maxBounds[2] = stod(input);
                
                // Runtime
                cout << "Minimum runtime in minutes (or Enter to skip): ";
                getline(cin, input);
                if (!input.empty()) minBounds[3] = stod(input);
                
                cout << "Maximum runtime in minutes (or Enter to skip): ";
                getline(cin, input);
                if (!input.empty()) maxBounds[3] = stod(input);
                
                // Budget
                cout << "Minimum budget in USD (or Enter to skip): ";
                getline(cin, input);
                if (!input.empty()) minBounds[4] = stod(input);
                
                cout << "Maximum budget in USD (or Enter to skip): ";
                getline(cin, input);
                if (!input.empty()) maxBounds[4] = stod(input);
                break;
            }
            default:
                cout << "Invalid choice. Please try again." << endl;
                continue;
        }
        
        // Perform range query to find candidates
        cout << "\n=== Searching for Candidates ===" << endl;
        auto candidates = tree.rangeQuery(minBounds, maxBounds, movieById);
        
        cout << "\nFound " << candidates.size() << " candidate(s) matching your criteria:" << endl;
        cout << "----------------------------------------" << endl;
        
        // Display candidates (limit to first 20 for readability)
        size_t displayLimit = min(20UL, candidates.size());
        for (size_t i = 0; i < displayLimit; i++) {
            Movie* movie = candidates[i];
            cout << "\n" << (i+1) << ". Movie ID: " << movie->id << endl;
            cout << "   Title: " << movie->title << endl;
            cout << "   Year: " << extractYear(movie->release_date) << endl;
            cout << "   Popularity: " << movie->popularity << endl;
            cout << "   Vote Average: " << movie->vote_average << endl;
            cout << "   Runtime: " << movie->runtime << " minutes" << endl;
            cout << "   Budget: $" << movie->budget << endl;
        }
        
        if (candidates.size() > displayLimit) {
            cout << "\n... and " << (candidates.size() - displayLimit) << " more candidates." << endl;
        }
        
        cout << "\nTotal candidates found: " << candidates.size() << endl;
    }

    return 0;
}