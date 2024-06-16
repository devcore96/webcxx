CXXFLAGS=-Wall -std=c++23 -O3 -ffast-math -I.

OBJECTS=.out/app/kernel/Main.o \
		.out/app/services/Env.o \
		.out/app/services/Json.o \
		.out/app/services/Router.o \
		.out/routes/Routes.o \

LIBS=-lcgicc \
	 -lpng \
	 -lcurl \
	 $(if $(echo "int main() { return 0; }" | $(CXX) -xc++ - -lstdc++exp -o has_stdc++exp && rm has_stdc++exp), -lstdc++exp, ) \

.out/%.o: %.cpp
	mkdir -p `dirname $@` && $(CXX) $(CXXFLAGS) -c -o $@ $<

.out/%.o: %.cc
	mkdir -p `dirname $@` && $(CXX) $(CXXFLAGS) -c -o $@ $<

all: $(SYSTEM_HEADERS) $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o index.cgi $(OBJECTS) $(LIBS)

clean:
	rm -rf $(OBJECTS) .out index.cgi
