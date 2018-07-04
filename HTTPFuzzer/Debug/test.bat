@echo off
for /L %%x in (1,1,10) do (
   start HTTPFuzzer.exe
   ping 127.0.0.1 -n 1 > nul
)