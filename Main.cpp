#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <unordered_map>
#include <array>
#include "Movie.h"
#include "FileReader.h"
#include "Rtree/Rtree.h"
#include "KDtree/KDTree.h"
#include "QuadTree/quadtree.h"

using namespace std;

// Display results helper function
void displayResults(const vector<Movie*>& results, size_t limit = 20) {
    cout << "\nFound " << results.size() << " candidate(s) matching your criteria:" << endl;
    cout << "----------------------------------------" << endl;
    
    size_t displayLimit = min(limit, results.size());
    for (size_t i = 0; i < displayLimit; i++) {
        Movie* movie = results[i];
        cout << "\n" << (i+1) << ". Movie ID: " << movie->id << endl;
        cout << "   Title: " << movie->title << endl;
        cout << "   Budget: $" << movie->budget << endl;
        cout << "   Revenue: $" << movie->revenue << endl;
        cout << "   Runtime: " << movie->runtime << " minutes" << endl;
        cout << "   Popularity: " << movie->popularity << endl;
        cout << "   Vote Average: " << movie->vote_average << "/10" << endl;
    }
    
    if (results.size() > displayLimit) {
        cout << "\n... and " << (results.size() - displayLimit) << " more candidates." << endl;
    }
    
    cout << "\nTotal candidates found: " << results.size() << endl;
}

// Get query bounds from user - returns pair of min/max arrays
pair<array<double, KD_DIMS>, array<double, KD_DIMS>> getUserQueryBounds() {
    array<double, KD_DIMS> minBounds = {0.0, 0.0, 0.0, 0.0, 0.0};
    array<double, KD_DIMS> maxBounds = {1e15, 1e15, 9999.0, 9999.0, 10.0};
    
    string attrNames[KD_DIMS] = {"Budget", "Revenue", "Runtime", "Popularity", "Vote Average"};
    
    int choice;
    cout << "\n========================================" << endl;
    cout << "        Search Filter Menu" << endl;
    cout << "========================================" << endl;
    cout << "1. Filter by Budget" << endl;
    cout << "2. Filter by Revenue" << endl;
    cout << "3. Filter by Runtime" << endl;
    cout << "4. Filter by Popularity" << endl;
    cout << "5. Filter by Vote Average" << endl;
    cout << "6. Filter by Multiple Criteria" << endl;
    cout << "7. Back to Main Menu" << endl;
    cout << "========================================" << endl;
    cout << "Enter your choice (1-7): ";
    cin >> choice;
    
    if (choice == 7) {
        // Return empty bounds to signal going back
        minBounds[0] = -1;
        return {minBounds, maxBounds};
    }
    
    switch (choice) {
        case 1: { // Budget
            double minVal, maxVal;
            cout << "\n--- Filter by Budget ---" << endl;
            cout << "Enter minimum budget (USD): ";
            cin >> minVal;
            cout << "Enter maximum budget (USD): ";
            cin >> maxVal;
            minBounds[0] = minVal;
            maxBounds[0] = maxVal;
            break;
        }
        case 2: { // Revenue
            double minVal, maxVal;
            cout << "\n--- Filter by Revenue ---" << endl;
            cout << "Enter minimum revenue (USD): ";
            cin >> minVal;
            cout << "Enter maximum revenue (USD): ";
            cin >> maxVal;
            minBounds[1] = minVal;
            maxBounds[1] = maxVal;
            break;
        }
        case 3: { // Runtime
            int minVal, maxVal;
            cout << "\n--- Filter by Runtime ---" << endl;
            cout << "Enter minimum runtime (minutes): ";
            cin >> minVal;
            cout << "Enter maximum runtime (minutes): ";
            cin >> maxVal;
            minBounds[2] = static_cast<double>(minVal);
            maxBounds[2] = static_cast<double>(maxVal);
            break;
        }
        case 4: { // Popularity
            double minVal, maxVal;
            cout << "\n--- Filter by Popularity ---" << endl;
            cout << "Enter minimum popularity: ";
            cin >> minVal;
            cout << "Enter maximum popularity: ";
            cin >> maxVal;
            minBounds[3] = minVal;
            maxBounds[3] = maxVal;
            break;
        }
        case 5: { // Vote Average
            double minVal, maxVal;
            cout << "\n--- Filter by Vote Average ---" << endl;
            cout << "Enter minimum vote average (0-10): ";
            cin >> minVal;
            cout << "Enter maximum vote average (0-10): ";
            cin >> maxVal;
            minBounds[4] = minVal;
            maxBounds[4] = maxVal;
            break;
        }
        case 6: { // Multiple Criteria
            cout << "\n--- Filter by Multiple Criteria ---" << endl;
            cout << "Leave blank (press Enter) to skip a filter" << endl;
            
            string input;
            cin.ignore();
            
            for (int i = 0; i < KD_DIMS; i++) {
                cout << "Minimum " << attrNames[i] << " (or Enter to skip): ";
                getline(cin, input);
                if (!input.empty()) minBounds[i] = stod(input);
                
                cout << "Maximum " << attrNames[i] << " (or Enter to skip): ";
                getline(cin, input);
                if (!input.empty()) maxBounds[i] = stod(input);
            }
            break;
        }
        default:
            cout << "Invalid choice." << endl;
            minBounds[0] = -1;
            break;
    }
    
    return {minBounds, maxBounds};
}

