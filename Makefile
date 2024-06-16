CXXFLAGS=-Wall -std=c++23 -O3 -ffast-math -I.

OBJECTS=.out/app/kernel/Main.o \
		.out/app/services/Env.o \
		.out/app/services/Json.o \
		.out/app/services/Router.o \
		.out/routes/Routes.o \

LIBS=-lcgicc \
	 -lpng \
	 -lcurl \

.out/%.o: %.cpp
	mkdir -p `dirname $@` && $(CXX) $(CXXFLAGS) -c -o $@ $<

.out/%.o: %.cc
	mkdir -p `dirname $@` && $(CXX) $(CXXFLAGS) -c -o $@ $<

all: $(SYSTEM_HEADERS) $(OBJECTS)
	if `echo "int main() { return 0; }" | $(CXX) -xc++ - -lstdc++exp -o has_stdc++exp && rm has_stdc++exp`; \
	then \
		$(CXX) $(CXXFLAGS) -o index.cgi $(OBJECTS) $(LIBS) -lstdc++exp; \
	else \
		$(CXX) $(CXXFLAGS) -o index.cgi $(OBJECTS) $(LIBS); \
	fi

clean:
	rm -rf $(OBJECTS) .out index.cgi
