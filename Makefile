CXX      := g++
CXXFLAGS := -std=c++11 -O2 -Wall -Wextra -pedantic
TARGET   := sistema_solar
SRC      := src/main.cpp src/engine.cpp src/render.cpp src/input.cpp
LIBS     := -lGL -lGLU -lglut

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(LIBS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) screenshot.ppm screenshot.png
