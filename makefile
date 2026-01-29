# Ορισμός του compiler
CXX = g++

# Flags για το compilation (-O3 για βελτιστοποίηση ταχύτητας στις αναζητήσεις)
CXXFLAGS = -Wall -std=c++17 -O3

# Το όνομα του εκτελέσιμου
TARGET = run

# Ορισμός των πηγαίων αρχείων (.cpp)
# Περιλαμβάνει τα αρχεία στον κεντρικό κατάλογο και στους υποφακέλους
SRCS = Main.cpp \
       FileReader.cpp \
       KDtree/KDTree.cpp \
       Rtree/Rtree.cpp \
       QuadTree/quadtree.cpp

# Μετατροπή των .cpp αρχείων σε .o (object files)
OBJS = $(SRCS:.cpp=.o)

# Default rule: χτίσιμο του εκτελέσιμου
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)
	@echo "\nCompilation complete! Type './$(TARGET)' to start the program."

# Rule για τη δημιουργία των .o αρχείων
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Καθαρισμός των παραγόμενων αρχείων
clean:
	rm -f $(OBJS) $(TARGET)
	@echo "Cleaned up object files and executable."

# Rule για επανεκκίνηση (clean και build)
re: clean all