// Convert array to Rect for R-Tree
Rect arrayToRect(const array<double, KD_DIMS>& minBounds, const array<double, KD_DIMS>& maxBounds) {
    Rect r;
    for (int i = 0; i < KD_DIMS; i++) {
        r.min_coords[i] = minBounds[i];
        r.max_coords[i] = maxBounds[i];
    }
    return r;
}

// Convert array to QuadRect for Quadtree
QuadRect arrayToQuadRect(const array<double, KD_DIMS>& minBounds, const array<double, KD_DIMS>& maxBounds) {
    QuadRect qr;
    for (int i = 0; i < Q_K; i++) {
        qr.min_coords[i] = minBounds[i];
        qr.max_coords[i] = maxBounds[i];
    }
    return qr;
}

int main() {
    // Load movies from CSV file
    cout << "Loading movies from CSV..." << endl;
    vector<Movie> movies = loadMoviesFromCSV("data_movies_clean.csv");
    
    if (movies.empty()) {
        cerr << "No movies loaded. Exiting." << endl;
        return 1;
    }
    
    cout << "Loaded " << movies.size() << " movies successfully.\n" << endl;
    
    // Prepare ID lookup map for KD-Tree results
    unordered_map<int, Movie*> movieById;
    movieById.reserve(movies.size());
    for (auto& movie : movies) {
        movieById[movie.id] = &movie;
    }
    
    // Tree instances (built on demand)
    RTree* rtree = nullptr;
    KDTree* kdtree = nullptr;
    Quadtree* qtree = nullptr;
    
    bool running = true;
    
    while (running) {
        cout << "\n╔════════════════════════════════════════╗" << endl;
        cout << "║     Multi-Dimensional Data Structures  ║" << endl;
        cout << "║          Movie Search System           ║" << endl;
        cout << "╠════════════════════════════════════════╣" << endl;
        cout << "║  1. Use R-Tree                         ║" << endl;
        cout << "║  2. Use KD-Tree                        ║" << endl;
        cout << "║  3. Use Quadtree (Hyperoctree)         ║" << endl;
        cout << "║  4. Compare all three                  ║" << endl;
        cout << "║  5. Exit                               ║" << endl;
        cout << "╚════════════════════════════════════════╝" << endl;
        cout << "Enter your choice (1-5): ";
        
        int treeChoice;
        cin >> treeChoice;
        
        if (treeChoice == 5) {
            cout << "Exiting..." << endl;
            running = false;
            break;
        }
        
        if (treeChoice < 1 || treeChoice > 4) {
            cout << "Invalid choice. Please try again." << endl;
            continue;
        }
        
        // Build trees if needed
        if ((treeChoice == 1 || treeChoice == 4) && rtree == nullptr) {
            auto start = chrono::high_resolution_clock::now();
            rtree = new RTree();
            rtree->build(movies);
            auto end = chrono::high_resolution_clock::now();
            auto ms = chrono::duration_cast<chrono::milliseconds>(end - start).count();
            cout << "R-Tree built in " << ms << " ms." << endl;
        }
        
        if ((treeChoice == 2 || treeChoice == 4) && kdtree == nullptr) {
            auto start = chrono::high_resolution_clock::now();
            kdtree = new KDTree();
            kdtree->build(movies);
            auto end = chrono::high_resolution_clock::now();
            auto ms = chrono::duration_cast<chrono::milliseconds>(end - start).count();
            cout << "KD-Tree built in " << ms << " ms." << endl;
        }
        
        // Build Quadtree if needed
        if ((treeChoice == 3 || treeChoice == 4) && qtree == nullptr) {
            auto start = chrono::high_resolution_clock::now();
            qtree = new Quadtree();
            qtree->build(movies);
            auto end = chrono::high_resolution_clock::now();
            auto ms = chrono::duration_cast<chrono::milliseconds>(end - start).count();
            cout << "Quadtree built in " << ms << " ms." << endl;
        }
        
        // Search loop for selected tree(s)
        bool searching = true;
        while (searching) {
            auto [minBounds, maxBounds] = getUserQueryBounds();
            
            // Check if user wants to go back
            if (minBounds[0] == -1) {
                searching = false;
                continue;
            }
            
            cout << "\n=== Searching for Candidates ===" << endl;
            
            if (treeChoice == 1) {
                // R-Tree only
                Rect query = arrayToRect(minBounds, maxBounds);
                auto start = chrono::high_resolution_clock::now();
                vector<Movie*> results = rtree->search(query);
                auto end = chrono::high_resolution_clock::now();
                auto ms = chrono::duration_cast<chrono::milliseconds>(end - start).count();
                
                cout << "\n[R-Tree] Search completed in " << ms << " ms." << endl;
                displayResults(results);
            }
            else if (treeChoice == 2) {
                // KD-Tree only
                auto start = chrono::high_resolution_clock::now();
                vector<Movie*> results = kdtree->search(minBounds, maxBounds, movieById);
                auto end = chrono::high_resolution_clock::now();
                auto ms = chrono::duration_cast<chrono::milliseconds>(end - start).count();
                
                cout << "\n[KD-Tree] Search completed in " << ms << " ms." << endl;
                displayResults(results);
            }
            else if (treeChoice == 3) {
                // Quadtree only
                QuadRect query = arrayToQuadRect(minBounds, maxBounds);
                auto start = chrono::high_resolution_clock::now();
                vector<Movie*> results = qtree->search(query);
                auto end = chrono::high_resolution_clock::now();
                auto ms = chrono::duration_cast<chrono::milliseconds>(end - start).count();

                cout << "\n[Quadtree] Search completed in " << ms << " ms." << endl;
                displayResults(results);
            }
            else if (treeChoice == 4) {
                // Compare all three trees
                Rect rQuery = arrayToRect(minBounds, maxBounds);
                QuadRect qQuery = arrayToQuadRect(minBounds, maxBounds);
                
                // R-Tree search
                auto rtreeStart = chrono::high_resolution_clock::now();
                vector<Movie*> rtreeResults = rtree->search(rQuery);
                auto rtreeEnd = chrono::high_resolution_clock::now();
                auto rtreeMs = chrono::duration_cast<chrono::milliseconds>(rtreeEnd - rtreeStart).count();
                
                // KD-Tree search
                auto kdtreeStart = chrono::high_resolution_clock::now();
                vector<Movie*> kdtreeResults = kdtree->search(minBounds, maxBounds, movieById);
                auto kdtreeEnd = chrono::high_resolution_clock::now();
                auto kdtreeMs = chrono::duration_cast<chrono::milliseconds>(kdtreeEnd - kdtreeStart).count();

                // Quadtree search
                auto qtreeStart = chrono::high_resolution_clock::now();
                vector<Movie*> qtreeResults = qtree->search(qQuery);
                auto qtreeEnd = chrono::high_resolution_clock::now();
                auto qtreeMs = chrono::duration_cast<chrono::milliseconds>(qtreeEnd - qtreeStart).count();
                
                cout << "\n╔════════════════════════════════════════╗" << endl;
                cout << "║         Performance Comparison         ║" << endl;
                cout << "╠════════════════════════════════════════╣" << endl;
                cout << "║  R-Tree:    " << rtreeMs  << " ms, " << rtreeResults.size()  << " results" << endl;
                cout << "║  KD-Tree:   " << kdtreeMs << " ms, " << kdtreeResults.size() << " results" << endl;
                cout << "║  Quadtree:  " << qtreeMs  << " ms, " << qtreeResults.size()  << " results" << endl;
                cout << "╚════════════════════════════════════════╝" << endl;
                
                // Display KD-Tree results as the detailed view
                displayResults(kdtreeResults);
            }
        }
    }
    
    // Cleanup
    if (rtree != nullptr) delete rtree;
    if (kdtree != nullptr) delete kdtree;
    if (qtree != nullptr) delete qtree;
    
    return 0;
}
