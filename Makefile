# Compiler
CXX = g++

# Detect all .cpp files in the directory
SOURCES = $(wildcard *.cpp)

# Object files directory
OBJDIR = obj

# Object files (map each .cpp file to a corresponding .o file in obj/)
OBJECTS = $(patsubst %.cpp, $(OBJDIR)/%.o, $(SOURCES))

# Executable name
EXECUTABLE = tv_parser_app

# Compiler flags
CXXFLAGS = -I. -Idata_structures/ -Idata_interface/ -Wall -std=c++17 -g3 -O0

ifeq ($(USE_ASAN), 1)
  CXXFLAGS += -fsanitize=address
  LDFLAGS += -fsanitize=address
endif

# Default target - build the executable
all: $(EXECUTABLE)

# Rule to link the object files to create the final executable
$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(EXECUTABLE) $(LDFLAGS)

# Ensure obj directory exists before compiling
$(OBJDIR):
	@mkdir -p $(OBJDIR)

# Rule to compile the source files into object files in obj/ directory
$(OBJDIR)/%.o: %.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean target - removes all object files and the executable
clean:
	@rm -rf $(OBJDIR) $(EXECUTABLE)
