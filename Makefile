CXXFLAGS=-std=c++23 -O3 -ffast-math -I. -w -DMYSQLPP_MYSQL_HEADERS_BURIED

MAIN=.out/app/kernel/Main.o \

BASE_OBJECTS=.out/app/services/env/Env.o \
			 .out/app/services/rest/Rest.o \
		     .out/app/services/serialization/Model.o \
		     .out/app/services/serialization/Json.o \
		     .out/app/services/router/Router.o \
		     .out/app/services/router/RouteTypes.o \
			 .out/app/services/database/Table.o \
		     .out/routes/Web.o \
		     .out/routes/Api.o \

MYSQL_OBJECTS=.out/app/services/mysql/Connection.o \
              .out/app/services/mysql/Model.o \

BASE_TESTS=.out/app/tests/serialization/serialize-json.test \
           .out/app/tests/serialization/deserialize-json.test \
           .out/app/tests/serialization/serialize-model.test \
           .out/app/tests/serialization/deserialize-model.test \

MYSQL_TESTS=

BASE_LIBS=-lcgicc \
	      -lpng \
	      -lcurl \

MYSQL_LIBS=-lmysqlcppconn -lmysqlcppconn8 \

ifndef DISABLE_MYSQL
OBJECTS=$(MAIN) $(BASE_OBJECTS) $(MYSQL_OBJECTS)
TEST_OBJECTS=$(BASE_OBJECTS) $(MYSQL_OBJECTS)
TESTS=$(BASE_TESTS) $(MYSQL_TESTS)
LIBS=$(BASE_LIBS) $(MYSQL_LIBS)
else
OBJECTS=$(MAIN) $(BASE_OBJECTS)
TEST_OBJECTS=$(BASE_OBJECTS)
TESTS=$(BASE_TESTS)
LIBS=$(BASE_LIBS)
endif

.out/%.o: %.cpp
	@echo "\e[1A\e[K[`echo $(OBJECTS) | awk 'NR>0' RS=' ' | grep -n $@ | awk 'NR==1' RS=':'`/`echo $(OBJECTS) | awk 'NR>0' RS=' ' | wc -l`] compiling $<..."
	@if mkdir -p `dirname $@` && $(CXX) $(CXXFLAGS) -c -o $@ $<; \
	then \
		echo "\e[1A\e[K[`echo $(OBJECTS) | awk 'NR>0' RS=' ' | grep -n $@ | awk 'NR==1' RS=':'`/`echo $(OBJECTS) | awk 'NR>0' RS=' ' | wc -l`] \e[32mok\e[39m $<"; \
	else \
		echo "[`echo $(OBJECTS) | awk 'NR>0' RS=' ' | grep -n $@ | awk 'NR==1' RS=':'`/`echo $(OBJECTS) | awk 'NR>0' RS=' ' | wc -l | xargs expr -1 + `] \e[31mfail\e[39m $<"; \
		exit 1; \
	fi

.out/%.test: %.cpp
	@echo "\e[1A\e[K[`echo $(TESTS) | awk 'NR>0' RS=' ' | grep -n $@ | awk 'NR==1' RS=':'`/`echo $(TESTS) | awk 'NR>0' RS=' ' | wc -l | xargs expr -1 + `] compiling test $<..."
	@mkdir -p `dirname $@` && $(CXX) $(CXXFLAGS) -o $@ $(TEST_OBJECTS) $< $(LIBS)
	@echo "\e[1A\e[K[`echo $(TESTS) | awk 'NR>0' RS=' ' | grep -n $@ | awk 'NR==1' RS=':'`/`echo $(TESTS) | awk 'NR>0' RS=' ' | wc -l | xargs expr -1 + `] running test $<..."
	@if $@; \
	then \
		echo "\e[1A\e[K[`echo $(TESTS) | awk 'NR>0' RS=' ' | grep -n $@ | awk 'NR==1' RS=':'`/`echo $(TESTS) | awk 'NR>0' RS=' ' | wc -l | xargs expr -1 + `] \e[32mpass\e[39m $<"; \
	else \
		echo "[`echo $(TESTS) | awk 'NR>0' RS=' ' | grep -n $@ | awk 'NR==1' RS=':'`/`echo $(TESTS) | awk 'NR>0' RS=' ' | wc -l | xargs expr -1 + `] \e[31mfail\e[39m $<"; \
		rm $@; \
		exit 1; \
	fi

all: $(SYSTEM_HEADERS) $(OBJECTS)
	@echo "\e[1A\e[K[`echo $(OBJECTS) | awk 'NR>0' RS=' ' | wc -l`/`echo $(OBJECTS) | awk 'NR>0' RS=' ' | wc -l`] compiling index.cgi..."
	@if (echo "#include <stacktrace>\n#include <iostream>\nint main() { std::cout << std::stacktrace::current() << std::endl; return 0; }" | $(CXX) -xc++ - -o has_stdc++exp -std=c++23 -lstdc++exp) >> /dev/null 2> /dev/null; \
	then \
		rm has_stdc++exp; \
		if $(CXX) $(CXXFLAGS) -o index.cgi $(OBJECTS) $(LIBS) -lstdc++exp; \
		then \
			echo "\e[1A\e[K[`echo $(OBJECTS) | awk 'NR>0' RS=' ' | wc -l`/`echo $(OBJECTS) | awk 'NR>0' RS=' ' | wc -l`] \e[32mok\e[39m index.cgi"; \
		else \
			echo "[`echo $(OBJECTS) | awk 'NR>0' RS=' ' | wc -l`/`echo $(OBJECTS) | awk 'NR>0' RS=' ' | wc -l`] \e[31mfail\e[39m index.cgi"; \
			exit 1; \
		fi \
	else \
		if $(CXX) $(CXXFLAGS) -o index.cgi $(OBJECTS) $(LIBS); \
		then \
			echo "\e[1A\e[K[`echo $(OBJECTS) | awk 'NR>0' RS=' ' | wc -l`/`echo $(OBJECTS) | awk 'NR>0' RS=' ' | wc -l`] \e[32mok\e[39m index.cgi"; \
		else \
			echo "[`echo $(OBJECTS) | awk 'NR>0' RS=' ' | wc -l`/`echo $(OBJECTS) | awk 'NR>0' RS=' ' | wc -l`] \e[31mfail\e[39m index.cgi"; \
			exit 1; \
		fi \
	fi

test: $(TEST_OBJECTS) $(TESTS)

clean:
	rm -rf $(OBJECTS) .out index.cgi
