COMPILER := g++ 
FLAGS := -w -g 
SOURCE := main.cc chip8.cc chip8.hpp
OUTPUT := chip8
INCLUDE := -Ideps/SDL2
LINK := -lSDL2main -lSDL2 -lm 

all: 
	$(COMPILER) $(SOURCE) $(FLAGS) -o $(OUTPUT) $(INCLUDE) $(LINK)

