#include "Dashboard/Auth.hpp"
#include "Dashboard/Admin.hpp"
#include "Dashboard/Customer.hpp"
#include "Dashboard/Photographer.hpp"
#include "UIHelper.hpp"
#include "Database.hpp"
#include "CodeGenerator.hpp"
#include "Security.hpp"
#include <iostream>
#include <windows.h>

using namespace std;

static bool isNameUnique(MYSQL* conn, const string& name) {
    string q = "SELECT COUNT(*) FROM USERS WHERE name = ?";
    auto res = DBHelper::executeQuery(conn, q, {name});
    if (!res.empty() && !res[0].empty()) {
        return res[0][0] == "0";
    }
    return true;
}

static bool isEmailUnique(MYSQL* conn, const string& email) {
    string q = "SELECT COUNT(*) FROM USERS WHERE email = ?";
    auto res = DBHelper::executeQuery(conn, q, {email});
    if (!res.empty() && !res[0].empty()) {
        return res[0][0] == "0";
    }
    return true;
}

static bool isValidEmailFormat(const string& email) {
    size_t atPos = email.find('@');
    if (atPos == string::npos || atPos == 0 || atPos == email.length() - 1) {
        return false;
    }
    if (email.find('@', atPos + 1) != string::npos) {
        return false;
    }
    size_t dotPos = email.find('.', atPos + 1);
    if (dotPos == string::npos || dotPos == atPos + 1 || dotPos == email.length() - 1) {
        return false;
    }
    return true;
}

static bool isValidPhoneFormat(const string& phone) {
    if (phone.empty()) return false;
    size_t start = 0;
    if (phone[0] == '+') start = 1;
    
    int digitCount = 0;
    for (size_t i = start; i < phone.length(); ++i) {
        if (phone[i] == '-' || phone[i] == ' ') continue;
        if (!isdigit(phone[i])) return false;
        digitCount++;
    }
    return digitCount >= 8 && digitCount <= 15;
}

void registerUser(MYSQL* conn) {
    showScreenHeader("REGISTRATION");
    
    string name;
    while (true) {
        name = getInput("Enter Name (or press ESC to cancel): ");
        if (name.empty()) {
            showMsg("Registration cancelled.", "err");
            waitKey(); return;
        }
        
        bool allSpaces = true;
        for (char c : name) {
            if (!isspace(c)) { allSpaces = false; break; }
        }
        if (allSpaces) {
            showMsg("Name cannot consist of only spaces.", "err");
            cout << endl;
            continue;
        }

        if (!isNameUnique(conn, name)) {
            showMsg("Name is already taken by another user.", "err");
            cout << endl;
        } else {
            break;
        }
    }

    string email;
    while (true) {
        email = getInput("Enter Email (or press ESC to cancel): ");
        if (email.empty()) {
            showMsg("Registration cancelled.", "err");
            waitKey(); return;
        }

        if (!isValidEmailFormat(email)) {
            showMsg("Invalid email format (must be user@example.com).", "err");
            cout << endl;
        } else if (!isEmailUnique(conn, email)) {
            showMsg("Email is already registered.", "err");
            cout << endl;
        } else {
            break;
        }
    }

    string password;
    while (true) {
        password = getPasswordInput("Enter Password (or press ESC to cancel): ");
        if (password.empty()) {
            showMsg("Registration cancelled.", "err");
            waitKey(); return;
        }

        if (password.length() < 6) {
            showMsg("Password must be at least 6 characters long.", "err");
            cout << endl;
        } else {
            break;
        }
    }

    string phone;
    while (true) {
        phone = getInput("Enter Phone Number (or press ESC to cancel): ");
        if (phone.empty()) {
            showMsg("Registration cancelled.", "err");
            waitKey(); return;
        }

        if (!isValidPhoneFormat(phone)) {
            showMsg("Invalid phone number (must be 8-15 digits, e.g., +60123456789).", "err");
            cout << endl;
        } else {
            break;
        }
    }

    cout << endl;
    vector<string> roles = {"Customer", "Photographer"};
    cout << CLR_BYL << "  Select Role:" << CLR_RS << endl;
    cout << CLR_GY << "    [1] Customer" << endl;
    cout << "    [2] Photographer" << CLR_RS << endl;
    string rc = getInput("Enter choice (1/2): ");
    string role = (rc == "2") ? "Photographer" : "Customer";
    string status = (role == "Photographer") ? "Pending_Verification" : "Active";

    string salt = Security::generateSalt();
    string hashedPwd = Security::hashWithSalt(salt, password);
    string ucode = CodeGenerator::generate(conn, "USR", "USERS", "user_code", false, false);
    string q = "INSERT INTO USERS (user_code, name, email, password, phone_no, role, account_status, salt) VALUES (?, ?, ?, ?, ?, ?, ?, ?)";
    
    if (DBHelper::executeUpdate(conn, q, {ucode, name, email, hashedPwd, phone, role, status, salt}) < 0) {
        showMsg("Registration Failed.", "err");
    } else {
        if (role == "Photographer") {
            string uq = "SELECT user_id FROM USERS WHERE email = ?";
            auto res = DBHelper::executeQuery(conn, uq, {email});
            if (!res.empty()) {
                string uid = res[0][0];
                cls();
                showScreenHeader("PHOTOGRAPHER VERIFICATION");
                showMsg("Please answer the following screening questions to complete your application.", "info");
                cout << endl;
                
                string q1, q2, q3, q4;
                while (true) {
                    q1 = getInput("1. Provide a link to your online portfolio (e.g., Instagram, Google Drive, Website): ");
                    if (q1.empty()) {
                        showMsg("Portfolio link cannot be empty.", "err");
                        cout << endl;
                    } else {
                        break;
                    }
                }
                while (true) {
                    q2 = getInput("2. What is your primary camera body and lens setup? (e.g., Sony a6400 + 18-105mm f/4): ");
                    if (q2.empty()) {
                        showMsg("Camera setup cannot be empty.", "err");
                        cout << endl;
                    } else {
                        break;
                    }
                }
                while (true) {
                    q3 = getInput("3. How many years of professional photography experience do you have?: ");
                    if (q3.empty()) {
                        showMsg("Experience years cannot be empty.", "err");
                        cout << endl;
                    } else {
                        break;
                    }
                }
                while (true) {
                    q4 = getInput("4. Which operational areas/states do you cover? (e.g., Kelantan, Melaka, Klang Valley): ");
                    if (q4.empty()) {
                        showMsg("Coverage areas cannot be empty.", "err");
                        cout << endl;
                    } else {
                        break;
                    }
                }
                
                string insApp = "INSERT INTO photographer_applications (user_id, portfolio_link, camera_setup, experience_years, coverage_areas) VALUES (?, ?, ?, ?, ?)";
                DBHelper::executeUpdate(conn, insApp, {uid, q1, q2, q3, q4});
            }
            showMsg("Registration Successful! Your application is pending Admin verification.", "ok");
        } else {
            showMsg("Registration Successful!", "ok");
        }
    }
    waitKey();
}

