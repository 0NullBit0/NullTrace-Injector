# NullTrace-Android-Inject
Inject and control processes remotely on android multi arch emulator supported

**Features:**
- Read and write process memory
- Remotely call functions in target process
- inject libraries
- hooking (soon)

Multi arch support + tested on emulator 

Setup:
[Change ANDROID_NDK path](./NullInject/CMakeLists.txt)
[Change CMAKE_TOOLCHAIN_FILE path](./NullInject/CMakeLists.txt)
On Windows run build.bat on other oses create a similar script and run

Executable will be located at outputs or you can use NullTrace seperately as a library for different projects

Emulator inject showcase:
# [Video]https://www.youtube.com/watch?v=wfjUcNLGZdw
