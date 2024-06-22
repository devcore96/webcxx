CXXFLAGS=-std=c++23 -O3 -ffast-math -I.

BASE_OBJECTS=.out/app/kernel/Main.o \
		     .out/app/services/env/Env.o \
			 .out/app/services/rest/Rest.o \
		     .out/app/services/serialization/Json.o \
		     .out/app/services/router/Router.o \
		     .out/app/services/router/RouteTypes.o \
		     .out/routes/Web.o \
		     .out/routes/Api.o \

MYSQL_OBJECTS=

BASE_LIBS=-lcgicc \
	      -lpng \
	      -lcurl \

MYSQL_LIBS=-lmysqlcppconn \

ifndef DISABLE_MYSQL
OBJECTS=$(BASE_OBJECTS) $(MYSQL_OBJECTS)
LIBS=$(BASE_LIBS) $(MYSQL_LIBS)
else
OBJECTS=$(BASE_OBJECTS)
LIBS=$(BASE_LIBS)
endif

.out/%.o: %.cpp
	@echo [`echo $(OBJECTS) | awk 'NR>0' RS=' ' | grep -n $@ | awk 'NR==1' RS=':'`/`echo $(OBJECTS) | awk 'NR>0' RS=' ' | wc -l`] compiling $<...
	@mkdir -p `dirname $@` && $(CXX) $(CXXFLAGS) -c -o $@ $<

.out/%.o: %.cc
	@echo [`echo $(OBJECTS) | awk 'NR>0' RS=' ' | grep -n $@ | awk 'NR==1' RS=':'`/`echo $(OBJECTS) | awk 'NR>0' RS=' ' | wc -l`] compiling $<...
	@mkdir -p `dirname $@` && $(CXX) $(CXXFLAGS) -c -o $@ $<

all: $(SYSTEM_HEADERS) $(OBJECTS)
	@echo [`echo $(OBJECTS) | awk 'NR>0' RS=' ' | wc -l`/`echo $(OBJECTS) | awk 'NR>0' RS=' ' | wc -l`] compiling index.cgi...
	@if (echo "#include <stacktrace>\n#include <iostream>\nint main() { std::cout << std::stacktrace::current() << std::endl; return 0; }" | $(CXX) -xc++ - -o has_stdc++exp -std=c++23 -lstdc++exp) >> /dev/null 2> /dev/null; \
	then \
		rm has_stdc++exp; \
		$(CXX) $(CXXFLAGS) -o index.cgi $(OBJECTS) $(LIBS) -lstdc++exp; \
	else \
		$(CXX) $(CXXFLAGS) -o index.cgi $(OBJECTS) $(LIBS); \
	fi

clean:
	rm -rf $(OBJECTS) .out index.cgi
