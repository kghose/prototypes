# Lopsided spinning ship


## Compiling

Make a `build` directory and then from there

```
export CC=clang
export CXX=clang++

cmake -DChrono_DIR=/home/kghose/code/chrono/build/cmake/ ../
```

Generate `compile_commands.json` so that `clangd` figures things out.

```
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
```

Make a soft link from the base directory so that `clangd` can find it when we
are editing.

```
ln -s build/compile_commands.json .
```
