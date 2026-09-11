#  System Overview

The system implements a specialized two-phase approach for high-dimensional data retrieval and similarity analysis:

### Phase 1: Spatial Filtering
Execution of **Range Queries** across 5 numerical dimensions:
* **Budget**, **Revenue**, **Runtime**, **Release Year**, and **Vote Average**.

### Phase 2: Similarity Search
Textual similarity search on titles, genres, and production companies using the **LSH (Locality-Sensitive Hashing)** technique with **MinHash** and **3-shingles**.

---

##  Implemented Data Structures

The project focuses on comparing the efficiency of various multidimensional indexing structures:

### 1. k-d Tree (k=5)
* **Balancing:** Uses a recursive axis-cycling approach.
* **Optimization:** Implements `std::nth_element` during construction to ensure a perfectly balanced tree (median splitting).
* **Usage:** Efficient for point and range searches in 5D space.

### 2. R-tree
* **Organization:** Groups data into **Minimum Bounding Rectangles (MBRs)**.
* **Pruning:** Optimized for spatial queries by quickly discarding (pruning) entire branches of the tree that do not intersect with the query range.

### 3. 2D Range Tree
* **Hybrid Structure:** Combines nested Binary Search Trees (BSTs) for the primary dimensions.
* **Filtering:** Performs initial spatial filtering on key dimensions and applies linear filtering for the remaining attributes to maintain a balance between memory usage and search speed.

### 4. Quadtree (Hyperoctree)
* **Dimensionality:** A generalization of the Quadtree into 5 dimensions, resulting in a **Hyperoctree** with $2^5 = 32$ children per internal node.
* **Optimization:** Utilizes a custom **bucket size** to limit tree depth and improve cache locality during traversal.

---

##  Similarity Search (LSH)
To handle non-numerical data, the system implements:
* **Shingling:** Breaking text into 3-character sequences (3-shingles).
* **MinHashing:** Reducing large sets of shingles into short signatures while preserving Jaccard similarity.
* **LSH:** Using hashing bands to find candidate similar pairs in sub-linear time.

## 📊 Performance Analysis
The project includes a benchmarking suite to measure:
* **Construction Time:** Time taken to build each index.
* **Query Latency:** Response time for range queries of varying sizes.
* **Memory Footprint:** Space complexity comparison between the Quadtree, R-tree, and k-d Tree.
