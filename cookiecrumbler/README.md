# Cookie crumbler for Wasm

Implementation of cookie crumbler for Wasm. See https://bierbaumer.net/security/cookie/

Changes : 

- remove the signals part (detecting if memory is writable), useless in Wasm
- fix detection of the cookie ref location
- remove threads measuring

Compile with :

```
/opt/wasi-sdk/bin/clang --target=wasm32-wasi cookie-wasm.c -o cookie.wasm
/opt/wasi-sdk/bin/clang --target=wasm32-wasi -Wl,--stack-first cookie-wasm.c -o cookie-stack-first.wasm

```

Run with :

```
wasmtime run cookie.wasm
```

Files :

cookiecrumbler.c is the ref file 
cookie-wasm.c is the modified file
cookie.wasm and cookie-stack-first.wasm are the resulting binaries
