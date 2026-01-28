#ifndef FILEREADER_H
#define FILEREADER_H

#include <vector>
#include <string>
#include "Movie.h"

// Only declare the function signature here
std::vector<Movie> loadMoviesFromCSV(const std::string &filename);

#endif