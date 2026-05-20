#include "Dashboard/Auth.hpp"
#include "Dashboard/Admin.hpp"
#include "Dashboard/Customer.hpp"
#include "Dashboard/Photographer.hpp"
#include "UIHelper.hpp"
#include "Database.hpp"
#include "Security.hpp"
#include <iostream>
#include <windows.h>

using namespace std;

void registerUser(MYSQL* conn) {
    showScreenHeader("REGISTRATION");
    string name = getInput("Enter Name: ");
    string email = getInput("Enter Email: ");
    string password = getInput("Enter Password: ", true);
    string phone = getInput("Enter Phone Number: ");

    cout << endl;
    vector<string> roles = {"Customer", "Photographer"};
    cout << CLR_BYL << "  Select Role:" << CLR_RS << endl;
    cout << CLR_GY << "    [1] Customer" << endl;
    cout << "    [2] Photographer" << CLR_RS << endl;
    string rc = getInput("Enter choice (1/2): ");
    string role = (rc == "2") ? "Photographer" : "Customer";
    string status = (role == "Photographer") ? "Pending" : "Active";

    if (name.empty() || email.empty() || password.empty()) {
        showMsg("Registration cancelled.", "err");
        waitKey(); return;
    }

    string hashedPwd = Security::hashSHA256(password);
    string q = "INSERT INTO USERS (name, email, password, phone_no, role, account_status) VALUES (?, ?, ?, ?, ?, ?)";
    
    if (DBHelper::executeUpdate(conn, q, {name, email, hashedPwd, phone, role, status}) < 0) {
        showMsg("Registration Failed.", "err");
    } else {
        showMsg("Registration Successful! " + string(role == "Photographer" ? "Please wait for Admin approval." : ""), "ok");
    }
    waitKey();
}

int loginUser(MYSQL* conn) {
    showScreenHeader("LOGIN");
    string email = getInput("Enter Email: ");
    string password = getInput("Enter Password: ", true);

    if (email.empty() || password.empty()) { return -1; }

    string hashedPwd = Security::hashSHA256(password);
    string q = "SELECT user_id, role, account_status FROM USERS WHERE email = ? AND password = ?";
    
    auto results = DBHelper::executeQuery(conn, q, {email, hashedPwd});
    if (!results.empty()) {
        auto& row = results[0];
        int uid = stoi(row[0]);
        string role = row[1], status = row[2];
        for (char& c : role) c = tolower(c);
        if (role == "photographer") role = "Photographer";
        else if (role == "customer") role = "Customer";
        else if (role == "admin") role = "Admin";

        if (role == "Photographer" && status == "Pending") {
            showMsg("Account pending admin approval.", "err");
            waitKey(); return -1;
        }
        showMsg("Login Successful! Logged in as " + role + ".", "ok");
        Sleep(800);
        if (role == "Admin") adminDashboard(conn, uid);
        else if (role == "Photographer") photographerDashboard(conn, uid);
        else if (role == "Customer") customerDashboard(conn, uid);
        return uid;
    } else {
        showMsg("Invalid email or password!", "err");
        waitKey(); return -1;
    }
}
