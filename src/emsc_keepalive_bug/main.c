#include <stdio.h>
#include <emscripten/emscripten.h>

EMSCRIPTEN_KEEPALIVE void cfunc(void) {
    printf(">> I am C code.\n\n");
}

int main() {
    // replace the cfunc function object on Module with a Javascript shim
    // which first prints some text and then calls the original function
    EM_ASM({
        const cfunc = Module['_cfunc'];
        Module['_cfunc'] = () => {
            console.log('>> I am JS code.');
            cfunc();
        }
    });
    // call that function a couple of times from js this works as expected
    // without Closure pass, but with the Closure pass enabled the
    // Javascript shim is only called once and resets the Module['_cfunc'] item
    // after the first call
    EM_ASM({
        for (let i = 0; i < 3; i++) {
            Module['_cfunc']();
        }
    });

}