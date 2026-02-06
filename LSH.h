#ifndef LSH_H
#define LSH_H

#include <vector>
#include <string>
#include <set>
#include <map>
#include "Movie.h"

// LSH Parameters
const int NUM_HASH_FUNCTIONS = 100; // Number of permutations for MinHash
const int SHINGLE_SIZE = 3;         // Size of k-grams (e.g., "the" -> "the")

class LSH {
private:
    // Coefficients for random hash functions (h(x) = (ax + b) % p)
    std::vector<int> a_coeffs;
    std::vector<int> b_coeffs;
    const int prime = 2147483647; // Mersenne prime

    // Store signatures for the current subset of movies
    // Map: Movie Pointer -> Signature (vector of min-hash values)
    std::map<Movie*, std::vector<int>> signatures;

    // Helper: Generate random coefficients
    void generateHashFunctions();

    // Helper: Create k-shingles from a string
    std::set<size_t> getShingles(const std::string& text);

    // Helper: Compute MinHash signature for a set of shingles
    std::vector<int> computeSignature(const std::set<size_t>& shingles);

    // Helper: Calculate Jaccard similarity between two signatures
    double calculateJaccard(const std::vector<int>& sig1, const std::vector<int>& sig2);

public:
    LSH();
    
    // Process the filtered results to build the LSH index
    // attributeType: 1 = Title, 2 = Production Companies, 3 = Genres
    void buildIndex(const std::vector<Movie*>& movies, int attributeType);

    // Query the LSH index with a user string
    std::vector<std::pair<Movie*, double>> search(const std::string& queryStr, double threshold);
};

#endif