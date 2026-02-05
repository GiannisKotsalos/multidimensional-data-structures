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
#include "RangeTree/RangeTree.h"

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
        minBounds[0] = -1; // Signal to go back
        return {minBounds, maxBounds};
    }
    
    switch (choice) {
        case 1: { // Budget
            double minVal, maxVal;
            cout << "\n--- Filter by Budget ---" << endl;
            cout << "Enter minimum budget (USD): "; cin >> minVal;
            cout << "Enter maximum budget (USD): "; cin >> maxVal;
            minBounds[0] = minVal; maxBounds[0] = maxVal;
            break;
        }
        case 2: { // Revenue
            double minVal, maxVal;
            cout << "\n--- Filter by Revenue ---" << endl;
            cout << "Enter minimum revenue (USD): "; cin >> minVal;
            cout << "Enter maximum revenue (USD): "; cin >> maxVal;
            minBounds[1] = minVal; maxBounds[1] = maxVal;
            break;
        }
        case 3: { // Runtime
            int minVal, maxVal;
            cout << "\n--- Filter by Runtime ---" << endl;
            cout << "Enter minimum runtime (minutes): "; cin >> minVal;
            cout << "Enter maximum runtime (minutes): "; cin >> maxVal;
            minBounds[2] = static_cast<double>(minVal); maxBounds[2] = static_cast<double>(maxVal);
            break;
        }
        case 4: { // Popularity
            double minVal, maxVal;
            cout << "\n--- Filter by Popularity ---" << endl;
            cout << "Enter minimum popularity: "; cin >> minVal;
            cout << "Enter maximum popularity: "; cin >> maxVal;
            minBounds[3] = minVal; maxBounds[3] = maxVal;
            break;
        }
        case 5: { // Vote Average
            double minVal, maxVal;
            cout << "\n--- Filter by Vote Average ---" << endl;
            cout << "Enter minimum vote average (0-10): "; cin >> minVal;
            cout << "Enter maximum vote average (0-10): "; cin >> maxVal;
            minBounds[4] = minVal; maxBounds[4] = maxVal;
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

// Helper Converters
Rect arrayToRect(const array<double, KD_DIMS>& minBounds, const array<double, KD_DIMS>& maxBounds) {
    Rect r;
    for (int i = 0; i < KD_DIMS; i++) {
        r.min_coords[i] = minBounds[i];
        r.max_coords[i] = maxBounds[i];
    }
    return r;
}

QuadRect arrayToQuadRect(const array<double, KD_DIMS>& minBounds, const array<double, KD_DIMS>& maxBounds) {
    QuadRect qr;
    for (int i = 0; i < Q_K; i++) {
        qr.min_coords[i] = minBounds[i];
        qr.max_coords[i] = maxBounds[i];
    }
    return qr;
}

int main() {
    cout << "Loading movies from CSV..." << endl;
    vector<Movie> movies = loadMoviesFromCSV("data_movies_clean.csv");
    
    if (movies.empty()) {
        cerr << "No movies loaded. Exiting." << endl;
        return 1;
    }
    
    cout << "Loaded " << movies.size() << " movies successfully.\n" << endl;
    
    // Prepare ID lookup for KD-Tree
    unordered_map<int, Movie*> movieById;
    movieById.reserve(movies.size());
    for (auto& movie : movies) movieById[movie.id] = &movie;
    
    // Tree pointers
    RTree* rtree = nullptr;
    KDTree* kdtree = nullptr;
    Quadtree* qtree = nullptr;
    RangeTree* rangeTree = nullptr; // Added Range Tree
    
    bool running = true;
    
    while (running) {
        cout << "\n╔════════════════════════════════════════╗" << endl;
        cout << "║     Multi-Dimensional Data Structures  ║" << endl;
        cout << "║          Movie Search System           ║" << endl;
        cout << "╠════════════════════════════════════════╣" << endl;
        cout << "║  1. Use R-Tree                         ║" << endl;
        cout << "║  2. Use KD-Tree                        ║" << endl;
        cout << "║  3. Use Quadtree (Hyperoctree)         ║" << endl;
        cout << "║  4. Use Range Tree                     ║" << endl;
        cout << "║  5. Compare ALL Algorithms             ║" << endl;
        cout << "║  6. Exit                               ║" << endl;
        cout << "╚════════════════════════════════════════╝" << endl;
        cout << "Enter your choice (1-6): ";
        
        int treeChoice;
        cin >> treeChoice;
        
        if (treeChoice == 6) {
            cout << "Exiting..." << endl;
            running = false;
            break;
        }
        
        if (treeChoice < 1 || treeChoice > 5) {
            cout << "Invalid choice. Please try again." << endl;
            continue;
        }
        
        // --- Build Trees On Demand ---
        
        // R-Tree
        if ((treeChoice == 1 || treeChoice == 5) && rtree == nullptr) {
            auto start = chrono::high_resolution_clock::now();
            rtree = new RTree();
            rtree->build(movies);
            auto end = chrono::high_resolution_clock::now();
            cout << "R-Tree built in " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms." << endl;
        }
        
        // KD-Tree
        if ((treeChoice == 2 || treeChoice == 5) && kdtree == nullptr) {
            auto start = chrono::high_resolution_clock::now();
            kdtree = new KDTree();
            kdtree->build(movies);
            auto end = chrono::high_resolution_clock::now();
            cout << "KD-Tree built in " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms." << endl;
        }
        
        // Quadtree
        if ((treeChoice == 3 || treeChoice == 5) && qtree == nullptr) {
            auto start = chrono::high_resolution_clock::now();
            qtree = new Quadtree();
            qtree->build(movies);
            auto end = chrono::high_resolution_clock::now();
            cout << "Quadtree built in " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms." << endl;
        }

        // Range Tree
        if ((treeChoice == 4 || treeChoice == 5) && rangeTree == nullptr) {
            auto start = chrono::high_resolution_clock::now();
            rangeTree = new RangeTree();
            rangeTree->build(movies);
            auto end = chrono::high_resolution_clock::now();
            cout << "Range Tree built in " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms." << endl;
        }
        
        // --- Search Loop ---
        bool searching = true;
        while (searching) {
            auto [minBoundsArr, maxBoundsArr] = getUserQueryBounds();
            
            if (minBoundsArr[0] == -1) {
                searching = false;
                continue;
            }
            
            // Convert to raw pointers for Range Tree (if needed) or simple access
            double minB[KD_DIMS], maxB[KD_DIMS];
            for(int i=0; i<KD_DIMS; ++i) { minB[i] = minBoundsArr[i]; maxB[i] = maxBoundsArr[i]; }

            cout << "\n=== Searching for Candidates ===" << endl;
            
            if (treeChoice == 1) { // R-Tree
                Rect query = arrayToRect(minBoundsArr, maxBoundsArr);
                auto start = chrono::high_resolution_clock::now();
                vector<Movie*> results = rtree->search(query);
                auto end = chrono::high_resolution_clock::now();
                cout << "\n[R-Tree] Search completed in " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms." << endl;
                displayResults(results);
            }
            else if (treeChoice == 2) { // KD-Tree
                auto start = chrono::high_resolution_clock::now();
                vector<Movie*> results = kdtree->search(minBoundsArr, maxBoundsArr, movieById);
                auto end = chrono::high_resolution_clock::now();
                cout << "\n[KD-Tree] Search completed in " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms." << endl;
                displayResults(results);
            }
            else if (treeChoice == 3) { // Quadtree
                QuadRect query = arrayToQuadRect(minBoundsArr, maxBoundsArr);
                auto start = chrono::high_resolution_clock::now();
                vector<Movie*> results = qtree->search(query);
                auto end = chrono::high_resolution_clock::now();
                cout << "\n[Quadtree] Search completed in " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms." << endl;
                displayResults(results);
            }
            else if (treeChoice == 4) { // Range Tree
                auto start = chrono::high_resolution_clock::now();
                vector<Movie*> results = rangeTree->search(minB, maxB);
                auto end = chrono::high_resolution_clock::now();
                cout << "\n[Range Tree] Search completed in " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms." << endl;
                displayResults(results);
            }
            else if (treeChoice == 5) { // Compare All
                Rect rQuery = arrayToRect(minBoundsArr, maxBoundsArr);
                QuadRect qQuery = arrayToQuadRect(minBoundsArr, maxBoundsArr);
                
                // R-Tree
                auto start1 = chrono::high_resolution_clock::now();
                auto res1 = rtree->search(rQuery);
                auto ms1 = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - start1).count();
                
                // KD-Tree
                auto start2 = chrono::high_resolution_clock::now();
                auto res2 = kdtree->search(minBoundsArr, maxBoundsArr, movieById);
                auto ms2 = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - start2).count();

                // Quadtree
                auto start3 = chrono::high_resolution_clock::now();
                auto res3 = qtree->search(qQuery);
                auto ms3 = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - start3).count();
                
                // Range Tree
                auto start4 = chrono::high_resolution_clock::now();
                auto res4 = rangeTree->search(minB, maxB);
                auto ms4 = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - start4).count();
                
                cout << "\n╔═══════════════════════════════════════════════════╗" << endl;
                cout << "║               Performance Comparison              ║" << endl;
                cout << "╠═══════════════════════════════════════════════════╣" << endl;
                cout << "║  R-Tree:     " << ms1 << " ms, " << res1.size() << " results" << endl;
                cout << "║  KD-Tree:    " << ms2 << " ms, " << res2.size() << " results" << endl;
                cout << "║  Quadtree:   " << ms3 << " ms, " << res3.size() << " results" << endl;
                cout << "║  Range Tree: " << ms4 << " ms, " << res4.size() << " results" << endl;
                cout << "╚═══════════════════════════════════════════════════╝" << endl;
                
                displayResults(res4); // Display results from Range Tree (or any)
            }
        }
    }
    
    // Cleanup
    if (rtree) delete rtree;
    if (kdtree) delete kdtree;
    if (qtree) delete qtree;
    if (rangeTree) delete rangeTree;
    
    return 0;
}