int loginUser(MYSQL* conn) {
    showScreenHeader("LOGIN");
    string email = getInput("Enter Email: ");
    string password = getPasswordInput("Enter Password: ");

    if (email.empty() || password.empty()) { return -1; }

    string q = "SELECT user_id, role, account_status, password, salt, name, email FROM USERS WHERE email = ?";
    auto results = DBHelper::executeQuery(conn, q, {email});
    
    if (!results.empty()) {
        auto& row = results[0];
        int uid = stoi(row[0]);
        string role = row[1], status = row[2], dbPwd = row[3], dbSalt = row[4];
        string name = row[5], dbEmail = row[6];
        
        bool authSuccess = false;
        
        if (dbSalt.empty()) {
            // Legacy user (salt is NULL) -> check with old hashing method
            string legacyHash = Security::hashSHA256(password);
            if (legacyHash == dbPwd) {
                authSuccess = true;
                // Auto-migrate: Generate new salt, hash, and update DB
                string newSalt = Security::generateSalt();
                string newHash = Security::hashWithSalt(newSalt, password);
                DBHelper::executeUpdate(conn, "UPDATE USERS SET password = ?, salt = ? WHERE user_id = ?", {newHash, newSalt, to_string(uid)});
            }
        } else {
            // New/migrated user -> check with salted hashing method
            string saltedHash = Security::hashWithSalt(dbSalt, password);
            if (saltedHash == dbPwd) {
                authSuccess = true;
            }
        }
        
        if (authSuccess) {
            for (char& c : role) c = tolower(c);
            if (role == "photographer") role = "Photographer";
            else if (role == "customer") role = "Customer";
            else if (role == "admin") role = "Admin";
    
            if (role == "Photographer" && status == "Pending") {
                showMsg("Account pending admin approval.", "err");
                waitKey(); return -1;
            }
            
            UserSession session{uid, name, dbEmail, role};
            
            showMsg("Login Successful! Logged in as " + role + " (" + name + ").", "ok");
            Sleep(800);
            if (role == "Admin") adminDashboard(conn, session);
            else if (role == "Photographer") photographerDashboard(conn, session);
            else if (role == "Customer") customerDashboard(conn, session);
            return uid;
        }
    }
    
    showMsg("Invalid email or password!", "err");
    waitKey(); return -1;
}
