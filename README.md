# sokol-repros

Common repository for sokol-related bug reproducers.

## Sample Links

NOTE: the below sample links only work from the Github Pages URL: [https://floooh.github.io/sokol-repros/](https://floooh.github.io/sokol-repros/)

* [Chrome Issue: https://issues.chromium.org/issues/355605685](https://issues.chromium.org/issues/355605685)
    * WASM version: [chrome127-offscreen](deploy/chrome127-offscreen.html)
    * JS version: [chrome127-offscreen-js](js/chrome127-offscreen-js.html)

* [WebGPU Dynamic Offset Oddity](js/wgpu-dyn-offsets.html)

## Clone and Build

With `cmake`, `emcmake` and `emrun` in the path:

```
git clone https://github.com/floooh/sokol-repros.git && cd sokol-repros
mkdir build
emcmake cmake -B build
cmake --build build
```

Build results will go to a sibling directory of build `deploy/`

## Run

From the project root dir:
```
cd deploy
emrun chrome127-offscreen.html
```

Emrun launches the system default browser, might want to explitly open Chrome
on that same URL.

## Reproducers:

### src/chrome127-offscreen.c

Chrome ticket: https://issues.chromium.org/issues/35560568

Sample link: (todo)

Renders a quad on a grey background.

Expected: The color of the quad must changes from red to yellow.

Bug: The color of the quad stays red, and doesn't cycle to yello.
