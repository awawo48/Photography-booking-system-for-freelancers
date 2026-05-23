#include "Dashboard/Customer.hpp"
#include "UIHelper.hpp"
#include "Database.hpp"
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>

using namespace std;

void customerDashboard(MYSQL* conn, int userId) {
    vector<string> opts = {"Browse Available Packages", "Make a Booking", "Make Payment", "Leave a Review", "Back / Logout"};
    while (true) {
        string sub = "";
        auto notifRes = DBHelper::executeQuery(conn, "SELECT message FROM NOTIFICATIONS WHERE target_role IN ('All', 'Customer') ORDER BY created_at DESC LIMIT 1", {});
        if (!notifRes.empty()) sub = "  " + CLR_BYL + "[BROADCAST]: " + CLR_RS + notifRes[0][0];

        int c = showMenu("CUSTOMER DASHBOARD  (ID: " + to_string(userId) + ")", opts, sub);
        if (c == 4 || c == -1) break;

        // --- Browse Packages ---
        if (c == 0) {
            showScreenHeader("AVAILABLE PACKAGES");
            string q = "SELECT p.package_id, u.name AS photographer_name, p.package_name, "
                       "p.category, p.price, p.details FROM PACKAGES p JOIN USERS u ON p.user_id = u.user_id";
            auto results = DBHelper::executeQuery(conn, q, {});
            for (const auto& row : results) {
                showDivider();
                showField("Package ID", row[0]);
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
            string pkg_id = getInput("Enter Package ID to book: ");
            string bdate = getInput("Enter Booking Date (YYYY-MM-DD): ");
            if (pkg_id.empty() || bdate.empty()) { showMsg("Cancelled.", "info"); waitKey(); continue; }

            string fq = "SELECT user_id FROM PACKAGES WHERE package_id = ?";
            auto pkgResults = DBHelper::executeQuery(conn, fq, {pkg_id});
            if (pkgResults.empty()) {
                showMsg("Package not found!", "err");
                waitKey(); continue;
            }
            int photographer_id = stoi(pkgResults[0][0]);

            string cq = "SELECT status FROM SCHEDULES WHERE user_id = ? AND target_date = ? AND status = 'Blocked'";
            auto blockResults = DBHelper::executeQuery(conn, cq, {to_string(photographer_id), bdate});
            if (!blockResults.empty()) {
                showMsg("Photographer not available on " + bdate + ".", "err");
            } else {
                string iq = "INSERT INTO BOOKINGS (user_id, package_id, booking_date, job_status) VALUES (?, ?, ?, 'Pending')";
                if (DBHelper::executeUpdate(conn, iq, {to_string(userId), pkg_id, bdate}) < 0) showMsg("Booking Failed.", "err");
                else showMsg("Booking placed successfully! Status: Pending.", "ok");
            }
            waitKey();

        // --- Make Payment ---
        } else if (c == 2) {
            showScreenHeader("MAKE PAYMENT");
            string q = "SELECT b.booking_id, p.package_name, b.booking_date, b.job_status "
                       "FROM BOOKINGS b JOIN PACKAGES p ON b.package_id = p.package_id "
                       "WHERE b.user_id = ? AND b.job_status = 'Approved'";
            auto results = DBHelper::executeQuery(conn, q, {to_string(userId)});
            if (results.empty()) {
                showMsg("No approved bookings for payment.", "info");
                waitKey(); continue;
            }
            for (const auto& row : results) {
                showDivider();
                showField("Booking ID", row[0]);
                showField("Package", row[1]);
                showField("Booking Date", row[2]);
                showField("Status", row[3]);
            }
            showDivider();
            cout << endl;
            string bid = getInput("Enter Booking ID to pay: ");
            if (bid.empty()) { showMsg("Cancelled.", "info"); waitKey(); continue; }

            // Fetch original package price
            string pq = "SELECT p.price FROM PACKAGES p JOIN BOOKINGS b ON p.package_id = b.package_id WHERE b.booking_id = ?";
            auto pRes = DBHelper::executeQuery(conn, pq, {bid});
            if (pRes.empty()) { showMsg("Booking not found.", "err"); waitKey(); continue; }

            string price = pRes[0][0];
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

            string iq = "INSERT INTO PAYMENTS (booking_id, amount, payment_type, payment_method, payment_date) VALUES (?, ?, 'Deposit', ?, ?)";
            if (DBHelper::executeUpdate(conn, iq, {bid, amt, pm, pd}) < 0) { showMsg("Payment Failed.", "err"); }
            else {
                string uq = "UPDATE BOOKINGS SET job_status = 'Deposit Paid' WHERE booking_id = ?";
                if (DBHelper::executeUpdate(conn, uq, {bid}) < 0) showMsg("Error updating status.", "err");
                else showMsg("Payment recorded! Status updated to 'Deposit Paid'.", "ok");
            }
            waitKey();

        // --- Leave Review ---
        } else if (c == 3) {
            showScreenHeader("LEAVE A REVIEW");
            string q = "SELECT b.booking_id, p.package_name, b.booking_date "
                       "FROM BOOKINGS b JOIN PACKAGES p ON b.package_id = p.package_id "
                       "WHERE b.user_id = ? AND b.job_status = 'Completed'";
            auto results = DBHelper::executeQuery(conn, q, {to_string(userId)});
            if (results.empty()) {
                showMsg("No completed bookings for review.", "info");
                waitKey(); continue;
            }
            for (const auto& row : results) {
                showDivider();
                showField("Booking ID", row[0]);
                showField("Package", row[1]);
                showField("Booking Date", row[2]);
            }
            showDivider();
            cout << endl;
            string bid = getInput("Enter Booking ID to review: ");
            string rating = getInput("Enter Rating (1-5): ");
            int r = 0;
            try { r = stoi(rating); } catch (...) {}
            if (r < 1 || r > 5) { showMsg("Invalid rating (1-5).", "err"); waitKey(); continue; }
            string comment = getInput("Enter Comment: ");
            string iq = "INSERT INTO REVIEWS (booking_id, rating, comment) VALUES (?, ?, ?)";
            if (DBHelper::executeUpdate(conn, iq, {bid, rating, comment}) < 0) showMsg("Review Failed.", "err");
            else showMsg("Review submitted! Thank you!", "ok");
            waitKey();
        }
    }
}
