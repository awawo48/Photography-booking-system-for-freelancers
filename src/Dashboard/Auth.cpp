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

void registerUser(MYSQL* conn) {
    showScreenHeader("REGISTRATION");
    string name = getInput("Enter Name: ");
    string email = getInput("Enter Email: ");
    string password = getPasswordInput("Enter Password: ");
    string phone = getInput("Enter Phone Number: ");

    cout << endl;
    vector<string> roles = {"Customer", "Photographer"};
    cout << CLR_BYL << "  Select Role:" << CLR_RS << endl;
    cout << CLR_GY << "    [1] Customer" << endl;
    cout << "    [2] Photographer" << CLR_RS << endl;
    string rc = getInput("Enter choice (1/2): ");
    string role = (rc == "2") ? "Photographer" : "Customer";
    string status = (role == "Photographer") ? "Pending_Verification" : "Active";

    if (name.empty() || email.empty() || password.empty()) {
        showMsg("Registration cancelled.", "err");
        waitKey(); return;
    }

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
                string q1 = getInput("1. Provide a link to your online portfolio (e.g., Instagram, Google Drive, Website): ");
                string q2 = getInput("2. What is your primary camera body and lens setup? (e.g., Sony a6400 + 18-105mm f/4): ");
                string q3 = getInput("3. How many years of professional photography experience do you have?: ");
                string q4 = getInput("4. Which operational areas/states do you cover? (e.g., Kelantan, Melaka, Klang Valley): ");
                
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
