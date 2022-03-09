set(CMAKE_HOST_SYSTEM "Linux-5.13.0-30-generic")
set(CMAKE_HOST_SYSTEM_NAME "Linux")
set(CMAKE_HOST_SYSTEM_VERSION "5.13.0-30-generic")
set(CMAKE_HOST_SYSTEM_PROCESSOR "x86_64")

include("/ITKWebAssemblyInterface/toolchain.cmake")

set(CMAKE_SYSTEM "WASI-1")
set(CMAKE_SYSTEM_NAME "WASI")
set(CMAKE_SYSTEM_VERSION "1")
set(CMAKE_SYSTEM_PROCESSOR "wasm32")

set(CMAKE_CROSSCOMPILING "TRUE")

set(CMAKE_SYSTEM_LOADED 1)
