Related Emscripten ticket: https://github.com/emscripten-core/emscripten/issues/23337

First build and run without optimization, note the terminal output:

```
emcc main.c -o main.js
node main.js
>> I am JS code.
>> I am C code.

>> I am JS code.
>> I am C code.

>> I am JS code.
>> I am C code.
```

...now build and run with optimization:

```
emcc main.c -o main.js -O1
node main.js
>> I am JS code.
>> I am C code.

>> I am C code.

>> I am C code.
```

Note how the manually patched JS shim is no longer called after the first invocation.

Here's why it happens (all in the generated main.js):

Without optimization, the `Module['_cfunc']` item is initialized with the result
of `createExportWrapper('cfunc', 0);`:

```js
var _cfunc = Module['_cfunc'] = createExportWrapper('cfunc', 0);
```

...with optimization this same line looks mighty weird, and causes the assignment
to happen again with each invocation:

```js
var _cfunc = Module['_cfunc'] = () => (_cfunc = Module['_cfunc'] = wasmExports['cfunc'])();
```

...manually changing this line like this makes it work as expected:

```js
var _cfunc = Module['_cfunc'] = () => (wasmExports['cfunc'])();
```

...so it looks like there's something weird going on with the JS optimizer that's
used on -O (at first I thought it's a Closure thing, but apparently not).