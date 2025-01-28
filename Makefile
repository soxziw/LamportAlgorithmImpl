.PHONY: all clean build tests release

build:
	@mkdir -p build
	@CC=/usr/bin/clang CXX=/usr/bin/clang++ cmake -B build -S .
	@make -C build build | grep -vE "make\[[0-9]+\]"

clean:
	@mkdir -p build
	@CC=/usr/bin/clang CXX=/usr/bin/clang++ cmake -B build -S . > /dev/null 2>&1
	@make -C build cleanup | grep -vE "make\[[0-9]+\]"
	@rm -rf build