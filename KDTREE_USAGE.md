# KDTree Operations Usage Guide

## How to Delete (Remove) a Point

### Method 1: Using Point Object
```cpp
// Convert Movie to Point<5>
Point<5> moviePoint = movieToPoint(movie);

// Delete the point
bool success = tree.remove(moviePoint);
if (success) {
    cout << "Movie deleted successfully!" << endl;
}
```

### Method 2: Using Coordinates Array and ID
```cpp
// Delete using coordinates and movie ID
array<double, 5> coords = {1900.0, 6.37, 4.0, 1.0, 2426.7};
int movieId = 195554;
bool success = tree.remove(coords, movieId);
```

### Method 3: Delete by Movie ID (after finding it)
```cpp
// Find the movie first, then delete
for (const auto& movie : movies) {
    if (movie.id == targetMovieId) {
        Point<5> pt = movieToPoint(movie);
        tree.remove(pt);
        break;
    }
}
```

---

## How to Update a Point

### Method 1: Using Point Objects
```cpp
// Get the old point
Point<5> oldPoint = movieToPoint(movie);

// Modify the movie
movie.popularity += 10.0;
movie.vote_average += 1.0;

// Create new point
Point<5> newPoint = movieToPoint(movie);

// Update in tree
bool success = tree.update(oldPoint, newPoint);
```

### Method 2: Using Arrays and IDs
```cpp
// Old coordinates and ID
array<double, 5> oldCoords = {1900.0, 6.37, 4.0, 1.0, 2426.7};
int oldId = 195554;

// New coordinates (modified values)
array<double, 5> newCoords = {1900.0, 16.37, 5.0, 1.0, 2426.7};
int newId = 195554;  // Usually keep the same ID

// Update
bool success = tree.update(oldCoords, oldId, newCoords, newId);
```

---

## Complete CRUD Operations Summary

### 1. **INSERT** - Add a point
```cpp
// From Movie
Point<5> pt = movieToPoint(movie);
tree.insert(pt);

// Or directly with coordinates
tree.insert({1900.0, 6.37, 4.0, 1.0, 2426.7}, movieId);
```

### 2. **SEARCH** - Check if point exists
```cpp
// Using Point
Point<5> pt = movieToPoint(movie);
bool exists = tree.search(pt);

// Using coordinates and ID
bool exists = tree.search({1900.0, 6.37, 4.0, 1.0, 2426.7}, movieId);
```

### 3. **DELETE** - Remove a point
```cpp
// Using Point
Point<5> pt = movieToPoint(movie);
tree.remove(pt);

// Using coordinates and ID
tree.remove({1900.0, 6.37, 4.0, 1.0, 2426.7}, movieId);
```

### 4. **UPDATE** - Change a point
```cpp
// Using Points (delete old, insert new)
Point<5> oldPt = movieToPoint(oldMovie);
Point<5> newPt = movieToPoint(newMovie);
tree.update(oldPt, newPt);

// Using arrays
tree.update(oldCoords, oldId, newCoords, newId);
```

---

## Important Notes

1. **Point Matching**: The `search`, `remove`, and `update` operations match points based on BOTH coordinates AND ID. Two points with the same coordinates but different IDs are considered different.

2. **Update Behavior**: The `update` function actually performs a delete + insert operation to maintain the proper k-d tree structure. This is necessary because changing coordinates might require repositioning in the tree.

3. **Return Values**:
   - `insert()`: void (always succeeds)
   - `search()`: returns `bool` (true if found)
   - `remove()`: returns `bool` (true if deleted)
   - `update()`: returns `bool` (true if updated)

4. **Finding Movies by ID**: If you want to delete/update a movie by ID only, you'll need to search through your movies vector first to get the Movie object, then convert it to a Point.

