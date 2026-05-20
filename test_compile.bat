@echo off
g++ -std=c++17 -I"C:\Program Files\MariaDB\MariaDB Connector C64\include" C:\Users\proto\.gemini\antigravity-ide\brain\66a03a76-b2e0-48f7-ad9f-fd9a881b7723\scratch\test_stmt.cpp -o test_stmt.exe libmariadb.dll
if %errorlevel% neq 0 exit /b %errorlevel%
test_stmt.exe
