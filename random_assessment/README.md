# Assessment of the robustness of randomness

This folder contains the source code and binaries used to assess the robustness of WebAssembly runtimes implementations when using the `random_get` function.

The following commands assume that the runtimes to evaluate are already installed, along with the [wasi-sdk](https://github.com/WebAssembly/wasi-sdk/) in `/opt`. 

Compile with:

```
clang seccomp.c -lseccomp -o seccomp
/opt/wasi-sdk/bin/clang -fstack-protector-all poc.c -o poc.wasm
```

Then use the resulting binaries as explained in the paper.

For assessing M1 (only blocking the `getrandom` Linux syscall), run `./seccomp <tested runtime> poc.wasm`.

For assessing M2 (blocking access to all of the `/dev` folder):
- Enter a user namespace with `unshare -mUr`. You should see that you are now root in the new namespace.
- Run `mkdir /tmp/empty`
- Run `mount -bind /tmp/empty /dev`
- Run `./seccomp <tested runtime> poc.wasm`

