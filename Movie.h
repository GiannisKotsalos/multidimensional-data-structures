#ifndef MOVIE_H
#define MOVIE_H

#include <string>
#include <vector>

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

#endif