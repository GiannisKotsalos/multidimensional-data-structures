#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cctype>
#include "Movie.h" // Include the shared Movie struct


using namespace std;



// Splits CSV respecting quotes
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

// Extracts list fields like ['US', 'GB'] → vector<string>
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
        cerr << "Error opening file.\n";
        return movies;
    }

    string line;
    getline(file, line); // skip header

    while (getline(file, line)) {
        if (line.empty()) continue;

        auto fields = parseCSVRow(line);
        if (fields.size() != 14) {
            cerr << "Skipping malformed row: " << line << "\n";
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
