# bg-generation-triangle [![C++ CI](https://github.com/edmBernard/bg-generation-triangle/actions/workflows/cpp.yml/badge.svg)](https://github.com/edmBernard/bg-generation-triangle/actions/workflows/cpp.yml)

Small executable that generate fractal triangle pattern and save it in svg.

- Github : [https://github.com/edmBernard/bg-generation-triangle](https://github.com/edmBernard/bg-generation-triangle)

## Generated command

- `bg-generation-triangle-regular` : Regular subdivision of triangles
- `bg-generation-triangle-pleasing` : Pleasing subdivision based on [this blog](https://tylerxhobbs.com/essays/2017/aesthetically-pleasing-triangle-subdivision)

## Dependencies

We use [vcpkg](https://github.com/Microsoft/vcpkg) to manage dependencies

This project depends on:
- [cxxopts](https://github.com/jarro2783/cxxopts): Command line argument parsing
- [fmt](https://fmt.dev/latest/index.html): A modern formatting library
- [spdlog](https://github.com/gabime/spdlog): Very fast, header-only/compiled, C++ logging library


```
./vcpkg install spdlog cxxopts fmt
```

### Compilation

```bash
mkdir build
cd build
# configure make with vcpkg toolchain
cmake .. -DCMAKE_TOOLCHAIN_FILE=${VCPKG_DIR}/scripts/buildsystems/vcpkg.cmake
# on Windows : cmake .. -DCMAKE_TOOLCHAIN_FILE=${env:VCPKG_DIR}/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release
```

## Disclaimer

It's a toy project. So if you spot error, improvement comments are welcome.
