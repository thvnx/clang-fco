# clang-fco
clang tool for source-to-source floating-point code optimization

### Build

Get the sources and build __clang__ as explained
[here](http://clang.llvm.org/get_started.html) (steps 1, 2, 3, 4, and
7), but add the following step between 4 and 7:

```bash
cd llvm/tools/clang/tools/extra
git clone https://github.com/thvnx/clang-fco.git
cd ../../../../..
```

### Usage

```bash
./clang-fco source.c --
```
