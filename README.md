# Robust Stack Smashing Protection in WebAssembly

This repository contains the source code of the tools mentionned in the paper "Robust Stack Smashing Protection in WebAssembly".

- The `cookiecrumbler` folder contains the original source code of the CookieCrumbler tool and our modified version.
- The `poc` folder contains the source code and binaries of our proofs of concept.
- The `random_assessment` contains the source code and binaries for our assessment of the robustness of the `random_get` function implementation.

The source code of our improvements for SSP are available [here](https://github.com/ThalesGroup/llvm-project/tree/new-wasm-ssp) for LLVM and [here](https://github.com/ThalesGroup/wasi-libc/tree/new-wasm-ssp) for wasi-libc.

## License

All the content in this repository is available under the Apache 2.0 license, with the exception of the `cookiecrumbler/cookiecrumbl0r.c` file and the resulting binary `cookiecrumbler/cookiecrumbl0r`, which are Copyright (C) 2018 Bruno Bierbaumer, Julian Kirsch (see the associated website [here](https://bierbaumer.net/security/cookie/)). See more in the [LICENSE](LICENSE).
