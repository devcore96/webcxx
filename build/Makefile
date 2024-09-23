CXXFLAGS=-std=c++26 -Ofast -ffast-math -I. -w -g

OBJECTS=.out/Terminal.o \
		.out/ProgressBar.o \
		.out/CompileUnit.o \
		.out/Target.o \
		.out/Test.o \
		.out/BuildManager.o \
		.out/Main.o \

all: $(OBJECTS) 
	@$(CXX) $(CXXFLAGS) -o build $(OBJECTS)
	
.out/%.o: %.cpp
	@$(CXX) $(CXXFLAGS) -c -o $@ $<;

clean:
	rm -rf $(OBJECTS) build .out/
