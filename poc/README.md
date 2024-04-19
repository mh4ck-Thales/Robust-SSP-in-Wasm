# Wasm canary PoC

This PoC is demonstrating that one can bypass the SSP mechanism in WebAssembly if the overflow is long enough, by overwriting the reference value in memory.

## Usage

The current exploit and binary files are configured to work out of the box. Just ensure that you have a Wasm runtime installed and in your PATH (e.g. wasmtime), and modify the line with `p = process(...)` to use your Wasm runtime. 

This exploit have been tested with Python 3.11.8 and the librairies detailed in `requirements.txt`.

All .wasm files are compiled from the same `poc.c` source file. The exploit and binary finishing with `no-ssp` are a demonstration of an attack without SSP in place. The ones finishing with `-ssp` are a demonstration of the attack with SSP in place.

You can simply execute the exploits using `python <exploit.py>`.

The compilation commands for the different files are the following, using :

```bash
# for no-spp
/opt/wasi-sdk/bin/clang -g --target=wasm32-wasi -Wl,--stack-first poc.c -O2 -o poc-no-ssp.wasm
# for ssp 
/opt/wasi-sdk/bin/clang -g --target=wasm32-wasi -fstack-protector-all -Wl,--stack-first poc-ssp.c -O2 -o poc.wasm
```

## Debug the exploit 

You **NEED** to have `iwasm` with its debugging features (see [here](https://bytecodealliance.github.io/wamr.dev/blog/wamr-source-debugging-basic/) and [here](https://github.com/bytecodealliance/wasm-micro-runtime/blob/main/doc/source_debugging_interpreter.md)) to reproduce the exploit on a modified binary. 

Then, you need to recreate a stack dump and put it in `ref.bin`. To do this, start `iwasm` in debug mode, then connect to it with lldb. Then :

- put a breakpoint on main with `b main`
- continue the execution to main with `c`
- go line by line until you are on the `fgets` / buffer reading line using `n` several times.
- dump the memory on the stack starting from the user buffer with `memory read -c 4000 -b -o ref.bin --force &user`.

Now you can try to reproduce the exploit by starting it.
