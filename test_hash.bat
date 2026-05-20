@echo off
g++ -std=c++17 C:\Users\proto\.gemini\antigravity-ide\brain\66a03a76-b2e0-48f7-ad9f-fd9a881b7723\scratch\test_hash.cpp -o test_hash.exe -lbcrypt
if %errorlevel% neq 0 exit /b %errorlevel%
test_hash.exe
