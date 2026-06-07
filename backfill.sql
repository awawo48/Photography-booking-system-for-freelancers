UPDATE BOOKINGS SET booking_code = CONCAT('BKG-202606-', LPAD(booking_id, 3, '0')) WHERE booking_code IS NULL;
UPDATE PACKAGES SET package_code = CONCAT('PKG-', LPAD(package_id, 3, '0')) WHERE package_code IS NULL;
UPDATE REPORTS SET report_code = CONCAT('RPT-20260601-', LPAD(report_id, 2, '0')) WHERE report_code IS NULL;
UPDATE PAYMENTS SET payment_code = CONCAT('PAY-20260601-', LPAD(payment_id, 3, '0')) WHERE payment_code IS NULL;
UPDATE PORTFOLIOS SET portfolio_code = CONCAT('PTF-', LPAD(portfolio_id, 3, '0')) WHERE portfolio_code IS NULL;
UPDATE REVIEWS SET review_code = CONCAT('REV-', LPAD(review_id, 3, '0')) WHERE review_code IS NULL;
