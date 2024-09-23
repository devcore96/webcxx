all: bootstrap
	@mkdir -p .out
	@./build/build

bootstrap:
	@if [ ! -f ./build/build ]; then \
    	echo "Bootstrapping build system..."; \
		mkdir -p ./build/.out; \
	fi
	@make -j 8 -C build -s

clean:
	@#./build/build clean
	@rm -rf .out jobs tests
