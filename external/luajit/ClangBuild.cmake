# Adapt only the disposable build copy of LuaJIT's upstream Windows script.
# clang-cl is Clang's MS-compatible driver; the engine itself uses clang++.
file(READ "${BUILD_ROOT}/src/msvcbuild.bat" SCRIPT)
string(REPLACE "@set LJCOMPILE=cl " "@set LJCOMPILE=\"${LLVM_BIN}/clang-cl.exe\" " SCRIPT "${SCRIPT}")
string(REPLACE "@set LJLINK=link " "@set LJLINK=\"${LLVM_BIN}/lld-link.exe\" " SCRIPT "${SCRIPT}")
string(REPLACE "@set LJLIB=lib " "@set LJLIB=\"${LLVM_BIN}/llvm-lib.exe\" " SCRIPT "${SCRIPT}")
file(WRITE "${BUILD_ROOT}/src/clangbuild.bat" "${SCRIPT}")
