#include "Dashboard/Customer.hpp"
#include "UIHelper.hpp"
#include "Database.hpp"
#include "CodeGenerator.hpp"
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>

using namespace std;

void customerDashboard(MYSQL* conn, const UserSession& session) {
    int userId = session.userId;
    vector<string> opts = {"Browse Available Packages", "Make a Booking", "Pay Deposit", "Pay Final Balance", "Leave a Review", "Back / Logout"};
    while (true) {
        string sub = "";
        auto notifRes = DBHelper::executeQuery(conn, "SELECT message FROM NOTIFICATIONS WHERE target_role IN ('All', 'Customer') ORDER BY created_at DESC LIMIT 1", {});
        if (!notifRes.empty()) sub = "  " + CLR_BYL + "[BROADCAST]: " + CLR_RS + notifRes[0][0];

        int c = showMenu("CUSTOMER DASHBOARD  (ID: " + to_string(userId) + " | " + session.name + ")", opts, sub);
        if (c == 5 || c == -1) break;

        // --- Browse Packages ---
        if (c == 0) {
            showScreenHeader("AVAILABLE PACKAGES");
            string q = "SELECT p.package_code, u.name AS photographer_name, p.package_name, "
                       "p.category, p.price, p.details FROM PACKAGES p JOIN USERS u ON p.user_id = u.user_id";
            auto results = DBHelper::executeQuery(conn, q, {});
            for (const auto& row : results) {
                showDivider();
                showField("Package Code", row[0]);
                showField("Photographer", row[1]);
                showField("Package Name", row[2]);
                showField("Category", row[3]);
                showField("Price", string("RM ") + row[4]);
                showField("Details", row[5]);
            }
            if (!results.empty()) showDivider();
            waitKey();

        // --- Make Booking ---
        } else if (c == 1) {
            showScreenHeader("MAKE A BOOKING");
            string pkg_code = getInput("Enter Package Code to book (e.g., PKG-001): ");
            string bdate = getInput("Enter Booking Date (YYYY-MM-DD): ");
            if (pkg_code.empty() || bdate.empty()) { showMsg("Cancelled.", "info"); waitKey(); continue; }

            string fq = "SELECT package_id, user_id FROM PACKAGES WHERE package_code = ?";
            auto pkgResults = DBHelper::executeQuery(conn, fq, {pkg_code});
            if (pkgResults.empty()) {
                showMsg("Package not found!", "err");
                waitKey(); continue;
            }
            string pkg_id = pkgResults[0][0];
            int photographer_id = stoi(pkgResults[0][1]);

            string cq = "SELECT status FROM SCHEDULES WHERE user_id = ? AND target_date = ? AND status = 'Blocked'";
            auto blockResults = DBHelper::executeQuery(conn, cq, {to_string(photographer_id), bdate});
            if (!blockResults.empty()) {
                showMsg("Photographer not available on " + bdate + ".", "err");
            } else {
                string bcode = CodeGenerator::generate(conn, "BKG", "BOOKINGS", "booking_code", true, false);
                string iq = "INSERT INTO BOOKINGS (user_id, package_id, booking_code, booking_date, job_status) VALUES (?, ?, ?, ?, 'Pending')";
                if (DBHelper::executeUpdate(conn, iq, {to_string(userId), pkg_id, bcode, bdate}) < 0) showMsg("Booking Failed.", "err");
                else showMsg("Booking placed successfully! Booking Code: " + bcode, "ok");
            }
            waitKey();

        // --- Pay Deposit (for Approved bookings) ---
        } else if (c == 2) {
            showScreenHeader("PAY DEPOSIT");
            string q = "SELECT b.booking_code, p.package_name, b.booking_date, b.job_status "
                       "FROM BOOKINGS b JOIN PACKAGES p ON b.package_id = p.package_id "
                       "WHERE b.user_id = ? AND b.job_status = 'Approved'";
            auto results = DBHelper::executeQuery(conn, q, {to_string(userId)});
            if (results.empty()) {
                showMsg("No approved bookings for deposit payment.", "info");
                waitKey(); continue;
            }
            for (const auto& row : results) {
                showDivider();
                showField("Booking Code", row[0]);
                showField("Package", row[1]);
                showField("Booking Date", row[2]);
                showField("Status", row[3]);
            }
            showDivider();
            cout << endl;
            string bcode = getInput("Enter Booking Code to pay deposit: ");
            if (bcode.empty()) { showMsg("Cancelled.", "info"); waitKey(); continue; }

            // Fetch original package price and internal booking_id
            string pq = "SELECT p.price, b.booking_id FROM PACKAGES p JOIN BOOKINGS b ON p.package_id = b.package_id WHERE b.booking_code = ?";
            auto pRes = DBHelper::executeQuery(conn, pq, {bcode});
            if (pRes.empty()) { showMsg("Booking not found.", "err"); waitKey(); continue; }

            string price = pRes[0][0];
            string bid = pRes[0][1];
            double finalPrice = 0; try { finalPrice = stod(price); } catch(...) {}
            
            cout << "  " << CLR_CY << "Original Package Price: " << CLR_WH << "RM " << fixed << setprecision(2) << finalPrice << CLR_RS << endl;
            
            string promo = getInput("Enter Promo Code (leave blank if none): ");
            if (!promo.empty()) {
                string promoQ = "SELECT discount_pct FROM PROMO_CODES WHERE code = ? AND valid_until >= CURDATE()";
                auto promoRes = DBHelper::executeQuery(conn, promoQ, {promo});
                if (!promoRes.empty()) {
                    double pct = 0; try { pct = stod(promoRes[0][0]); } catch(...) {}
                    finalPrice = finalPrice * (1.0 - (pct / 100.0));
                    cout << "  " << CLR_BGR << "Promo Applied! Discount: " << pct << "%" << CLR_RS << endl;
                    cout << "  " << CLR_CY << "Discounted Price: " << CLR_WH << "RM " << finalPrice << CLR_RS << endl;
                } else {
                    cout << "  " << CLR_BRD << "Invalid or Expired Promo Code." << CLR_RS << endl;
                }
            }
            
            cout << endl;
            string amt = getInput("Enter Deposit Amount (RM): ");
            string pm = getInput("Enter Payment Method: ");
            string pd = getInput("Enter Payment Date (YYYY-MM-DD): ");

            string pcode = CodeGenerator::generate(conn, "PAY", "PAYMENTS", "payment_code", true, true);
            string iq = "INSERT INTO PAYMENTS (booking_id, payment_code, amount, payment_type, payment_method, payment_date) VALUES (?, ?, ?, 'Deposit', ?, ?)";
            if (DBHelper::executeUpdate(conn, iq, {bid, pcode, amt, pm, pd}) < 0) { showMsg("Payment Failed.", "err"); }
            else {
                // State transition guard: only Approved → Deposit Paid
                string uq = "UPDATE BOOKINGS SET job_status = 'Deposit Paid' WHERE booking_id = ? AND job_status = 'Approved'";
                int rows = DBHelper::executeUpdate(conn, uq, {bid});
                if (rows < 0) showMsg("Error updating status.", "err");
                else if (rows == 0) showMsg("Payment recorded, but booking status could not be updated (may no longer be 'Approved').", "err");
                else showMsg("Deposit recorded! Status updated to 'Deposit Paid'.", "ok");
            }
            waitKey();

        // --- Pay Final Balance (for Deposit Paid bookings) ---
        } else if (c == 3) {
            showScreenHeader("PAY FINAL BALANCE");
            string q = "SELECT b.booking_code, p.package_name, p.price, b.booking_date, b.job_status, b.booking_id "
                       "FROM BOOKINGS b JOIN PACKAGES p ON b.package_id = p.package_id "
                       "WHERE b.user_id = ? AND b.job_status = 'Deposit Paid'";
            auto results = DBHelper::executeQuery(conn, q, {to_string(userId)});
            if (results.empty()) {
                showMsg("No pending balance payments found.", "info");
                waitKey(); continue;
            }
            for (const auto& row : results) {
                string bcode = row[0];
                string bkId = row[5];
                double pkgPrice = 0; try { pkgPrice = stod(row[2]); } catch(...) {}

                auto paidRes = DBHelper::executeQuery(conn, "SELECT COALESCE(SUM(amount), 0) FROM PAYMENTS WHERE booking_id = ?", {bkId});
                double totalPaid = 0;
                if (!paidRes.empty()) { try { totalPaid = stod(paidRes[0][0]); } catch(...) {} }
                double remaining = pkgPrice - totalPaid;
                if (remaining < 0) remaining = 0;

                showDivider();
                showField("Booking Code", bcode);
                showField("Package", row[1]);
                showField("Package Price", string("RM ") + row[2]);
                showField("Booking Date", row[3]);
                showField("Status", row[4]);

                ostringstream oss;
                oss << fixed << setprecision(2) << totalPaid;
                showField("Total Paid", string("RM ") + oss.str());

                ostringstream oss2;
                oss2 << fixed << setprecision(2) << remaining;
                showField("Remaining Balance", string("RM ") + oss2.str());
            }
            showDivider();
            cout << endl;
            string bcode = getInput("Enter Booking Code to pay balance (leave empty to cancel): ");
            if (bcode.empty()) { waitKey(); continue; }

            string getBid = "SELECT b.booking_id, p.price FROM BOOKINGS b JOIN PACKAGES p ON b.package_id = p.package_id WHERE b.booking_code = ? AND b.job_status = 'Deposit Paid'";
            auto bidRes = DBHelper::executeQuery(conn, getBid, {bcode});
            if (bidRes.empty()) { showMsg("Booking not found or not in 'Deposit Paid' status.", "err"); waitKey(); continue; }
            string bid = bidRes[0][0];
            double pkgPrice = 0; try { pkgPrice = stod(bidRes[0][1]); } catch(...) {}
            
            auto paidRes = DBHelper::executeQuery(conn, "SELECT COALESCE(SUM(amount), 0) FROM PAYMENTS WHERE booking_id = ?", {bid});
            double totalPaid = 0;
            if (!paidRes.empty()) { try { totalPaid = stod(paidRes[0][0]); } catch(...) {} }
            double remaining = pkgPrice - totalPaid;
            if (remaining < 0) remaining = 0;

            cout << "  " << CLR_CY << "Remaining Balance: " << CLR_WH << "RM " << fixed << setprecision(2) << remaining << CLR_RS << endl;
            cout << endl;

            string amt = getInput("Enter Payment Amount (RM): ");
            string pm = getInput("Enter Payment Method: ");
            string pd = getInput("Enter Payment Date (YYYY-MM-DD): ");

            string pcode = CodeGenerator::generate(conn, "PAY", "PAYMENTS", "payment_code", true, true);
            string iq = "INSERT INTO PAYMENTS (booking_id, payment_code, amount, payment_type, payment_method, payment_date) VALUES (?, ?, ?, 'Final Payment', ?, ?)";
            if (DBHelper::executeUpdate(conn, iq, {bid, pcode, amt, pm, pd}) < 0) {
                showMsg("Payment Failed.", "err");
            } else {
                string uq = "UPDATE BOOKINGS SET job_status = 'Completed' WHERE booking_id = ?";
                DBHelper::executeUpdate(conn, uq, {bid});
                showMsg("Final payment of RM " + amt + " recorded successfully! Status updated to 'Completed'.", "ok");
            }
            waitKey();

        // --- Leave Review (for Completed bookings) ---
        } else if (c == 4) {
            showScreenHeader("LEAVE A REVIEW");
            string q = "SELECT b.booking_code, p.package_name, b.booking_date "
                       "FROM BOOKINGS b JOIN PACKAGES p ON b.package_id = p.package_id "
                       "WHERE b.user_id = ? AND b.job_status = 'Completed'";
            auto results = DBHelper::executeQuery(conn, q, {to_string(userId)});
            if (results.empty()) {
                showMsg("No completed bookings for review.", "info");
                waitKey(); continue;
            }
            for (const auto& row : results) {
                showDivider();
                showField("Booking Code", row[0]);
                showField("Package", row[1]);
                showField("Booking Date", row[2]);
            }
            showDivider();
            cout << endl;
            string bcode = getInput("Enter Booking Code to review: ");
            
            string getBid = "SELECT booking_id FROM BOOKINGS WHERE booking_code = ?";
            auto bidRes = DBHelper::executeQuery(conn, getBid, {bcode});
            if (bidRes.empty()) { showMsg("Booking not found.", "err"); waitKey(); continue; }
            string bid = bidRes[0][0];

            int ratingVal = getIntInput("Enter Rating (1-5): ");
            int r = ratingVal;
            if (r < 1 || r > 5) {
                showMsg("Invalid rating! Must be 1-5.", "err");
                waitKey(); continue;
            }
            string comment = getInput("Enter Comment: ");
            string rcode = CodeGenerator::generate(conn, "REV", "REVIEWS", "review_code", false, false);
            string iq = "INSERT INTO REVIEWS (booking_id, review_code, rating, comment) VALUES (?, ?, ?, ?)";
            if (DBHelper::executeUpdate(conn, iq, {bid, rcode, to_string(r), comment}) < 0) showMsg("Review Failed.", "err");
            else showMsg("Review submitted! Thank you!", "ok");
            waitKey();
        }
    }
}
