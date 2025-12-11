#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cctype>

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

// Splits CSV respecting quotes
std::vector<std::string> parseCSVRow(const std::string &line) {
    std::vector<std::string> fields;
    std::string cur;
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
std::vector<std::string> parseList(const std::string &s) {
    std::vector<std::string> result;
    std::string temp;
    bool inWord = false;

    for (char c : s) {
        if (std::isalnum(c)) {
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

std::vector<Movie> loadMoviesFromCSV(const std::string &filename) {
    std::vector<Movie> movies;
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error opening file.\n";
        return movies;
    }

    std::string line;
    std::getline(file, line); // skip header

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        auto fields = parseCSVRow(line);
        if (fields.size() != 14) {
            std::cerr << "Skipping malformed row: " << line << "\n";
            continue;
        }

        Movie m;
        int i = 0;

        m.id = std::stoi(fields[i++]);
        m.title = fields[i++];
        m.adult = (fields[i++] == "TRUE" || fields[i-1] == "true");
        m.original_language = fields[i++];
        m.origin_country = parseList(fields[i++]);
        m.release_date = fields[i++];
        m.genre_names = parseList(fields[i++]);
        m.production_company_names = parseList(fields[i++]);
        m.budget = std::stod(fields[i++]);
        m.revenue = std::stod(fields[i++]);
        m.runtime = std::stoi(fields[i++]);
        m.popularity = std::stod(fields[i++]);
        m.vote_average = std::stod(fields[i++]);
        m.vote_count = std::stoi(fields[i++]);

        movies.push_back(m);
    }

    return movies;
}

int main() {
    std::string filename = "data_movies_clean.csv";

    std::vector<Movie> movies = loadMoviesFromCSV(filename);

    std::cout << "Loaded " << movies.size() << " movies.\n";

    if (!movies.empty()) {
        std::cout << "\nFirst movie loaded:\n";
        std::cout << "ID: " << movies[0].id << "\n";
        std::cout << "Title: " << movies[0].title << "\n";
        std::cout << "Language: " << movies[0].original_language << "\n";
        std::cout << "Budget: " << movies[0].budget << "\n";
    }

    return 0;
}
