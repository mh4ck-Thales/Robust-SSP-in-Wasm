# Assessment of the robustness of randomness

This folder contains the source code and binaries used to assess the robustness of WebAssembly runtimes implementations when using the `random_get` function.

Compile with:

```
clang seccomp.c -lseccomp -o seccomp
/opt/wasi-sdk/bin/clang -fstack-protector-all poc.c -o poc.wasm
```

Then use the resulting binaries as explained in the paper.

