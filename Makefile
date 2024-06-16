CXXFLAGS=-Wall -std=c++23 -O3 -ffast-math -I.

OBJECTS=.out/app/kernel/Main.o \
		.out/app/services/Env.o \
		.out/app/services/Json.o \
		.out/app/services/Router.o \
		.out/routes/Routes.o \

LIBS=-lcgicc \
     -lstdc++exp \
	 -lpng \
	 -lcurl \

.out/%.o: %.cpp
	mkdir -p `dirname $@` && $(CXX) $(CXXFLAGS) -c -o $@ $<

.out/%.o: %.cc
	mkdir -p `dirname $@` && $(CXX) $(CXXFLAGS) -c -o $@ $<

all: $(SYSTEM_HEADERS) $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o index.cgi $(OBJECTS) $(LIBS)

clean:
	rm -rf $(OBJECTS) .out index.cgi
