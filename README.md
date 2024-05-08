# NullTrace-Android-Injector
# Inject shared libraries on any process in android

Successful shared library injections tested on real devices and emulators (handles nativebridge for arm lib injections on emulator)

# Building:
1: Get the Android-NDK: https://developer.android.com/ndk/downloads?hl=en
and make a new environment variable called NDK and link the root NDK directory to it, 
Or manually adjust the NDK and CMake_Toolchain paths here: 
[Link NDK](https://github.com/0NullBit0/NullTrace-Android-Inject/blob/main/CMakeLists.txt#L7)
[Link CMAKE_TC](https://github.com/0NullBit0/NullTrace-Android-Inject/blob/main/CMakeLists.txt#L8)

Install CMake and Ninja and make sure the Path for both is set<br>
https://cmake.org/download/
https://ninja-build.org/

2: On Windows: run the build.bat file in powershell by: ```.\build.bat```

On Unix: ```chmod +x build.sh``` and run the build.sh file by:
```./build.sh``` or ```bash build.sh``` or ```sh build.sh```

3: The executables will be inside ```build_out/{arch}/NullTrace-Injector```



# Running: 
```
su
chmod 777 NullTrace-Injector
chmod 777 (libraryname).so
./NullTrace-Injector -p (packagename) -lib (library path)
```
flags:
```
-p flag : provide package name like com.nullbit.pocinject
-lib flag : provide full library path like /data/local/tmp/libhacklib.so
```

![showcase](nulltrace-demo.png)

# Emulator injection showcase:
# https://www.youtube.com/watch?v=Og3SngzD6TI

