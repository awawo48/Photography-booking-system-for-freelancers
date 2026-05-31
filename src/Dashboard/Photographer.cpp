#include "Dashboard/Photographer.hpp"
#include "UIHelper.hpp"
#include "Database.hpp"
#include "CodeGenerator.hpp"
#include <iostream>
#include <vector>
#include <string>

using namespace std;

void photographerDashboard(MYSQL* conn, const UserSession& session) {
    int userId = session.userId;
    vector<string> opts = {"Manage Profile/Portfolio", "Manage Packages", "Manage Schedule", "Booking Management", "Back / Logout"};
    while (true) {
        // ── Account Status Gate ──
        // Re-check every iteration so Admin changes take effect mid-session
        auto statusRes = DBHelper::executeQuery(conn,
            "SELECT account_status FROM USERS WHERE user_id = ?",
            {to_string(userId)});
        string acctStatus = (!statusRes.empty()) ? statusRes[0][0] : "Unknown";

        if (acctStatus != "Active") {
            if (acctStatus == "Pending_Verification" || acctStatus == "Pending") {
                cls();
                showScreenHeader("ACCOUNT PENDING");
                showMsg("Your screening answers are currently under review by the Admin. Please check back later.", "info");
                waitKey();
                break;
            } else {
                cls();
                showScreenHeader("ACCOUNT RESTRICTED");
                showMsg("Your account is currently " + acctStatus + ". Please contact the Admin for assistance.", "err");
                waitKey();
                break;
            }
        }

        // ── Normal Active Photographer Dashboard ──
        string sub = "";
        auto notifRes = DBHelper::executeQuery(conn, "SELECT message FROM NOTIFICATIONS WHERE target_role IN ('All', 'Photographer') ORDER BY created_at DESC LIMIT 1", {});
        if (!notifRes.empty()) sub = "  " + CLR_BYL + "[BROADCAST]: " + CLR_RS + notifRes[0][0];

        int c = showMenu("PHOTOGRAPHER DASHBOARD  (ID: " + to_string(userId) + " | " + session.name + ")", opts, sub);
        if (c == 4 || c == -1) break;

        // ── Manage Profile/Portfolio ──
        if (c == 0) {
            showScreenHeader("ADD PORTFOLIO ITEM");
            string title = getInput("Enter Title: ");
            string link = getInput("Enter Media Link: ");
            string date = getInput("Enter Upload Date (YYYY-MM-DD): ");
            if (title.empty()) { showMsg("Cancelled.", "info"); waitKey(); continue; }
            string pcode = CodeGenerator::generate(conn, "PTF", "PORTFOLIOS", "portfolio_code", false, false);
            string q = "INSERT INTO PORTFOLIOS (user_id, portfolio_code, title, media_link, upload_date) VALUES (?, ?, ?, ?, ?)";
            if (DBHelper::executeUpdate(conn, q, {to_string(userId), pcode, title, link, date}) < 0) showMsg("Error updating database.", "err");
            else showMsg("Portfolio added!", "ok");
            waitKey();

        // ── Manage Packages ──
        } else if (c == 1) {
            showScreenHeader("ADD PACKAGE");
            string pname = getInput("Enter Package Name: ");
            string cat = getInput("Enter Category: ");
            string price = getInput("Enter Price (RM): ");
            string details = getInput("Enter Details: ");
            if (pname.empty()) { showMsg("Cancelled.", "info"); waitKey(); continue; }
            string pcode = CodeGenerator::generate(conn, "PKG", "PACKAGES", "package_code", false, false);
            string q = "INSERT INTO PACKAGES (user_id, package_code, package_name, category, price, details) VALUES (?, ?, ?, ?, ?, ?)";
            if (DBHelper::executeUpdate(conn, q, {to_string(userId), pcode, pname, cat, price, details}) < 0) showMsg("Error updating database.", "err");
            else showMsg("Package added! Code: " + pcode, "ok");
            waitKey();

        // ── Manage Schedule ──
        } else if (c == 2) {
            showScreenHeader("MANAGE SCHEDULE");
            string tdate = getInput("Enter Target Date (YYYY-MM-DD): ");
            string status = getInput("Enter Status (Available/Blocked): ");
            if (tdate.empty()) { showMsg("Cancelled.", "info"); waitKey(); continue; }
            string q = "INSERT INTO SCHEDULES (user_id, target_date, status) VALUES (?, ?, ?)";
            if (DBHelper::executeUpdate(conn, q, {to_string(userId), tdate, status}) < 0) showMsg("Error updating database.", "err");
            else showMsg("Schedule updated!", "ok");
            waitKey();

        // ── Booking Management (Expanded Sub-Menu) ──
        } else if (c == 3) {
            vector<string> bmopts = {"Review Pending Bookings", "Mark Booking as Completed", "View All My Bookings", "Back"};
            while (true) {
                int bmc = showMenu("BOOKING MANAGEMENT", bmopts, "Manage your booking requests and job status");
                if (bmc == 3 || bmc == -1) break;

                // ════════════════════════════════════════════════
                // Option 0: Review Pending Bookings (Approve/Reject)
                // ════════════════════════════════════════════════
                if (bmc == 0) {
                    showScreenHeader("PENDING BOOKINGS");
                    string q = "SELECT b.booking_code, c.name AS customer_name, p.package_name, b.booking_date, b.job_status "
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
                        showField("Booking Code", row[0]);
                        showField("Customer", row[1]);
                        showField("Package", row[2]);
                        showField("Date", row[3]);
                        showField("Status", row[4]);
                    }
                    showDivider();
                    cout << endl;
                    string bcode = getInput("Enter Booking Code (leave empty to cancel): ");
                    if (!bcode.empty()) {
                        string act = getInput("Enter Action (Approved/Rejected): ");

                        if (act == "Approved") {
                            // ── Double-Booking Prevention ──
                            // Fetch the booking's date and verify ownership
                            auto bookingInfo = DBHelper::executeQuery(conn,
                                "SELECT b.booking_date, p.user_id, b.booking_id FROM BOOKINGS b "
                                "JOIN PACKAGES p ON b.package_id = p.package_id "
                                "WHERE b.booking_code = ? AND b.job_status = 'Pending'",
                                {bcode});

                            if (bookingInfo.empty()) {
                                showMsg("Booking not found or no longer pending.", "err");
                                waitKey(); continue;
                            }

                            string bookDate = bookingInfo[0][0];
                            string photogId = bookingInfo[0][1];
                            string bid = bookingInfo[0][2];

                            // Check 1: Is the date blocked in photographer's schedule?
                            auto blockCheck = DBHelper::executeQuery(conn,
                                "SELECT 1 FROM SCHEDULES WHERE user_id = ? AND target_date = ? AND status = 'Blocked'",
                                {photogId, bookDate});
                            if (!blockCheck.empty()) {
                                showMsg("Cannot approve: You have blocked " + bookDate + " in your schedule.", "err");
                                waitKey(); continue;
                            }

                            // Check 2: Is there already another approved/active booking on the same date?
                            auto conflictCheck = DBHelper::executeQuery(conn,
                                "SELECT b.booking_code FROM BOOKINGS b "
                                "JOIN PACKAGES p ON b.package_id = p.package_id "
                                "WHERE p.user_id = ? AND b.booking_date = ? "
                                "AND b.job_status IN ('Approved','Deposit Paid','Completed') "
                                "AND b.booking_id != ?",
                                {photogId, bookDate, bid});
                            if (!conflictCheck.empty()) {
                                showMsg("Cannot approve: Booking " + conflictCheck[0][0]
                                        + " is already confirmed for " + bookDate + ".", "err");
                                waitKey(); continue;
                            }

                            // All checks passed — approve with state transition guard
                            string uq = "UPDATE BOOKINGS SET job_status = 'Approved' WHERE booking_id = ? AND job_status = 'Pending'";
                            int rows = DBHelper::executeUpdate(conn, uq, {bid});
                            if (rows < 0) showMsg("Error updating database.", "err");
                            else if (rows == 0) showMsg("Booking was already processed by another action.", "err");
                            else showMsg("Booking " + bcode + " approved!", "ok");

                        } else if (act == "Rejected") {
                            // Reject with state transition guard
                            string uq = "UPDATE BOOKINGS SET job_status = 'Rejected' WHERE booking_code = ? AND job_status = 'Pending'";
                            int rows = DBHelper::executeUpdate(conn, uq, {bcode});
                            if (rows < 0) showMsg("Error updating database.", "err");
                            else if (rows == 0) showMsg("Booking was already processed.", "err");
                            else showMsg("Booking " + bcode + " rejected.", "ok");

                        } else {
                            showMsg("Invalid action. Please enter 'Approved' or 'Rejected'.", "err");
                        }
                    }
                    waitKey();

                // ════════════════════════════════════════════════
                // Option 1: Mark Booking as Completed
                // ════════════════════════════════════════════════
                } else if (bmc == 1) {
                    showScreenHeader("MARK BOOKING AS COMPLETED");
                    string q = "SELECT b.booking_code, c.name AS customer_name, p.package_name, b.booking_date, b.job_status "
                               "FROM BOOKINGS b JOIN PACKAGES p ON b.package_id = p.package_id "
                               "JOIN USERS c ON b.user_id = c.user_id "
                               "WHERE p.user_id = ? AND b.job_status = 'Deposit Paid'";
                    auto results = DBHelper::executeQuery(conn, q, {to_string(userId)});
                    if (results.empty()) {
                        showMsg("No 'Deposit Paid' bookings to mark as completed.", "info");
                        waitKey(); continue;
                    }
                    for (const auto& row : results) {
                        showDivider();
                        showField("Booking Code", row[0]);
                        showField("Customer", row[1]);
                        showField("Package", row[2]);
                        showField("Date", row[3]);
                        showField("Status", row[4]);
                    }
                    showDivider();
                    cout << endl;
                    string bcode = getInput("Enter Booking Code to mark as Completed (leave empty to cancel): ");
                    if (!bcode.empty()) {
                        string confirm = getInput("Type 'yes' to confirm completion for booking " + bcode + ": ");
                        if (confirm == "yes") {
                            string notes = getInput("Enter Payment Notes (Optional, e.g., 'Remaining balance paid via Cash on site'): ");
                            // State transition guard: only Deposit Paid → Completed
                            string uq = "UPDATE BOOKINGS SET job_status = 'Completed', payment_notes = ? WHERE booking_code = ? AND job_status = 'Deposit Paid'";
                            int rows = DBHelper::executeUpdate(conn, uq, {notes, bcode});
                            if (rows < 0) showMsg("Error updating database.", "err");
                            else if (rows == 0) showMsg("Booking not found or not in 'Deposit Paid' status.", "err");
                            else showMsg("Booking " + bcode + " marked as Completed! Payment notes logged.", "ok");
                        } else {
                            showMsg("Action cancelled.", "info");
                        }
                    }
                    waitKey();

                // ════════════════════════════════════════════════
                // Option 2: View All My Bookings (Read-Only)
                // ════════════════════════════════════════════════
                } else if (bmc == 2) {
                    showScreenHeader("ALL MY BOOKINGS");
                    string q = "SELECT b.booking_code, c.name AS customer_name, p.package_name, "
                               "b.booking_date, b.job_status, p.price "
                               "FROM BOOKINGS b JOIN PACKAGES p ON b.package_id = p.package_id "
                               "JOIN USERS c ON b.user_id = c.user_id "
                               "WHERE p.user_id = ? ORDER BY b.booking_id DESC";
                    auto results = DBHelper::executeQuery(conn, q, {to_string(userId)});
                    if (results.empty()) {
                        showMsg("No bookings found.", "info");
                    } else {
                        for (const auto& row : results) {
                            showDivider();
                            showField("Booking Code", row[0]);
                            showField("Customer", row[1]);
                            showField("Package", row[2]);
                            showField("Date", row[3]);
                            showField("Status", row[4]);
                            showField("Price", string("RM ") + row[5]);
                        }
                        showDivider();
                    }
                    waitKey();
                }
            }
        }
    }
}
