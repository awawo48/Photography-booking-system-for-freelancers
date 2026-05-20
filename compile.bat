@echo off
echo Compiling Photography Booking System (Modular)...
g++ -std=c++17 -I"C:\Program Files\MariaDB\MariaDB Connector C64\include" -Iinclude ^
    src/main.cpp ^
    src/Database.cpp ^
    src/UIHelper.cpp ^
    src/PDFGenerator.cpp ^
    src/Dashboard/Auth.cpp ^
    src/Dashboard/Admin.cpp ^
    src/Dashboard/Customer.cpp ^
    src/Dashboard/Photographer.cpp ^
    src/Security.cpp ^
    -o modular_app.exe libmariadb.dll -lbcrypt
if %errorlevel% neq 0 (
    echo Compilation FAILED!
    exit /b %errorlevel%
)
echo Compilation SUCCESSFUL! Created modular_app.exe
