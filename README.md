Stanford CS 144 Networking Lab
==============================

These labs are open to the public under the (friendly) request that to
preserve their value as a teaching tool, solutions not be posted
publicly by anybody.

Website: https://cs144.stanford.edu

To set up the build system: `cmake -S . -B build`

To compile: `cmake --build build`

若出现错误，考虑脚本`minnow/scripts/make-parallel.sh`是 Windows 格式（CRLF）,转换为linux格式，其他脚本中也可能出现相同换行问题，使用类似方法解决
```bash
sudo apt update
sudo apt install dos2unix -y
dos2unix ~/minnow/scripts/make-parallel.sh
```

To run tests: `cmake --build build --target test`

To run speed benchmarks: `cmake --build build --target speed`

To run clang-tidy (which suggests improvements): `cmake --build build --target tidy`

To format code: `cmake --build build --target format`
