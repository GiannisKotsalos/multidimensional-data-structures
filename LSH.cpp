#include "LSH.h"
#include <iostream>
#include <random>
#include <climits>
#include <algorithm>
#include <sstream>

// Constructor: Initialize random hash functions
LSH::LSH() {
    generateHashFunctions();
}

void LSH::generateHashFunctions() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(1, prime - 1);

    a_coeffs.resize(NUM_HASH_FUNCTIONS);
    b_coeffs.resize(NUM_HASH_FUNCTIONS);

    for (int i = 0; i < NUM_HASH_FUNCTIONS; ++i) {
        a_coeffs[i] = dist(gen);
        b_coeffs[i] = dist(gen);
    }
}

// Convert string to set of hashed shingles (k-grams)
std::set<size_t> LSH::getShingles(const std::string& text) {
    std::set<size_t> shingles;
    std::string processed = text;
    
    // Basic preprocessing: lowercase
    std::transform(processed.begin(), processed.end(), processed.begin(), ::tolower);

    if (processed.length() < SHINGLE_SIZE) {
        shingles.insert(std::hash<std::string>{}(processed));
        return shingles;
    }

    for (size_t i = 0; i <= processed.length() - SHINGLE_SIZE; ++i) {
        std::string shingle = processed.substr(i, SHINGLE_SIZE);
        shingles.insert(std::hash<std::string>{}(shingle));
    }
    return shingles;
}

// Compute MinHash signature
std::vector<int> LSH::computeSignature(const std::set<size_t>& shingles) {
    std::vector<int> sig(NUM_HASH_FUNCTIONS, INT_MAX);

    for (const auto& shingleHash : shingles) {
        for (int i = 0; i < NUM_HASH_FUNCTIONS; ++i) {
            // h(x) = (ax + b) % p
            long long hashVal = (static_cast<long long>(a_coeffs[i]) * shingleHash + b_coeffs[i]) % prime;
            if (hashVal < sig[i]) {
                sig[i] = static_cast<int>(hashVal);
            }
        }
    }
    return sig;
}

// Build the index based on the filtered movie subset
void LSH::buildIndex(const std::vector<Movie*>& movies, int attributeType) {
    signatures.clear();

    for (Movie* m : movies) {
        std::string textToProcess = "";

        if (attributeType == 1) { // Title
            textToProcess = m->title;
        } 
        else if (attributeType == 2) { // Production Companies
            // Join vector into single string for shingling
            for (const auto& comp : m->production_company_names) {
                textToProcess += comp + " ";
            }
        }
        else if (attributeType == 3) { // Genres
             for (const auto& genre : m->genre_names) {
                textToProcess += genre + " ";
            }
        }

        std::set<size_t> shingles = getShingles(textToProcess);
        signatures[m] = computeSignature(shingles);
    }
}

double LSH::calculateJaccard(const std::vector<int>& sig1, const std::vector<int>& sig2) {
    int matches = 0;
    for (int i = 0; i < NUM_HASH_FUNCTIONS; ++i) {
        if (sig1[i] == sig2[i]) {
            matches++;
        }
    }
    return static_cast<double>(matches) / NUM_HASH_FUNCTIONS;
}

std::vector<std::pair<Movie*, double>> LSH::search(const std::string& queryStr, double threshold) {
    std::vector<std::pair<Movie*, double>> results;

    // 1. Process the user's query string into a signature
    std::set<size_t> queryShingles = getShingles(queryStr);
    std::vector<int> querySig = computeSignature(queryShingles);

    // 2. Compare against all movies in the current filtered subset
    for (auto const& [movie, movieSig] : signatures) {
        double similarity = calculateJaccard(querySig, movieSig);
        if (similarity >= threshold) {
            results.push_back({movie, similarity});
        }
    }

    // 3. Sort by similarity (descending)
    std::sort(results.begin(), results.end(), 
        [](const std::pair<Movie*, double>& a, const std::pair<Movie*, double>& b) {
            return a.second > b.second; 
        });

    return results;
}