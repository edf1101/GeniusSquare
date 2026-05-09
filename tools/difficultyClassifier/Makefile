CXX      = g++
CXXFLAGS = -std=c++17 -O2 -I.

SOLVER_SRCS = main.cpp \
              solver/Board.cpp \
              solver/Piece.cpp \
              solver/Dice.cpp \
              solver/CombinationGenerator.cpp

SOLVER = genius_solver
CLASSIFIER = classifier
GENERATOR = generate_difficulty_data

all: $(SOLVER) $(CLASSIFIER) $(GENERATOR)

$(SOLVER): $(SOLVER_SRCS)
	$(CXX) $(CXXFLAGS) -o $(SOLVER) $(SOLVER_SRCS)

$(CLASSIFIER): classifier.cpp
	$(CXX) $(CXXFLAGS) -o $(CLASSIFIER) classifier.cpp

$(GENERATOR): generate_difficulty_data.cpp
	$(CXX) $(CXXFLAGS) -o $(GENERATOR) generate_difficulty_data.cpp

clean:
	rm -f $(SOLVER) $(CLASSIFIER) $(GENERATOR) results.txt classifications.txt
