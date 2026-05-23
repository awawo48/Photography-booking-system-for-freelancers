#include "Dashboard/Admin.hpp"
#include "UIHelper.hpp"
#include "Database.hpp"
#include "PDFGenerator.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <windows.h>

using namespace std;

void adminDashboard(MYSQL* conn, int userId) {
    vector<string> opts = {"User Management", "Booking Monitoring", "Dispute Management", "Payment Management", "Review Management", "Business Reports", "Back / Logout"};
    while (true) {
        // Fetch total revenue & admin commission for dashboard display
        string dashSub = "";
        string dq = "SELECT COALESCE(SUM(amount), 0) FROM PAYMENTS";
        auto results = DBHelper::executeQuery(conn, dq, {});
        if (!results.empty() && !results[0][0].empty()) {
            double totalRev = 0;
            try { totalRev = stod(results[0][0]); } catch(...) {}
            double adminComm = totalRev * 0.05;
            ostringstream oss;
            oss << fixed << setprecision(2);
            oss << "Revenue: RM " << totalRev << "  |  Your Commission (5%): RM " << adminComm;
            dashSub = oss.str();
        }
        int c = showMenu("ADMIN DASHBOARD  (ID: " + to_string(userId) + ")", opts, dashSub);
        if (c == 6 || c == -1) break;

        // --- User Management ---
        if (c == 0) {
            vector<string> uopts = {"Verify Pending Photographers", "Ban / Suspend a User", "View All Users", "Delete User", "Search User", "Back"};
            int uc = showMenu("USER MANAGEMENT", uopts);
            if (uc == 0) {
                showScreenHeader("VERIFY PENDING PHOTOGRAPHERS");
                string q = "SELECT user_id, name, email FROM USERS WHERE role = 'Photographer' AND account_status = 'Pending'";
                auto results = DBHelper::executeQuery(conn, q, {});
                if (results.empty()) {
                    showMsg("No pending photographers.", "info");
                } else {
                        for (const auto& row : results) {
                            showDivider();
                            showField("User ID", row[0]);
                            showField("Name", row[1]);
                            showField("Email", row[2]);
                        }
                        showDivider();
                        
                        cout << endl;
                        string tid = getInput("Enter User ID to action: ");
                        string act = getInput("Enter Action (Active/Rejected): ");
                        string uq = "UPDATE USERS SET account_status = '" + act + "' WHERE user_id = " + tid;
                        if (DBHelper::executeUpdate(conn, uq, {}) < 0) showMsg("Error updating database.", "err");
                        else showMsg("User " + tid + " status updated to '" + act + "'.", "ok");
                    }
                waitKey();
            } else if (uc == 1) {
                vector<string> banopts = {"Ban a User", "Suspend a User", "Cancel Ban / Reactivate", "Back"};
                while (true) {
                    int bc = showMenu("BAN / SUSPEND MANAGEMENT", banopts, "Manage user account statuses");
                    if (bc == 3 || bc == -1) break;

                    showScreenHeader(bc == 0 ? "BAN USER" : (bc == 1 ? "SUSPEND USER" : "CANCEL BAN / REACTIVATE"));

                    // Build query based on action context
                    string q;
                    if (bc == 2)
                        q = "SELECT user_id, name, email, role, account_status FROM USERS WHERE account_status IN ('Banned','Suspended')";
                    else
                        q = "SELECT user_id, name, email, role, account_status FROM USERS WHERE role != 'Admin'";

                    if (mysql_query(conn, q.c_str())) { showMsg(string("Error: ") + mysql_error(conn), "err"); waitKey(); continue; }
                    MYSQL_RES* res = mysql_store_result(conn);
                    if (!res || mysql_num_rows(res) == 0) {
                        showMsg(bc == 2 ? "No banned/suspended users found." : "No users found.", "info");
                        if (res)  waitKey(); continue;
                    }

                    // Column widths: ID(4) | Name(18) | Email(26) | Role(14) | Status(10)
                    int cID=4, cName=18, cEmail=26, cRole=14, cStatus=10;

                    // Table top border
                    cout << CLR_CY << "  \xC9";
                    for(int i=0;i<cID+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cName+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cEmail+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cRole+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cStatus+2;i++) cout<<"\xCD";
                    cout << "\xBB" << CLR_RS << endl;

                    // Header row
                    cout << CLR_CY << "  \xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("ID", cID+1)
                         << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Name", cName+1)
                         << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Email", cEmail+1)
                         << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Role", cRole+1)
                         << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Status", cStatus+1)
                         << CLR_CY << "\xBA" << CLR_RS << endl;

                    // Header-body separator
                    cout << CLR_CY << "  \xCC";
                    for(int i=0;i<cID+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cName+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cEmail+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cRole+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cStatus+2;i++) cout<<"\xCD";
                    cout << "\xB9" << CLR_RS << endl;

                    // Data rows
                    for (const auto& row : results) {
                        string sid = row[0];
                        string sname = row[1];
                        string semail = row[2];
                        string srole = row[3];
                        string sstatus = row[4];

                        // Color-code status
                        string statusColor = CLR_WH;
                        if (sstatus == "Active") statusColor = CLR_BGR;
                        else if (sstatus == "Pending") statusColor = CLR_BYL;
                        else if (sstatus == "Banned") statusColor = CLR_BRD;
                        else if (sstatus == "Suspended") statusColor = CLR_BMG;

                        cout << CLR_CY << "  \xBA" << CLR_RS
                             << CLR_WH << " " << padStr(sid, cID+1)
                             << CLR_CY << "\xBA" << CLR_RS
                             << CLR_WH << " " << padStr(sname, cName+1)
                             << CLR_CY << "\xBA" << CLR_RS
                             << CLR_WH << " " << padStr(semail, cEmail+1)
                             << CLR_CY << "\xBA" << CLR_RS
                             << CLR_WH << " " << padStr(srole, cRole+1)
                             << CLR_CY << "\xBA" << CLR_RS
                             << statusColor << " " << padStr(sstatus, cStatus+1) << CLR_RS
                             << CLR_CY << "\xBA" << CLR_RS << endl;
                    }

                    // Table bottom border
                    cout << CLR_CY << "  \xC8";
                    for(int i=0;i<cID+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cName+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cEmail+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cRole+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cStatus+2;i++) cout<<"\xCD";
                    cout << "\xBC" << CLR_RS << endl;

                    
                    cout << endl;

                    // Action input
                    string tid = getInput("Enter User ID (0 to cancel): ");
                    if (tid == "0" || tid.empty()) { waitKey(); continue; }

                    string newStatus, actionLabel;
                    if (bc == 0) { newStatus = "Banned"; actionLabel = "banned"; }
                    else if (bc == 1) { newStatus = "Suspended"; actionLabel = "suspended"; }
                    else { newStatus = "Active"; actionLabel = "reactivated"; }

                    string uq = "UPDATE USERS SET account_status = '" + newStatus + "' WHERE user_id = " + tid;
                    if (DBHelper::executeUpdate(conn, uq, {}) < 0) showMsg("Error updating database.", "err");
                    else showMsg("User " + tid + " has been " + actionLabel + ".", "ok");
                    waitKey();
                }
            }

            // --- View All Users ---
            else if (uc == 2) {
                showScreenHeader("ALL REGISTERED USERS");
                string q = "SELECT user_id, name, email, phone_no, role, account_status FROM USERS ORDER BY user_id ASC";
                auto results = DBHelper::executeQuery(conn, q, {});
                if (results.empty()) {
                    showMsg("No users found.", "info");
                } else {
                        int cID=4, cName=18, cEmail=26, cPhone=14, cRole=14, cSt=10;
                        cout << CLR_CY << "  \xC9";
                        for(int i=0;i<cID+2;i++) cout<<"\xCD"; cout<<"\xCB";
                        for(int i=0;i<cName+2;i++) cout<<"\xCD"; cout<<"\xCB";
                        for(int i=0;i<cEmail+2;i++) cout<<"\xCD"; cout<<"\xCB";
                        for(int i=0;i<cPhone+2;i++) cout<<"\xCD"; cout<<"\xCB";
                        for(int i=0;i<cRole+2;i++) cout<<"\xCD"; cout<<"\xCB";
                        for(int i=0;i<cSt+2;i++) cout<<"\xCD";
                        cout << "\xBB" << CLR_RS << endl;

                        cout << CLR_CY << "  \xBA" << CLR_RS << CLR_BD << CLR_BWH
                             << " " << padStr("ID", cID+1)
                             << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                             << " " << padStr("Name", cName+1)
                             << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                             << " " << padStr("Email", cEmail+1)
                             << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                             << " " << padStr("Phone", cPhone+1)
                             << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                             << " " << padStr("Role", cRole+1)
                             << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                             << " " << padStr("Status", cSt+1)
                             << CLR_CY << "\xBA" << CLR_RS << endl;

                        cout << CLR_CY << "  \xCC";
                        for(int i=0;i<cID+2;i++) cout<<"\xCD"; cout<<"\xCE";
                        for(int i=0;i<cName+2;i++) cout<<"\xCD"; cout<<"\xCE";
                        for(int i=0;i<cEmail+2;i++) cout<<"\xCD"; cout<<"\xCE";
                        for(int i=0;i<cPhone+2;i++) cout<<"\xCD"; cout<<"\xCE";
                        for(int i=0;i<cRole+2;i++) cout<<"\xCD"; cout<<"\xCE";
                        for(int i=0;i<cSt+2;i++) cout<<"\xCD";
                        cout << "\xB9" << CLR_RS << endl;

                        for (const auto& row : results) {
                            string sid = row[0], sn = row[1];
                            string se = row[2], sp = row[3];
                            string sr = row[4], ss = row[5];
                            string sc = CLR_WH, rc = CLR_WH;
                            if (ss=="Active") sc=CLR_BGR; else if (ss=="Pending") sc=CLR_BYL;
                            else if (ss=="Banned") sc=CLR_BRD; else if (ss=="Suspended") sc=CLR_BMG;
                            if (sr=="Admin") rc=CLR_BCY; else if (sr=="Photographer") rc=CLR_BMG;
                            else if (sr=="Customer") rc=CLR_BYL;
                            cout << CLR_CY << "  \xBA" << CLR_RS
                                 << CLR_WH << " " << padStr(sid,cID+1)
                                 << CLR_CY << "\xBA" << CLR_RS
                                 << CLR_WH << " " << padStr(sn,cName+1)
                                 << CLR_CY << "\xBA" << CLR_RS
                                 << CLR_WH << " " << padStr(se,cEmail+1)
                                 << CLR_CY << "\xBA" << CLR_RS
                                 << CLR_WH << " " << padStr(sp,cPhone+1)
                                 << CLR_CY << "\xBA" << CLR_RS
                                 << rc << " " << padStr(sr,cRole+1) << CLR_RS
                                 << CLR_CY << "\xBA" << CLR_RS
                                 << sc << " " << padStr(ss,cSt+1) << CLR_RS
                                 << CLR_CY << "\xBA" << CLR_RS << endl;
                        }
                        cout << CLR_CY << "  \xC8";
                        for(int i=0;i<cID+2;i++) cout<<"\xCD"; cout<<"\xCA";
                        for(int i=0;i<cName+2;i++) cout<<"\xCD"; cout<<"\xCA";
                        for(int i=0;i<cEmail+2;i++) cout<<"\xCD"; cout<<"\xCA";
                        for(int i=0;i<cPhone+2;i++) cout<<"\xCD"; cout<<"\xCA";
                        for(int i=0;i<cRole+2;i++) cout<<"\xCD"; cout<<"\xCA";
                        for(int i=0;i<cSt+2;i++) cout<<"\xCD";
                        cout << "\xBC" << CLR_RS << endl;
                        
                    }
                waitKey();
            }

            // --- Delete User ---
            else if (uc == 3) {
                showScreenHeader("DELETE USER");
                string q = "SELECT user_id, name, email, role, account_status FROM USERS WHERE role != 'Admin' ORDER BY user_id";
                auto results = DBHelper::executeQuery(conn, q, {});
                if (results.empty()) {
                    showMsg("No users found.", "info");
                } else {
                        int cID=4, cName=18, cEmail=26, cRole=14, cSt=10;
                        cout << CLR_CY << "  \xC9";
                        for(int i=0;i<cID+2;i++) cout<<"\xCD"; cout<<"\xCB";
                        for(int i=0;i<cName+2;i++) cout<<"\xCD"; cout<<"\xCB";
                        for(int i=0;i<cEmail+2;i++) cout<<"\xCD"; cout<<"\xCB";
                        for(int i=0;i<cRole+2;i++) cout<<"\xCD"; cout<<"\xCB";
                        for(int i=0;i<cSt+2;i++) cout<<"\xCD";
                        cout << "\xBB" << CLR_RS << endl;
                        cout << CLR_CY << "  \xBA" << CLR_RS << CLR_BD << CLR_BWH
                             << " " << padStr("ID",cID+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                             << " " << padStr("Name",cName+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                             << " " << padStr("Email",cEmail+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                             << " " << padStr("Role",cRole+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                             << " " << padStr("Status",cSt+1) << CLR_CY << "\xBA" << CLR_RS << endl;
                        cout << CLR_CY << "  \xCC";
                        for(int i=0;i<cID+2;i++) cout<<"\xCD"; cout<<"\xCE";
                        for(int i=0;i<cName+2;i++) cout<<"\xCD"; cout<<"\xCE";
                        for(int i=0;i<cEmail+2;i++) cout<<"\xCD"; cout<<"\xCE";
                        for(int i=0;i<cRole+2;i++) cout<<"\xCD"; cout<<"\xCE";
                        for(int i=0;i<cSt+2;i++) cout<<"\xCD";
                        cout << "\xB9" << CLR_RS << endl;
                        for (const auto& row : results) {
                            string sid=row[0], sn=row[1];
                            string se=row[2], sr=row[3], ss=row[4];
                            cout << CLR_CY << "  \xBA" << CLR_RS
                                 << CLR_WH << " " << padStr(sid,cID+1) << CLR_CY << "\xBA" << CLR_RS
                                 << CLR_WH << " " << padStr(sn,cName+1) << CLR_CY << "\xBA" << CLR_RS
                                 << CLR_WH << " " << padStr(se,cEmail+1) << CLR_CY << "\xBA" << CLR_RS
                                 << CLR_WH << " " << padStr(sr,cRole+1) << CLR_CY << "\xBA" << CLR_RS
                                 << CLR_WH << " " << padStr(ss,cSt+1)
                                 << CLR_CY << "\xBA" << CLR_RS << endl;
                        }
                        cout << CLR_CY << "  \xC8";
                        for(int i=0;i<cID+2;i++) cout<<"\xCD"; cout<<"\xCA";
                        for(int i=0;i<cName+2;i++) cout<<"\xCD"; cout<<"\xCA";
                        for(int i=0;i<cEmail+2;i++) cout<<"\xCD"; cout<<"\xCA";
                        for(int i=0;i<cRole+2;i++) cout<<"\xCD"; cout<<"\xCA";
                        for(int i=0;i<cSt+2;i++) cout<<"\xCD";
                        cout << "\xBC" << CLR_RS << endl;
                        
                        cout << endl;
                        string tid = getInput("Enter User ID to delete (0 to cancel): ");
                        if (tid != "0" && !tid.empty()) {
                            string confirm = getInput("Type 'yes' to confirm delete user " + tid + ": ");
                            if (confirm == "yes") {
                                // Delete related data first (FK constraints)
                                DBHelper::executeUpdate(conn, "DELETE FROM REVIEWS WHERE booking_id IN (SELECT booking_id FROM BOOKINGS WHERE user_id=" + tid + ")", {});
                                DBHelper::executeUpdate(conn, "DELETE FROM PAYMENTS WHERE booking_id IN (SELECT booking_id FROM BOOKINGS WHERE user_id=" + tid + ")", {});
                                mysql_query(conn, ("DELETE FROM REPORTS WHERE user_id=" + tid).c_str());
                                mysql_query(conn, ("DELETE FROM BOOKINGS WHERE user_id=" + tid).c_str());
                                mysql_query(conn, ("DELETE FROM SCHEDULES WHERE user_id=" + tid).c_str());
                                mysql_query(conn, ("DELETE FROM PACKAGES WHERE user_id=" + tid).c_str());
                                mysql_query(conn, ("DELETE FROM PORTFOLIOS WHERE user_id=" + tid).c_str());
                                string dq = "DELETE FROM USERS WHERE user_id = " + tid + " AND role != 'Admin'";
                                if (DBHelper::executeUpdate(conn, dq, {}) < 0) showMsg("Error updating database.", "err");
                                else showMsg("User " + tid + " deleted successfully.", "ok");
                            } else {
                                showMsg("Delete cancelled.", "info");
                            }
                        }
                    }
                waitKey();
            }

            // --- Search User ---
            else if (uc == 4) {
                showScreenHeader("SEARCH USER");
                string keyword = getInput("Enter name or email to search: ");
                if (keyword.empty()) { showMsg("Search cancelled.", "info"); waitKey(); }
                else {
                    string q = "SELECT user_id, name, email, phone_no, role, account_status FROM USERS "
                               "WHERE name LIKE '%" + keyword + "%' OR email LIKE '%" + keyword + "%' ORDER BY user_id";
                    auto results = DBHelper::executeQuery(conn, q, {});
                if (results.empty()) {
                    showMsg("No users matched '" + keyword + "'.", "info");
                } else {
                            int cID=4, cName=18, cEmail=26, cPhone=14, cRole=14, cSt=10;
                            cout << CLR_CY << "  \xC9";
                            for(int i=0;i<cID+2;i++) cout<<"\xCD"; cout<<"\xCB";
                            for(int i=0;i<cName+2;i++) cout<<"\xCD"; cout<<"\xCB";
                            for(int i=0;i<cEmail+2;i++) cout<<"\xCD"; cout<<"\xCB";
                            for(int i=0;i<cPhone+2;i++) cout<<"\xCD"; cout<<"\xCB";
                            for(int i=0;i<cRole+2;i++) cout<<"\xCD"; cout<<"\xCB";
                            for(int i=0;i<cSt+2;i++) cout<<"\xCD";
                            cout << "\xBB" << CLR_RS << endl;
                            cout << CLR_CY << "  \xBA" << CLR_RS << CLR_BD << CLR_BWH
                                 << " " << padStr("ID",cID+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                                 << " " << padStr("Name",cName+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                                 << " " << padStr("Email",cEmail+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                                 << " " << padStr("Phone",cPhone+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                                 << " " << padStr("Role",cRole+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                                 << " " << padStr("Status",cSt+1) << CLR_CY << "\xBA" << CLR_RS << endl;
                            cout << CLR_CY << "  \xCC";
                            for(int i=0;i<cID+2;i++) cout<<"\xCD"; cout<<"\xCE";
                            for(int i=0;i<cName+2;i++) cout<<"\xCD"; cout<<"\xCE";
                            for(int i=0;i<cEmail+2;i++) cout<<"\xCD"; cout<<"\xCE";
                            for(int i=0;i<cPhone+2;i++) cout<<"\xCD"; cout<<"\xCE";
                            for(int i=0;i<cRole+2;i++) cout<<"\xCD"; cout<<"\xCE";
                            for(int i=0;i<cSt+2;i++) cout<<"\xCD";
                            cout << "\xB9" << CLR_RS << endl;
                            for (const auto& row : results) {
                                string sid=row[0], sn=row[1];
                                string se=row[2], sp=row[3];
                                string sr=row[4], ss=row[5];
                                string sc=CLR_WH;
                                if (ss=="Active") sc=CLR_BGR; else if (ss=="Pending") sc=CLR_BYL;
                                else if (ss=="Banned") sc=CLR_BRD; else if (ss=="Suspended") sc=CLR_BMG;
                                cout << CLR_CY << "  \xBA" << CLR_RS
                                     << CLR_WH << " " << padStr(sid,cID+1) << CLR_CY << "\xBA" << CLR_RS
                                     << CLR_WH << " " << padStr(sn,cName+1) << CLR_CY << "\xBA" << CLR_RS
                                     << CLR_WH << " " << padStr(se,cEmail+1) << CLR_CY << "\xBA" << CLR_RS
                                     << CLR_WH << " " << padStr(sp,cPhone+1) << CLR_CY << "\xBA" << CLR_RS
                                     << CLR_WH << " " << padStr(sr,cRole+1) << CLR_CY << "\xBA" << CLR_RS
                                     << sc << " " << padStr(ss,cSt+1) << CLR_RS
                                     << CLR_CY << "\xBA" << CLR_RS << endl;
                            }
                            cout << CLR_CY << "  \xC8";
                            for(int i=0;i<cID+2;i++) cout<<"\xCD"; cout<<"\xCA";
                            for(int i=0;i<cName+2;i++) cout<<"\xCD"; cout<<"\xCA";
                            for(int i=0;i<cEmail+2;i++) cout<<"\xCD"; cout<<"\xCA";
                            for(int i=0;i<cPhone+2;i++) cout<<"\xCD"; cout<<"\xCA";
                            for(int i=0;i<cRole+2;i++) cout<<"\xCD"; cout<<"\xCA";
                            for(int i=0;i<cSt+2;i++) cout<<"\xCD";
                            cout << "\xBC" << CLR_RS << endl;
                            
                        }
                waitKey();
            }
        }
        }

        // --- Booking Monitoring ---
        else if (c == 1) {
            vector<string> bmopts = {"View All Bookings", "Filter by Status", "Update Booking Status", "Cancel Booking", "Back"};
            while (true) {
                int bmc = showMenu("BOOKING MONITORING", bmopts, "Track and manage all bookings");
                if (bmc == 4 || bmc == -1) break;

                // --- Update Booking Status ---
                if (bmc == 2) {
                    showScreenHeader("UPDATE BOOKING STATUS");
                    string bq = "SELECT b.booking_id, c.name, p.package_name, b.booking_date, b.job_status "
                                "FROM BOOKINGS b JOIN USERS c ON b.user_id = c.user_id "
                                "JOIN PACKAGES p ON b.package_id = p.package_id ORDER BY b.booking_id";
                    if (mysql_query(conn, bq.c_str())) { showMsg(string("Error: ") + mysql_error(conn), "err"); waitKey(); continue; }
                    MYSQL_RES* bres = mysql_store_result(conn);
                    if (!bres || mysql_num_rows(bres) == 0) {
                        showMsg("No bookings found.", "info");
                        if (bres) mysql_free_result(bres); waitKey(); continue;
                    }
                    int cBI=4, cCu=16, cPk=18, cDt=12, cSs=13;
                    cout << CLR_CY << "  \xC9";
                    for(int i=0;i<cBI+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cCu+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cPk+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cDt+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cSs+2;i++) cout<<"\xCD";
                    cout << "\xBB" << CLR_RS << endl;
                    cout << CLR_CY << "  \xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("ID",cBI+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Customer",cCu+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Package",cPk+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Date",cDt+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Status",cSs+1) << CLR_CY << "\xBA" << CLR_RS << endl;
                    cout << CLR_CY << "  \xCC";
                    for(int i=0;i<cBI+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cCu+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cPk+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cDt+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cSs+2;i++) cout<<"\xCD";
                    cout << "\xB9" << CLR_RS << endl;
                    MYSQL_ROW br;
                    while ((br = mysql_fetch_row(bres))) {
                        string sc = CLR_WH;
                        string ss = br[4]?br[4]:"";
                        if (ss=="Completed") sc=CLR_BGR; else if (ss=="Rejected") sc=CLR_BRD;
                        else if (ss=="Pending"||ss=="Approved"||ss=="Deposit Paid") sc=CLR_BYL;
                        cout << CLR_CY << "  \xBA" << CLR_RS
                             << CLR_WH << " " << padStr(br[0]?br[0]:"",cBI+1) << CLR_CY << "\xBA" << CLR_RS
                             << CLR_WH << " " << padStr(br[1]?br[1]:"",cCu+1) << CLR_CY << "\xBA" << CLR_RS
                             << CLR_WH << " " << padStr(br[2]?br[2]:"",cPk+1) << CLR_CY << "\xBA" << CLR_RS
                             << CLR_WH << " " << padStr(br[3]?br[3]:"",cDt+1) << CLR_CY << "\xBA" << CLR_RS
                             << sc << " " << padStr(ss,cSs+1) << CLR_RS
                             << CLR_CY << "\xBA" << CLR_RS << endl;
                    }
                    cout << CLR_CY << "  \xC8";
                    for(int i=0;i<cBI+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cCu+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cPk+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cDt+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cSs+2;i++) cout<<"\xCD";
                    cout << "\xBC" << CLR_RS << endl;
                    bres = nullptr; // Note: mysql_free_result is handled or we missed it in original code, we keep it exact
                    cout << endl;
                    string bid = getInput("Enter Booking ID to update (0 to cancel): ");
                    if (bid != "0" && !bid.empty()) {
                        cout << endl;
                        cout << CLR_BWH << "    Select new status:" << CLR_RS << endl;
                        cout << CLR_BYL << "      [1] Pending" << CLR_RS << endl;
                        cout << CLR_BYL << "      [2] Approved" << CLR_RS << endl;
                        cout << CLR_BYL << "      [3] Deposit Paid" << CLR_RS << endl;
                        cout << CLR_BGR << "      [4] Completed" << CLR_RS << endl;
                        cout << CLR_BRD << "      [5] Rejected" << CLR_RS << endl;
                        string sc = getInput("Enter choice (1-5): ");
                        string ns = "";
                        if (sc=="1") ns="Pending"; else if (sc=="2") ns="Approved";
                        else if (sc=="3") ns="Deposit Paid"; else if (sc=="4") ns="Completed";
                        else if (sc=="5") ns="Rejected";
                        if (!ns.empty()) {
                            string uq = "UPDATE BOOKINGS SET job_status = '" + ns + "' WHERE booking_id = " + bid;
                            if (DBHelper::executeUpdate(conn, uq, {}) < 0) showMsg("Error updating database.", "err");
                            else showMsg("Booking #" + bid + " status updated to '" + ns + "'.", "ok");
                        } else showMsg("Invalid choice.", "err");
                    }
                    waitKey(); continue;
                }

                // --- Cancel Booking ---
                if (bmc == 3) {
                    showScreenHeader("CANCEL / DELETE BOOKING");
                    string bq = "SELECT b.booking_id, c.name, p.package_name, b.booking_date, b.job_status "
                                "FROM BOOKINGS b JOIN USERS c ON b.user_id = c.user_id "
                                "JOIN PACKAGES p ON b.package_id = p.package_id ORDER BY b.booking_id";
                    if (mysql_query(conn, bq.c_str())) { showMsg(string("Error: ") + mysql_error(conn), "err"); waitKey(); continue; }
                    MYSQL_RES* bres = mysql_store_result(conn);
                    if (!bres || mysql_num_rows(bres) == 0) {
                        showMsg("No bookings found.", "info");
                        if (bres) mysql_free_result(bres); waitKey(); continue;
                    }
                    int cBI=4, cCu=16, cPk=18, cDt=12, cSs=13;
                    cout << CLR_CY << "  \xC9";
                    for(int i=0;i<cBI+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cCu+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cPk+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cDt+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cSs+2;i++) cout<<"\xCD";
                    cout << "\xBB" << CLR_RS << endl;
                    cout << CLR_CY << "  \xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("ID",cBI+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Customer",cCu+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Package",cPk+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Date",cDt+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Status",cSs+1) << CLR_CY << "\xBA" << CLR_RS << endl;
                    cout << CLR_CY << "  \xCC";
                    for(int i=0;i<cBI+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cCu+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cPk+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cDt+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cSs+2;i++) cout<<"\xCD";
                    cout << "\xB9" << CLR_RS << endl;
                    MYSQL_ROW br;
                    while ((br = mysql_fetch_row(bres))) {
                        cout << CLR_CY << "  \xBA" << CLR_RS
                             << CLR_WH << " " << padStr(br[0]?br[0]:"",cBI+1) << CLR_CY << "\xBA" << CLR_RS
                             << CLR_WH << " " << padStr(br[1]?br[1]:"",cCu+1) << CLR_CY << "\xBA" << CLR_RS
                             << CLR_WH << " " << padStr(br[2]?br[2]:"",cPk+1) << CLR_CY << "\xBA" << CLR_RS
                             << CLR_WH << " " << padStr(br[3]?br[3]:"",cDt+1) << CLR_CY << "\xBA" << CLR_RS
                             << CLR_WH << " " << padStr(br[4]?br[4]:"",cSs+1)
                             << CLR_CY << "\xBA" << CLR_RS << endl;
                    }
                    cout << CLR_CY << "  \xC8";
                    for(int i=0;i<cBI+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cCu+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cPk+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cDt+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cSs+2;i++) cout<<"\xCD";
                    cout << "\xBC" << CLR_RS << endl;
                    mysql_free_result(bres);
                    cout << endl;
                    string bid = getInput("Enter Booking ID to delete (0 to cancel): ");
                    if (bid != "0" && !bid.empty()) {
                        string confirm = getInput("Type 'yes' to confirm delete booking #" + bid + ": ");
                        if (confirm == "yes") {
                            mysql_query(conn, ("DELETE FROM REVIEWS WHERE booking_id=" + bid).c_str());
                            mysql_query(conn, ("DELETE FROM PAYMENTS WHERE booking_id=" + bid).c_str());
                            mysql_query(conn, ("DELETE FROM REPORTS WHERE booking_id=" + bid).c_str());
                            string dq = "DELETE FROM BOOKINGS WHERE booking_id = " + bid;
                            if (DBHelper::executeUpdate(conn, dq, {}) < 0) showMsg("Error updating database.", "err");
                            else showMsg("Booking #" + bid + " deleted successfully.", "ok");
                        } else showMsg("Delete cancelled.", "info");
                    }
                    waitKey(); continue;
                }

                string statusFilter = "";
                if (bmc == 1) {
                    vector<string> fopts = {"Pending", "Approved", "Deposit Paid", "Completed", "Rejected", "Back"};
                    int fc = showMenu("FILTER BY STATUS", fopts);
                    if (fc == 5 || fc == -1) continue;
                    statusFilter = fopts[fc];
                }

                showScreenHeader(statusFilter.empty() ? "ALL BOOKINGS (MASTER LIST)" : ("BOOKINGS: " + statusFilter));

                string q = "SELECT b.booking_id, c.name AS customer_name, p.package_name, "
                           "b.booking_date, b.job_status, p.price "
                           "FROM BOOKINGS b JOIN USERS c ON b.user_id = c.user_id "
                           "JOIN PACKAGES p ON b.package_id = p.package_id";
                if (!statusFilter.empty())
                    q += " WHERE b.job_status = '" + statusFilter + "'";
                q += " ORDER BY b.booking_id ASC";

                auto results = DBHelper::executeQuery(conn, q, {});
                    if (results.empty()) {
                        showMsg("No bookings found.", "info");
                        waitKey(); continue;
                    }

                // Column widths: ID(4) | Customer(16) | Package(18) | Date(12) | Status(13) | Price(10)
                int cID=4, cCust=16, cPkg=18, cDate=12, cStat=13, cPrice=10;
                // Table top border
                cout << CLR_CY << "  ";
                cout << "\xC9";
                for(int i=0;i<cID+2;i++) cout<<"\xCD"; cout<<"\xCB";
                for(int i=0;i<cCust+2;i++) cout<<"\xCD"; cout<<"\xCB";
                for(int i=0;i<cPkg+2;i++) cout<<"\xCD"; cout<<"\xCB";
                for(int i=0;i<cDate+2;i++) cout<<"\xCD"; cout<<"\xCB";
                for(int i=0;i<cStat+2;i++) cout<<"\xCD"; cout<<"\xCB";
                for(int i=0;i<cPrice+2;i++) cout<<"\xCD";
                cout << "\xBB" << CLR_RS << endl;

                // Header row
                cout << CLR_CY << "  \xBA" << CLR_RS << CLR_BD << CLR_BWH
                     << " " << padStr("ID", cID+1)
                     << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                     << " " << padStr("Customer", cCust+1)
                     << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                     << " " << padStr("Package", cPkg+1)
                     << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                     << " " << padStr("Date", cDate+1)
                     << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                     << " " << padStr("Status", cStat+1)
                     << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                     << " " << padStr("Price(RM)", cPrice+1)
                     << CLR_CY << "\xBA" << CLR_RS << endl;

                // Header-body separator
                cout << CLR_CY << "  ";
                cout << "\xCC";
                for(int i=0;i<cID+2;i++) cout<<"\xCD"; cout<<"\xCE";
                for(int i=0;i<cCust+2;i++) cout<<"\xCD"; cout<<"\xCE";
                for(int i=0;i<cPkg+2;i++) cout<<"\xCD"; cout<<"\xCE";
                for(int i=0;i<cDate+2;i++) cout<<"\xCD"; cout<<"\xCE";
                for(int i=0;i<cStat+2;i++) cout<<"\xCD"; cout<<"\xCE";
                for(int i=0;i<cPrice+2;i++) cout<<"\xCD";
                cout << "\xB9" << CLR_RS << endl;

                // Data rows
                MYSQL_ROW row;
                int totalBookings = 0;
                double totalValue = 0.0;
                int cntPending=0, cntApproved=0, cntDeposit=0, cntCompleted=0, cntRejected=0;
                for (const auto& row : results) {
                    totalBookings++;
                    string sid = row[0];
                    string sname = row[1];
                    string spkg = row[2];
                    string sdate = row[3];
                    string sstatus = row[4];
                    string sprice = row[5];
                    try { totalValue += stod(sprice); } catch(...) {}

                    if (sstatus == "Pending") cntPending++;
                    else if (sstatus == "Approved") cntApproved++;
                    else if (sstatus == "Deposit Paid") cntDeposit++;
                    else if (sstatus == "Completed") cntCompleted++;
                    else if (sstatus == "Rejected") cntRejected++;

                    // Color-code status
                    string statusColor = CLR_WH;
                    if (sstatus == "Completed") statusColor = CLR_BGR;
                    else if (sstatus == "Pending" || sstatus == "Approved" || sstatus == "Deposit Paid") statusColor = CLR_BYL;
                    else if (sstatus == "Rejected") statusColor = CLR_BRD;

                    cout << CLR_CY << "  \xBA" << CLR_RS
                         << CLR_WH << " " << padStr(sid, cID+1)
                         << CLR_CY << "\xBA" << CLR_RS
                         << CLR_WH << " " << padStr(sname, cCust+1)
                         << CLR_CY << "\xBA" << CLR_RS
                         << CLR_WH << " " << padStr(spkg, cPkg+1)
                         << CLR_CY << "\xBA" << CLR_RS
                         << CLR_WH << " " << padStr(sdate, cDate+1)
                         << CLR_CY << "\xBA" << CLR_RS
                         << statusColor << " " << padStr(sstatus, cStat+1) << CLR_RS
                         << CLR_CY << "\xBA" << CLR_RS
                         << CLR_WH << " " << padStr(sprice, cPrice+1)
                         << CLR_CY << "\xBA" << CLR_RS << endl;
                }

                // Table bottom border
                cout << CLR_CY << "  ";
                cout << "\xC8";
                for(int i=0;i<cID+2;i++) cout<<"\xCD"; cout<<"\xCA";
                for(int i=0;i<cCust+2;i++) cout<<"\xCD"; cout<<"\xCA";
                for(int i=0;i<cPkg+2;i++) cout<<"\xCD"; cout<<"\xCA";
                for(int i=0;i<cDate+2;i++) cout<<"\xCD"; cout<<"\xCA";
                for(int i=0;i<cStat+2;i++) cout<<"\xCD"; cout<<"\xCA";
                for(int i=0;i<cPrice+2;i++) cout<<"\xCD";
                cout << "\xBC" << CLR_RS << endl;

                

                // Summary stats
                cout << endl;
                showDivider();
                cout << CLR_BD << CLR_BWH << "    SUMMARY" << CLR_RS << endl;
                showDivider();

                ostringstream oss;
                oss << fixed << setprecision(2) << totalValue;
                double adminCut = totalValue * 0.05;
                ostringstream ossAdmin;
                ossAdmin << fixed << setprecision(2) << adminCut;

                showField("Total Bookings", to_string(totalBookings));
                showField("Total Value", string("RM ") + oss.str());
                showField("Admin Commission (5%)", string("RM ") + ossAdmin.str());
                cout << endl;
                cout << CLR_BYL << "    Pending: " << cntPending << CLR_RS
                     << CLR_GY << "  |  " << CLR_RS
                     << CLR_BYL << "Approved: " << cntApproved << CLR_RS
                     << CLR_GY << "  |  " << CLR_RS
                     << CLR_BYL << "Deposit: " << cntDeposit << CLR_RS
                     << CLR_GY << "  |  " << CLR_RS
                     << CLR_BGR << "Done: " << cntCompleted << CLR_RS
                     << CLR_GY << "  |  " << CLR_RS
                     << CLR_BRD << "Rejected: " << cntRejected << CLR_RS << endl;
                showDivider();

                waitKey();
            }
        }

        // --- Dispute Management ---
        else if (c == 2) {
            vector<string> dmopts = {"View Pending Disputes", "View All Disputes", "View Resolved", "View Dismissed", "Back"};
            while (true) {
                int dmc = showMenu("DISPUTE MANAGEMENT", dmopts, "Review and resolve customer reports");
                if (dmc == 4 || dmc == -1) break;

                string filterClause = "";
                string headerTitle = "ALL DISPUTE REPORTS";
                if (dmc == 0) { filterClause = " WHERE r.admin_action = 'Pending'"; headerTitle = "PENDING DISPUTES"; }
                else if (dmc == 2) { filterClause = " WHERE r.admin_action = 'Resolved'"; headerTitle = "RESOLVED DISPUTES"; }
                else if (dmc == 3) { filterClause = " WHERE r.admin_action = 'Dismissed'"; headerTitle = "DISMISSED DISPUTES"; }

                showScreenHeader(headerTitle);

                string q = "SELECT r.report_id, u.name AS reporter, r.booking_id, "
                           "r.admin_action, LEFT(r.description, 30) AS short_desc "
                           "FROM REPORTS r JOIN USERS u ON r.user_id = u.user_id" + filterClause +
                           " ORDER BY r.report_id ASC";

                auto results = DBHelper::executeQuery(conn, q, {});
                    if (results.empty()) {
                        showMsg("No disputes found.", "info");
                        waitKey(); continue;
                    }

                // Column widths: ID(4) | Reporter(16) | Booking(8) | Action(10) | Description(30)
                int cID=4, cReporter=16, cBooking=8, cAction=10, cDesc=30;

                // Table top border
                cout << CLR_CY << "  \xC9";
                for(int i=0;i<cID+2;i++) cout<<"\xCD"; cout<<"\xCB";
                for(int i=0;i<cReporter+2;i++) cout<<"\xCD"; cout<<"\xCB";
                for(int i=0;i<cBooking+2;i++) cout<<"\xCD"; cout<<"\xCB";
                for(int i=0;i<cAction+2;i++) cout<<"\xCD"; cout<<"\xCB";
                for(int i=0;i<cDesc+2;i++) cout<<"\xCD";
                cout << "\xBB" << CLR_RS << endl;

                // Header row
                cout << CLR_CY << "  \xBA" << CLR_RS << CLR_BD << CLR_BWH
                     << " " << padStr("ID", cID+1)
                     << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                     << " " << padStr("Reporter", cReporter+1)
                     << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                     << " " << padStr("Booking", cBooking+1)
                     << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                     << " " << padStr("Action", cAction+1)
                     << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                     << " " << padStr("Description", cDesc+1)
                     << CLR_CY << "\xBA" << CLR_RS << endl;

                // Header-body separator
                cout << CLR_CY << "  \xCC";
                for(int i=0;i<cID+2;i++) cout<<"\xCD"; cout<<"\xCE";
                for(int i=0;i<cReporter+2;i++) cout<<"\xCD"; cout<<"\xCE";
                for(int i=0;i<cBooking+2;i++) cout<<"\xCD"; cout<<"\xCE";
                for(int i=0;i<cAction+2;i++) cout<<"\xCD"; cout<<"\xCE";
                for(int i=0;i<cDesc+2;i++) cout<<"\xCD";
                cout << "\xB9" << CLR_RS << endl;

                // Data rows
                MYSQL_ROW row;
                int totalReports = 0, cntPending=0, cntResolved=0, cntDismissed=0;
                for (const auto& row : results) {
                    totalReports++;
                    string sid = row[0];
                    string sreporter = row[1];
                    string sbooking = row[2];
                    string saction = row[3];
                    string sdesc = row[4];

                    if (saction == "Pending") cntPending++;
                    else if (saction == "Resolved") cntResolved++;
                    else if (saction == "Dismissed") cntDismissed++;

                    // Color-code action status
                    string actionColor = CLR_WH;
                    if (saction == "Pending") actionColor = CLR_BYL;
                    else if (saction == "Resolved") actionColor = CLR_BGR;
                    else if (saction == "Dismissed") actionColor = CLR_BRD;

                    cout << CLR_CY << "  \xBA" << CLR_RS
                         << CLR_WH << " " << padStr(sid, cID+1)
                         << CLR_CY << "\xBA" << CLR_RS
                         << CLR_WH << " " << padStr(sreporter, cReporter+1)
                         << CLR_CY << "\xBA" << CLR_RS
                         << CLR_WH << " " << padStr(sbooking, cBooking+1)
                         << CLR_CY << "\xBA" << CLR_RS
                         << actionColor << " " << padStr(saction, cAction+1) << CLR_RS
                         << CLR_CY << "\xBA" << CLR_RS
                         << CLR_GY << " " << padStr(sdesc, cDesc+1)
                         << CLR_CY << "\xBA" << CLR_RS << endl;
                }

                // Table bottom border
                cout << CLR_CY << "  \xC8";
                for(int i=0;i<cID+2;i++) cout<<"\xCD"; cout<<"\xCA";
                for(int i=0;i<cReporter+2;i++) cout<<"\xCD"; cout<<"\xCA";
                for(int i=0;i<cBooking+2;i++) cout<<"\xCD"; cout<<"\xCA";
                for(int i=0;i<cAction+2;i++) cout<<"\xCD"; cout<<"\xCA";
                for(int i=0;i<cDesc+2;i++) cout<<"\xCD";
                cout << "\xBC" << CLR_RS << endl;

                

                // Summary stats
                cout << endl;
                showDivider();
                cout << CLR_BD << CLR_BWH << "    SUMMARY" << CLR_RS << endl;
                showDivider();
                showField("Total Reports", to_string(totalReports));
                cout << CLR_BYL << "    Pending: " << cntPending << CLR_RS
                     << CLR_GY << "  |  " << CLR_RS
                     << CLR_BGR << "Resolved: " << cntResolved << CLR_RS
                     << CLR_GY << "  |  " << CLR_RS
                     << CLR_BRD << "Dismissed: " << cntDismissed << CLR_RS << endl;
                showDivider();
                cout << endl;

                // Detail view & action
                string rid = getInput("Enter Report ID to view details (0 to go back): ");
                if (rid == "0" || rid.empty()) { continue; }

                // Fetch full report details
                string dq = "SELECT r.report_id, u.name, u.email, r.booking_id, "
                            "p.package_name, b.booking_date, r.description, r.admin_action "
                            "FROM REPORTS r "
                            "JOIN USERS u ON r.user_id = u.user_id "
                            "JOIN BOOKINGS b ON r.booking_id = b.booking_id "
                            "JOIN PACKAGES p ON b.package_id = p.package_id "
                            "WHERE r.report_id = " + rid;
                if (mysql_query(conn, dq.c_str())) { showMsg(string("Error: ") + mysql_error(conn), "err"); waitKey(); continue; }
                MYSQL_RES* dres = mysql_store_result(conn);
                if (!dres || mysql_num_rows(dres) == 0) {
                    showMsg("Report not found.", "err");
                    if (dres) mysql_free_result(dres); waitKey(); continue;
                }

                MYSQL_ROW dr = mysql_fetch_row(dres);
                cout << endl;
                showDivider();
                cout << CLR_BD << CLR_BCY << "    REPORT DETAILS  #" << rid << CLR_RS << endl;
                showDivider();
                showField("Report ID", dr[0] ? dr[0] : "");
                showField("Reporter", dr[1] ? dr[1] : "");
                showField("Reporter Email", dr[2] ? dr[2] : "");
                showField("Booking ID", dr[3] ? dr[3] : "");
                showField("Package", dr[4] ? dr[4] : "");
                showField("Booking Date", dr[5] ? dr[5] : "");
                showField("Current Action", dr[7] ? dr[7] : "");
                showDivider();
                cout << CLR_BYL << "    Description:" << CLR_RS << endl;
                cout << CLR_WH << "    " << (dr[6] ? dr[6] : "") << CLR_RS << endl;
                showDivider();

                string currentAction = dr[7] ? dr[7] : "";
                mysql_free_result(dres);

                if (currentAction != "Pending") {
                    showMsg("This report has already been " + currentAction + ".", "info");
                    waitKey(); continue;
                }

                // Action selection
                cout << endl;
                vector<string> actopts = {"Resolve", "Dismiss", "Cancel"};
                cout << CLR_BWH << "    Choose action:" << CLR_RS << endl;
                cout << CLR_BGR << "      [1] Resolve" << CLR_RS << "  -  Issue acknowledged, action taken" << endl;
                cout << CLR_BRD << "      [2] Dismiss" << CLR_RS << "  -  Report invalid or no action needed" << endl;
                cout << CLR_GY << "      [3] Cancel" << CLR_RS << "   -  Go back without action" << endl;
                cout << endl;
                string choice = getInput("Enter choice (1/2/3): ");

                if (choice == "1" || choice == "2") {
                    string newAction = (choice == "1") ? "Resolved" : "Dismissed";
                    string uq = "UPDATE REPORTS SET admin_action = '" + newAction + "' WHERE report_id = " + rid;
                    if (DBHelper::executeUpdate(conn, uq, {}) < 0) showMsg("Error updating database.", "err");
                    else showMsg("Report #" + rid + " has been marked as '" + newAction + "'.", "ok");
                }
                waitKey();
            }
        }

        // --- Payment Management ---
        else if (c == 3) {
            vector<string> pmopts = {"View All Payments", "Delete Payment", "Back"};
            while (true) {
                int pmc = showMenu("PAYMENT MANAGEMENT", pmopts, "Manage and review payment records");
                if (pmc == 2 || pmc == -1) break;

                if (pmc == 0) {
                    showScreenHeader("ALL PAYMENT RECORDS");
                    string q = "SELECT pay.payment_id, c.name AS customer, p.package_name, "
                               "pay.payment_type, pay.payment_method, pay.payment_date, pay.amount "
                               "FROM PAYMENTS pay "
                               "JOIN BOOKINGS b ON pay.booking_id = b.booking_id "
                               "JOIN USERS c ON b.user_id = c.user_id "
                               "JOIN PACKAGES p ON b.package_id = p.package_id "
                               "ORDER BY pay.payment_date DESC";
                    auto results = DBHelper::executeQuery(conn, q, {});
                    if (results.empty()) {
                        showMsg("No payment records found.", "info");
                        waitKey(); continue;
                    }
                    int cPI=4, cCu=14, cPk=16, cTy=14, cMe=14, cDt=12, cAm=10;
                    cout << CLR_CY << "  \xC9";
                    for(int i=0;i<cPI+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cCu+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cPk+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cTy+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cMe+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cDt+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cAm+2;i++) cout<<"\xCD";
                    cout << "\xBB" << CLR_RS << endl;
                    cout << CLR_CY << "  \xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("ID",cPI+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Customer",cCu+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Package",cPk+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Type",cTy+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Method",cMe+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Date",cDt+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Amt(RM)",cAm+1) << CLR_CY << "\xBA" << CLR_RS << endl;
                    cout << CLR_CY << "  \xCC";
                    for(int i=0;i<cPI+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cCu+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cPk+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cTy+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cMe+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cDt+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cAm+2;i++) cout<<"\xCD";
                    cout << "\xB9" << CLR_RS << endl;
                    MYSQL_ROW row; double totalAmt = 0; int totalPay = 0;
                    for (const auto& row : results) {
                        totalPay++;
                        string amt = row[6];
                        try { totalAmt += stod(amt); } catch(...) {}
                        cout << CLR_CY << "  \xBA" << CLR_RS
                             << CLR_WH << " " << padStr(row[0],cPI+1) << CLR_CY << "\xBA" << CLR_RS
                             << CLR_WH << " " << padStr(row[1],cCu+1) << CLR_CY << "\xBA" << CLR_RS
                             << CLR_WH << " " << padStr(row[2],cPk+1) << CLR_CY << "\xBA" << CLR_RS
                             << CLR_BYL << " " << padStr(row[3],cTy+1) << CLR_RS
                             << CLR_CY << "\xBA" << CLR_RS
                             << CLR_WH << " " << padStr(row[4],cMe+1) << CLR_CY << "\xBA" << CLR_RS
                             << CLR_WH << " " << padStr(row[5],cDt+1) << CLR_CY << "\xBA" << CLR_RS
                             << CLR_BGR << " " << padStr(amt,cAm+1) << CLR_RS
                             << CLR_CY << "\xBA" << CLR_RS << endl;
                    }
                    cout << CLR_CY << "  \xC8";
                    for(int i=0;i<cPI+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cCu+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cPk+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cTy+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cMe+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cDt+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cAm+2;i++) cout<<"\xCD";
                    cout << "\xBC" << CLR_RS << endl;
                    
                    cout << endl;
                    showDivider();
                    ostringstream oss; oss << fixed << setprecision(2) << totalAmt;
                    showField("Total Payments", to_string(totalPay));
                    showField("Total Amount", string("RM ") + oss.str());
                    showDivider();
                    waitKey();

                } else if (pmc == 1) {
                    showScreenHeader("DELETE PAYMENT");
                    string q = "SELECT pay.payment_id, c.name, pay.payment_type, pay.amount, pay.payment_date "
                               "FROM PAYMENTS pay JOIN BOOKINGS b ON pay.booking_id = b.booking_id "
                               "JOIN USERS c ON b.user_id = c.user_id ORDER BY pay.payment_id";
                    auto results = DBHelper::executeQuery(conn, q, {});
                    if (results.empty()) {
                        showMsg("No payments found.", "info");
                        waitKey(); continue;
                    }
                    int cPI=4, cCu=16, cTy=14, cAm=10, cDt=12;
                    cout << CLR_CY << "  \xC9";
                    for(int i=0;i<cPI+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cCu+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cTy+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cAm+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cDt+2;i++) cout<<"\xCD";
                    cout << "\xBB" << CLR_RS << endl;
                    cout << CLR_CY << "  \xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("ID",cPI+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Customer",cCu+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Type",cTy+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Amt(RM)",cAm+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Date",cDt+1) << CLR_CY << "\xBA" << CLR_RS << endl;
                    cout << CLR_CY << "  \xCC";
                    for(int i=0;i<cPI+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cCu+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cTy+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cAm+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cDt+2;i++) cout<<"\xCD";
                    cout << "\xB9" << CLR_RS << endl;
                    for (const auto& row : results) {
                        cout << CLR_CY << "  \xBA" << CLR_RS
                             << CLR_WH << " " << padStr(row[0],cPI+1) << CLR_CY << "\xBA" << CLR_RS
                             << CLR_WH << " " << padStr(row[1],cCu+1) << CLR_CY << "\xBA" << CLR_RS
                             << CLR_WH << " " << padStr(row[2],cTy+1) << CLR_CY << "\xBA" << CLR_RS
                             << CLR_BGR << " " << padStr(row[3],cAm+1) << CLR_RS
                             << CLR_CY << "\xBA" << CLR_RS
                             << CLR_WH << " " << padStr(row[4],cDt+1)
                             << CLR_CY << "\xBA" << CLR_RS << endl;
                    }
                    cout << CLR_CY << "  \xC8";
                    for(int i=0;i<cPI+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cCu+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cTy+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cAm+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cDt+2;i++) cout<<"\xCD";
                    cout << "\xBC" << CLR_RS << endl;
                    
                    cout << endl;
                    string pid = getInput("Enter Payment ID to delete (0 to cancel): ");
                    if (pid != "0" && !pid.empty()) {
                        string confirm = getInput("Type 'yes' to confirm delete payment #" + pid + ": ");
                        if (confirm == "yes") {
                            string dq = "DELETE FROM PAYMENTS WHERE payment_id = " + pid;
                            if (DBHelper::executeUpdate(conn, dq, {}) < 0) showMsg("Error updating database.", "err");
                            else showMsg("Payment #" + pid + " deleted successfully.", "ok");
                        } else showMsg("Delete cancelled.", "info");
                    }
                    waitKey();
                }
            }
        }

        // --- Review Management ---
        else if (c == 4) {
            vector<string> rmopts = {"View All Reviews", "Delete Review", "Back"};
            while (true) {
                int rmc = showMenu("REVIEW MANAGEMENT", rmopts, "Moderate customer reviews");
                if (rmc == 2 || rmc == -1) break;

                if (rmc == 0) {
                    showScreenHeader("ALL CUSTOMER REVIEWS");
                    string q = "SELECT rv.review_id, c.name AS customer, p.package_name, "
                               "rv.rating, rv.comment "
                               "FROM REVIEWS rv "
                               "JOIN BOOKINGS b ON rv.booking_id = b.booking_id "
                               "JOIN USERS c ON b.user_id = c.user_id "
                               "JOIN PACKAGES p ON b.package_id = p.package_id "
                               "ORDER BY rv.review_id DESC";
                    auto results = DBHelper::executeQuery(conn, q, {});
                    if (results.empty()) {
                        showMsg("No reviews found.", "info");
                        waitKey(); continue;
                    }
                    int cRI=4, cCu=16, cPk=18, cRt=6, cCm=30;
                    cout << CLR_CY << "  \xC9";
                    for(int i=0;i<cRI+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cCu+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cPk+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cRt+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cCm+2;i++) cout<<"\xCD";
                    cout << "\xBB" << CLR_RS << endl;
                    cout << CLR_CY << "  \xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("ID",cRI+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Customer",cCu+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Package",cPk+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Rating",cRt+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Comment",cCm+1) << CLR_CY << "\xBA" << CLR_RS << endl;
                    cout << CLR_CY << "  \xCC";
                    for(int i=0;i<cRI+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cCu+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cPk+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cRt+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cCm+2;i++) cout<<"\xCD";
                    cout << "\xB9" << CLR_RS << endl;
                    for (const auto& row : results) {
                        cout << CLR_CY << "  \xBA" << CLR_RS
                             << CLR_WH << " " << padStr(row[0],cRI+1) << CLR_CY << "\xBA" << CLR_RS
                             << CLR_WH << " " << padStr(row[1],cCu+1) << CLR_CY << "\xBA" << CLR_RS
                             << CLR_WH << " " << padStr(row[2],cPk+1) << CLR_CY << "\xBA" << CLR_RS
                             << CLR_BYL << " " << padStr(row[3],cRt+1) << CLR_RS
                             << CLR_CY << "\xBA" << CLR_RS
                             << CLR_GY << " " << padStr(row[4],cCm+1)
                             << CLR_CY << "\xBA" << CLR_RS << endl;
                    }
                    cout << CLR_CY << "  \xC8";
                    for(int i=0;i<cRI+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cCu+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cPk+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cRt+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cCm+2;i++) cout<<"\xCD";
                    cout << "\xBC" << CLR_RS << endl;
                    
                    cout << endl;
                    string rid = getInput("Enter Review ID to delete (0 to cancel): ");
                    if (rid != "0" && !rid.empty()) {
                        string confirm = getInput("Type 'yes' to confirm delete review #" + rid + ": ");
                        if (confirm == "yes") {
                            string dq = "DELETE FROM REVIEWS WHERE review_id = " + rid;
                            if (DBHelper::executeUpdate(conn, dq, {}) < 0) showMsg("Error updating database.", "err");
                            else showMsg("Review #" + rid + " deleted successfully.", "ok");
                        } else showMsg("Delete cancelled.", "info");
                    }
                    waitKey();
                }
            }
        }

        // --- Business Reports ---
        else if (c == 5) {
            vector<string> bropts = {"Revenue Report", "Activity Report", "Photographer Performance", "Show Analytics Graph (Python)", "Export Revenue to PDF", "Export Activity to PDF", "Back"};
            while (true) {
                int brc = showMenu("BUSINESS REPORTS", bropts, "Generate and export business analytics");
                if (brc == 6 || brc == -1) break;

                // ===== Revenue Report =====
                if (brc == 0 || brc == 4) {
                    string rq = "SELECT u.name AS photographer, p.package_name, "
                                "pay.payment_type, pay.payment_method, pay.payment_date, pay.amount "
                                "FROM PAYMENTS pay "
                                "JOIN BOOKINGS b ON pay.booking_id = b.booking_id "
                                "JOIN PACKAGES p ON b.package_id = p.package_id "
                                "JOIN USERS u ON p.user_id = u.user_id "
                                "ORDER BY pay.payment_date DESC";
                    auto results = DBHelper::executeQuery(conn, rq, {});
                    if (results.empty()) {
                        showMsg("No payment records found.", "info");
                        waitKey(); continue;
                    }

                    // Collect data
                    vector<vector<string>> pdfRows;
                    double totalRev = 0;
                    for (const auto& row : results) {
                        vector<string> r;
                        for (int i = 0; i < 6; i++) r.push_back(row[i]);
                        try { totalRev += stod(r[5]); } catch(...) {}
                        pdfRows.push_back(r);
                    }
                    

                    if (brc == 0) {
                        // Display table on screen
                        showScreenHeader("REVENUE REPORT");
                        int cPh=16, cPkg=18, cType=14, cMethod=14, cDate=12, cAmt=10;

                        cout << CLR_CY << "  \xC9";
                        for(int i=0;i<cPh+2;i++) cout<<"\xCD"; cout<<"\xCB";
                        for(int i=0;i<cPkg+2;i++) cout<<"\xCD"; cout<<"\xCB";
                        for(int i=0;i<cType+2;i++) cout<<"\xCD"; cout<<"\xCB";
                        for(int i=0;i<cMethod+2;i++) cout<<"\xCD"; cout<<"\xCB";
                        for(int i=0;i<cDate+2;i++) cout<<"\xCD"; cout<<"\xCB";
                        for(int i=0;i<cAmt+2;i++) cout<<"\xCD";
                        cout << "\xBB" << CLR_RS << endl;

                        cout << CLR_CY << "  \xBA" << CLR_RS << CLR_BD << CLR_BWH
                             << " " << padStr("Photographer", cPh+1)
                             << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                             << " " << padStr("Package", cPkg+1)
                             << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                             << " " << padStr("Type", cType+1)
                             << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                             << " " << padStr("Method", cMethod+1)
                             << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                             << " " << padStr("Date", cDate+1)
                             << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                             << " " << padStr("Amount(RM)", cAmt+1)
                             << CLR_CY << "\xBA" << CLR_RS << endl;

                        cout << CLR_CY << "  \xCC";
                        for(int i=0;i<cPh+2;i++) cout<<"\xCD"; cout<<"\xCE";
                        for(int i=0;i<cPkg+2;i++) cout<<"\xCD"; cout<<"\xCE";
                        for(int i=0;i<cType+2;i++) cout<<"\xCD"; cout<<"\xCE";
                        for(int i=0;i<cMethod+2;i++) cout<<"\xCD"; cout<<"\xCE";
                        for(int i=0;i<cDate+2;i++) cout<<"\xCD"; cout<<"\xCE";
                        for(int i=0;i<cAmt+2;i++) cout<<"\xCD";
                        cout << "\xB9" << CLR_RS << endl;

                        for (auto& r : pdfRows) {
                            string amtColor = CLR_BGR;
                            cout << CLR_CY << "  \xBA" << CLR_RS
                                 << CLR_WH << " " << padStr(r[0], cPh+1)
                                 << CLR_CY << "\xBA" << CLR_RS
                                 << CLR_WH << " " << padStr(r[1], cPkg+1)
                                 << CLR_CY << "\xBA" << CLR_RS
                                 << CLR_BYL << " " << padStr(r[2], cType+1) << CLR_RS
                                 << CLR_CY << "\xBA" << CLR_RS
                                 << CLR_WH << " " << padStr(r[3], cMethod+1)
                                 << CLR_CY << "\xBA" << CLR_RS
                                 << CLR_WH << " " << padStr(r[4], cDate+1)
                                 << CLR_CY << "\xBA" << CLR_RS
                                 << amtColor << " " << padStr(r[5], cAmt+1) << CLR_RS
                                 << CLR_CY << "\xBA" << CLR_RS << endl;
                        }

                        cout << CLR_CY << "  \xC8";
                        for(int i=0;i<cPh+2;i++) cout<<"\xCD"; cout<<"\xCA";
                        for(int i=0;i<cPkg+2;i++) cout<<"\xCD"; cout<<"\xCA";
                        for(int i=0;i<cType+2;i++) cout<<"\xCD"; cout<<"\xCA";
                        for(int i=0;i<cMethod+2;i++) cout<<"\xCD"; cout<<"\xCA";
                        for(int i=0;i<cDate+2;i++) cout<<"\xCD"; cout<<"\xCA";
                        for(int i=0;i<cAmt+2;i++) cout<<"\xCD";
                        cout << "\xBC" << CLR_RS << endl;

                        cout << endl;
                        showDivider();
                        ostringstream oss; oss << fixed << setprecision(2) << totalRev;
                        double adminCut = totalRev * 0.05;
                        ostringstream ossAdmin; ossAdmin << fixed << setprecision(2) << adminCut;
                        showField("Total Revenue", string("RM ") + oss.str());
                        showField("Admin Commission (5%)", string("RM ") + ossAdmin.str());
                        showField("Transactions", to_string(pdfRows.size()));
                        showDivider();
                        waitKey();
                    } else {
                        // Export to PDF
                        CreateDirectoryA("Revenue Report", NULL);
                        string fname = "Revenue Report\\Revenue_Report_" + getDateStamp() + ".pdf";
                        vector<string> hdrs = {"Photographer", "Package", "Type", "Method", "Date", "Amount(RM)"};
                        ostringstream oss; oss << fixed << setprecision(2) << totalRev;
                        double adminCutPdf = totalRev * 0.05;
                        ostringstream ossAdminPdf; ossAdminPdf << fixed << setprecision(2) << adminCutPdf;
                        vector<string> summary = {
                            "Total Revenue: RM " + oss.str(),
                            "Admin Commission (5%): RM " + ossAdminPdf.str(),
                            "Total Transactions: " + to_string(pdfRows.size()),
                            "Report Date: " + getTimestamp()
                        };
                        if (exportPDF(fname, "Revenue Report - Photography Booking System", hdrs, pdfRows, summary))
                            showMsg("PDF exported: " + fname, "ok");
                        else
                            showMsg("Failed to export PDF.", "err");
                        waitKey();
                    }

                // ===== Activity Report =====
                } else if (brc == 1 || brc == 5) {
                    string aq = "SELECT b.booking_id, c.name AS customer, u2.name AS photographer, "
                                "p.package_name, b.booking_date, b.job_status "
                                "FROM BOOKINGS b "
                                "JOIN USERS c ON b.user_id = c.user_id "
                                "JOIN PACKAGES p ON b.package_id = p.package_id "
                                "JOIN USERS u2 ON p.user_id = u2.user_id "
                                "ORDER BY b.booking_date DESC";
                    auto results = DBHelper::executeQuery(conn, aq, {});
                    if (results.empty()) {
                        showMsg("No booking activity found.", "info");
                        waitKey(); continue;
                    }

                    vector<vector<string>> pdfRows;
                    int cntP=0, cntA=0, cntD=0, cntC=0, cntR=0;
                    for (const auto& row : results) {
                        vector<string> r;
                        for (int i = 0; i < 6; i++) r.push_back(row[i]);
                        string st = r[5];
                        if (st == "Pending") cntP++;
                        else if (st == "Approved") cntA++;
                        else if (st == "Deposit Paid") cntD++;
                        else if (st == "Completed") cntC++;
                        else if (st == "Rejected") cntR++;
                        pdfRows.push_back(r);
                    }
                    

                    if (brc == 1) {
                        showScreenHeader("ACTIVITY REPORT");
                        int cID=4, cCust=14, cPhot=14, cPkg=16, cDate=12, cStat=13;

                        cout << CLR_CY << "  \xC9";
                        for(int i=0;i<cID+2;i++) cout<<"\xCD"; cout<<"\xCB";
                        for(int i=0;i<cCust+2;i++) cout<<"\xCD"; cout<<"\xCB";
                        for(int i=0;i<cPhot+2;i++) cout<<"\xCD"; cout<<"\xCB";
                        for(int i=0;i<cPkg+2;i++) cout<<"\xCD"; cout<<"\xCB";
                        for(int i=0;i<cDate+2;i++) cout<<"\xCD"; cout<<"\xCB";
                        for(int i=0;i<cStat+2;i++) cout<<"\xCD";
                        cout << "\xBB" << CLR_RS << endl;

                        cout << CLR_CY << "  \xBA" << CLR_RS << CLR_BD << CLR_BWH
                             << " " << padStr("ID", cID+1)
                             << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                             << " " << padStr("Customer", cCust+1)
                             << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                             << " " << padStr("Photographer", cPhot+1)
                             << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                             << " " << padStr("Package", cPkg+1)
                             << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                             << " " << padStr("Date", cDate+1)
                             << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                             << " " << padStr("Status", cStat+1)
                             << CLR_CY << "\xBA" << CLR_RS << endl;

                        cout << CLR_CY << "  \xCC";
                        for(int i=0;i<cID+2;i++) cout<<"\xCD"; cout<<"\xCE";
                        for(int i=0;i<cCust+2;i++) cout<<"\xCD"; cout<<"\xCE";
                        for(int i=0;i<cPhot+2;i++) cout<<"\xCD"; cout<<"\xCE";
                        for(int i=0;i<cPkg+2;i++) cout<<"\xCD"; cout<<"\xCE";
                        for(int i=0;i<cDate+2;i++) cout<<"\xCD"; cout<<"\xCE";
                        for(int i=0;i<cStat+2;i++) cout<<"\xCD";
                        cout << "\xB9" << CLR_RS << endl;

                        for (auto& r : pdfRows) {
                            string sc = CLR_WH;
                            if (r[5] == "Completed") sc = CLR_BGR;
                            else if (r[5] == "Pending" || r[5] == "Approved" || r[5] == "Deposit Paid") sc = CLR_BYL;
                            else if (r[5] == "Rejected") sc = CLR_BRD;

                            cout << CLR_CY << "  \xBA" << CLR_RS
                                 << CLR_WH << " " << padStr(r[0], cID+1)
                                 << CLR_CY << "\xBA" << CLR_RS
                                 << CLR_WH << " " << padStr(r[1], cCust+1)
                                 << CLR_CY << "\xBA" << CLR_RS
                                 << CLR_WH << " " << padStr(r[2], cPhot+1)
                                 << CLR_CY << "\xBA" << CLR_RS
                                 << CLR_WH << " " << padStr(r[3], cPkg+1)
                                 << CLR_CY << "\xBA" << CLR_RS
                                 << CLR_WH << " " << padStr(r[4], cDate+1)
                                 << CLR_CY << "\xBA" << CLR_RS
                                 << sc << " " << padStr(r[5], cStat+1) << CLR_RS
                                 << CLR_CY << "\xBA" << CLR_RS << endl;
                        }

                        cout << CLR_CY << "  \xC8";
                        for(int i=0;i<cID+2;i++) cout<<"\xCD"; cout<<"\xCA";
                        for(int i=0;i<cCust+2;i++) cout<<"\xCD"; cout<<"\xCA";
                        for(int i=0;i<cPhot+2;i++) cout<<"\xCD"; cout<<"\xCA";
                        for(int i=0;i<cPkg+2;i++) cout<<"\xCD"; cout<<"\xCA";
                        for(int i=0;i<cDate+2;i++) cout<<"\xCD"; cout<<"\xCA";
                        for(int i=0;i<cStat+2;i++) cout<<"\xCD";
                        cout << "\xBC" << CLR_RS << endl;

                        cout << endl;
                        showDivider();
                        cout << CLR_BD << CLR_BWH << "    BOOKING SUMMARY" << CLR_RS << endl;
                        showDivider();
                        showField("Total Bookings", to_string(pdfRows.size()));
                        cout << CLR_BYL << "    Pending: " << cntP << CLR_RS
                             << CLR_GY << " | " << CLR_RS
                             << CLR_BYL << "Approved: " << cntA << CLR_RS
                             << CLR_GY << " | " << CLR_RS
                             << CLR_BYL << "Deposit: " << cntD << CLR_RS
                             << CLR_GY << " | " << CLR_RS
                             << CLR_BGR << "Done: " << cntC << CLR_RS
                             << CLR_GY << " | " << CLR_RS
                             << CLR_BRD << "Rejected: " << cntR << CLR_RS << endl;
                        showDivider();
                        waitKey();
                    } else {
                        CreateDirectoryA("Activity Report", NULL);
                        string fname = "Activity Report\\Activity_Report_" + getDateStamp() + ".pdf";
                        vector<string> hdrs = {"ID", "Customer", "Photographer", "Package", "Date", "Status"};
                        vector<string> summary = {
                            "Total Bookings: " + to_string(pdfRows.size()),
                            "Pending: " + to_string(cntP) + "  Approved: " + to_string(cntA) + "  Deposit Paid: " + to_string(cntD),
                            "Completed: " + to_string(cntC) + "  Rejected: " + to_string(cntR),
                            "Report Date: " + getTimestamp()
                        };
                        if (exportPDF(fname, "Activity Report - Photography Booking System", hdrs, pdfRows, summary))
                            showMsg("PDF exported: " + fname, "ok");
                        else
                            showMsg("Failed to export PDF.", "err");
                        waitKey();
                    }

                // ===== Photographer Performance =====
                } else if (brc == 2) {
                    showScreenHeader("PHOTOGRAPHER PERFORMANCE");
                    string pq = "SELECT u.name, COUNT(b.booking_id) AS total_jobs, "
                                "COALESCE(SUM(pay.amount), 0) AS total_earned "
                                "FROM USERS u "
                                "LEFT JOIN PACKAGES p ON u.user_id = p.user_id "
                                "LEFT JOIN BOOKINGS b ON p.package_id = b.package_id AND b.job_status = 'Completed' "
                                "LEFT JOIN PAYMENTS pay ON b.booking_id = pay.booking_id "
                                "WHERE u.role = 'Photographer' AND u.account_status = 'Active' "
                                "GROUP BY u.user_id, u.name ORDER BY total_earned DESC";
                    auto results = DBHelper::executeQuery(conn, pq, {});
                    if (results.empty()) {
                        showMsg("No photographer data found.", "info");
                        waitKey(); continue;
                    }

                    int cRank=5, cName=20, cJobs=10, cEarned=14;

                    cout << CLR_CY << "  \xC9";
                    for(int i=0;i<cRank+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cName+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cJobs+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cEarned+2;i++) cout<<"\xCD";
                    cout << "\xBB" << CLR_RS << endl;

                    cout << CLR_CY << "  \xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Rank", cRank+1)
                         << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Photographer", cName+1)
                         << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Jobs Done", cJobs+1)
                         << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Earned (RM)", cEarned+1)
                         << CLR_CY << "\xBA" << CLR_RS << endl;

                    cout << CLR_CY << "  \xCC";
                    for(int i=0;i<cRank+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cName+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cJobs+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cEarned+2;i++) cout<<"\xCD";
                    cout << "\xB9" << CLR_RS << endl;

                    MYSQL_ROW row; int rank = 1;
                    for (const auto& row : results) {
                        string srank = "#" + to_string(rank++);
                        string sname = row[0];
                        string sjobs = row[1];
                        string searned = row[2];

                        string rankColor = (rank <= 2) ? CLR_BYL : ((rank <= 3) ? CLR_WH : CLR_GY);

                        cout << CLR_CY << "  \xBA" << CLR_RS
                             << rankColor << " " << padStr(srank, cRank+1) << CLR_RS
                             << CLR_CY << "\xBA" << CLR_RS
                             << CLR_WH << " " << padStr(sname, cName+1)
                             << CLR_CY << "\xBA" << CLR_RS
                             << CLR_BCY << " " << padStr(sjobs, cJobs+1) << CLR_RS
                             << CLR_CY << "\xBA" << CLR_RS
                             << CLR_BGR << " " << padStr(searned, cEarned+1) << CLR_RS
                             << CLR_CY << "\xBA" << CLR_RS << endl;
                    }

                    cout << CLR_CY << "  \xC8";
                    for(int i=0;i<cRank+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cName+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cJobs+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cEarned+2;i++) cout<<"\xCD";
                    cout << "\xBC" << CLR_RS << endl;

                    
                    waitKey();
                }

                // ===== Show Analytics Graph (Python) =====
                else if (brc == 3) {
                    showScreenHeader("GENERATING GRAPH DATA");
                    showMsg("Fetching database metrics...", "info");
                    
                    ostringstream json;
                    json << "{\n";
                    
                    // 1. Photographer Performance
                    json << "  \"photographers\": [\n";
                    string q1 = "SELECT u.name, COUNT(b.booking_id) AS total_jobs, COALESCE(SUM(pay.amount), 0) AS total_earned "
                                "FROM USERS u "
                                "LEFT JOIN PACKAGES p ON u.user_id = p.user_id "
                                "LEFT JOIN BOOKINGS b ON p.package_id = b.package_id AND b.job_status = 'Completed' "
                                "LEFT JOIN PAYMENTS pay ON b.booking_id = pay.booking_id "
                                "WHERE u.role = 'Photographer' AND u.account_status = 'Active' "
                                "GROUP BY u.user_id, u.name ORDER BY total_earned DESC";
                    auto r1 = DBHelper::executeQuery(conn, q1, {});
                    for (size_t i = 0; i < r1.size(); ++i) {
                        string name = r1[i][0];
                        string safeName = "";
                        for (char ch : name) {
                            if (ch == '"' || ch == '\\') safeName += '\\';
                            safeName += ch;
                        }
                        int jobs = 0; try { jobs = stoi(r1[i][1]); } catch(...) {}
                        double earned = 0.0; try { earned = stod(r1[i][2]); } catch(...) {}
                        json << "    {\"name\": \"" << safeName << "\", \"jobs\": " << jobs << ", \"earned\": " << earned << "}";
                        if (i < r1.size() - 1) json << ",";
                        json << "\n";
                    }
                    json << "  ],\n";
                    
                    // 2. Booking Status Distribution
                    json << "  \"booking_statuses\": {\n";
                    string q2 = "SELECT job_status, COUNT(*) FROM BOOKINGS GROUP BY job_status";
                    auto r2 = DBHelper::executeQuery(conn, q2, {});
                    for (size_t i = 0; i < r2.size(); ++i) {
                        string status = r2[i][0];
                        int count = 0; try { count = stoi(r2[i][1]); } catch(...) {}
                        json << "    \"" << status << "\": " << count;
                        if (i < r2.size() - 1) json << ",";
                        json << "\n";
                    }
                    json << "  },\n";
                    
                    // 3. Revenue Trend
                    json << "  \"revenue_trend\": [\n";
                    string q3 = "SELECT payment_date, SUM(amount) FROM PAYMENTS GROUP BY payment_date ORDER BY payment_date ASC";
                    auto r3 = DBHelper::executeQuery(conn, q3, {});
                    for (size_t i = 0; i < r3.size(); ++i) {
                        string date = r3[i][0];
                        double amount = 0.0; try { amount = stod(r3[i][1]); } catch(...) {}
                        json << "    {\"date\": \"" << date << "\", \"amount\": " << amount << "}";
                        if (i < r3.size() - 1) json << ",";
                        json << "\n";
                    }
                    json << "  ],\n";
                    
                    // 4. Payment Methods Split
                    json << "  \"payment_methods\": {\n";
                    string q4 = "SELECT payment_method, COUNT(*) FROM PAYMENTS GROUP BY payment_method";
                    auto r4 = DBHelper::executeQuery(conn, q4, {});
                    for (size_t i = 0; i < r4.size(); ++i) {
                        string method = r4[i][0];
                        int count = 0; try { count = stoi(r4[i][1]); } catch(...) {}
                        json << "    \"" << method << "\": " << count;
                        if (i < r4.size() - 1) json << ",";
                        json << "\n";
                    }
                    json << "  }\n";
                    
                    json << "}\n";
                    
                    // Write to graph_data.json
                    ofstream fout("graph_data.json");
                    if (fout.is_open()) {
                        fout << json.str();
                        fout.close();
                        showMsg("Analytics metrics exported successfully to graph_data.json", "ok");
                        showMsg("Launching Python Interactive Analytics GUI...", "info");
                        system("python show_graph.py");
                    } else {
                        showMsg("Failed to write to graph_data.json!", "err");
                    }
                    waitKey();
                }
            }
        }
    }
}
