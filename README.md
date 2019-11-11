# clang-fco
clang tool for source-to-source floating-point code optimization

## Build

Get the sources and build __clang__ as explained
[here](http://clang.llvm.org/get_started.html) (steps 1, 2, 3, 5, and
6), but add the following step between 2 and 3:

```bash
cd llvm-project/clang-tools-extra
echo "add_subdirectory(clang-fco)" >> CMakelist.txt
git clone https://github.com/thvnx/clang-fco.git
cd ../..
```

## Usage

```bash
./clang-fco source.c --
```
