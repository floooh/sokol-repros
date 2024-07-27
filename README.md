# sokol-repros

Common repository for sokol-related bug reproduders.

## Clone, Build and Run

With `cmake`, `emcmake` and `emrun` in the path:

```
git clone https://github.com/floooh/sokol-repros.git && cd sokol-repros
mkdir build && cd build
emcmake cmake ..
cmake --build .
emrun chrome127-offscreen.html
```

Emrun launches the system default browser, might want to explitly open Chrome
on that same URL.
