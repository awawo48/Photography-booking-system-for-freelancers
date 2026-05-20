#include "Dashboard/Photographer.hpp"
#include "UIHelper.hpp"
#include "Database.hpp"
#include <iostream>
#include <vector>
#include <string>

using namespace std;

void photographerDashboard(MYSQL* conn, int userId) {
    vector<string> opts = {"Manage Profile/Portfolio", "Manage Packages", "Manage Schedule", "Booking Management", "Back / Logout"};
    while (true) {
        int c = showMenu("PHOTOGRAPHER DASHBOARD  (ID: " + to_string(userId) + ")", opts);
        if (c == 4 || c == -1) break;

        if (c == 0) {
            showScreenHeader("ADD PORTFOLIO ITEM");
            string title = getInput("Enter Title: ");
            string link = getInput("Enter Media Link: ");
            string date = getInput("Enter Upload Date (YYYY-MM-DD): ");
            if (title.empty()) { showMsg("Cancelled.", "info"); waitKey(); continue; }
            string q = "INSERT INTO PORTFOLIOS (user_id, title, media_link, upload_date) VALUES (?, ?, ?, ?)";
            if (DBHelper::executeUpdate(conn, q, {to_string(userId), title, link, date}) < 0) showMsg("Error updating database.", "err");
            else showMsg("Portfolio added!", "ok");
            waitKey();

        } else if (c == 1) {
            showScreenHeader("ADD PACKAGE");
            string pname = getInput("Enter Package Name: ");
            string cat = getInput("Enter Category: ");
            string price = getInput("Enter Price (RM): ");
            string details = getInput("Enter Details: ");
            if (pname.empty()) { showMsg("Cancelled.", "info"); waitKey(); continue; }
            string q = "INSERT INTO PACKAGES (user_id, package_name, category, price, details) VALUES (?, ?, ?, ?, ?)";
            if (DBHelper::executeUpdate(conn, q, {to_string(userId), pname, cat, price, details}) < 0) showMsg("Error updating database.", "err");
            else showMsg("Package added!", "ok");
            waitKey();

        } else if (c == 2) {
            showScreenHeader("MANAGE SCHEDULE");
            string tdate = getInput("Enter Target Date (YYYY-MM-DD): ");
            string status = getInput("Enter Status (Available/Blocked): ");
            if (tdate.empty()) { showMsg("Cancelled.", "info"); waitKey(); continue; }
            string q = "INSERT INTO SCHEDULES (user_id, target_date, status) VALUES (?, ?, ?)";
            if (DBHelper::executeUpdate(conn, q, {to_string(userId), tdate, status}) < 0) showMsg("Error updating database.", "err");
            else showMsg("Schedule updated!", "ok");
            waitKey();

        } else if (c == 3) {
            showScreenHeader("PENDING BOOKINGS");
            string q = "SELECT b.booking_id, c.name AS customer_name, p.package_name, b.booking_date, b.job_status "
                       "FROM BOOKINGS b JOIN PACKAGES p ON b.package_id = p.package_id "
                       "JOIN USERS c ON b.user_id = c.user_id "
                       "WHERE p.user_id = ? AND b.job_status = 'Pending'";
            auto results = DBHelper::executeQuery(conn, q, {to_string(userId)});
            if (results.empty()) {
                showMsg("No pending bookings.", "info");
                waitKey(); continue;
            }
            for (const auto& row : results) {
                showDivider();
                showField("Booking ID", row[0]);
                showField("Customer", row[1]);
                showField("Package", row[2]);
                showField("Date", row[3]);
                showField("Status", row[4]);
            }
            showDivider();
            cout << endl;
            string bid = getInput("Enter Booking ID (0 to cancel): ");
            if (bid != "0" && !bid.empty()) {
                string act = getInput("Enter Action (Approved/Rejected): ");
                string uq = "UPDATE BOOKINGS SET job_status = ? WHERE booking_id = ?";
                if (DBHelper::executeUpdate(conn, uq, {act, bid}) < 0) showMsg("Error updating database.", "err");
                else showMsg("Booking updated!", "ok");
            }
            waitKey();
        }
    }
}
