@echo off

adb push build_out/x86/NullTrace-Injector /data/local/tmp

echo Press any key to continue . . .
pause > nul