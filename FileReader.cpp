#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cctype>

using namespace std;

struct Movie {
    int id;
    string title;
    bool adult;
    string original_language;
    vector<string> origin_country;
    string release_date;
    vector<string> genre_names;
    vector<string> production_company_names;
    double budget;
    double revenue;
    int runtime;
    double popularity;
    double vote_average;
    int vote_count;
};

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

int main() {
    string filename = "data_movies_clean.csv";

    vector<Movie> movies = loadMoviesFromCSV(filename);

    cout << "Loaded " << movies.size() << " movies.\n";

    if (!movies.empty()) {
        cout << "\nFirst movie loaded:\n";
        cout << "ID: " << movies[0].id << "\n";
        cout << "Title: " << movies[0].title << "\n";
        cout << "Language: " << movies[0].original_language << "\n";
        cout << "Budget: " << movies[0].budget << "\n";
    }

    return 0;
}
