CXX = g++
CXXFLAGS = -Wall

# Define the source files
SRC = test.cpp Primitives/bitstr.cpp Primitives/sbox.cpp Primitives/perm.cpp Primitives/feistel.cpp Primitives/trail.cpp Primitives/attack.cpp Primitives/trail_adv.cpp Primitives/bool_fn.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = test

# Default target
all: $(TARGET)

# Link the object files to create the executable
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile the source files into object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Subdirectory rule for primitives
primitives/%.o: primitives/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up the build files
clean:
	rm -f *.o $(TARGET)
