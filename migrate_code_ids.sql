ALTER TABLE BOOKINGS ADD COLUMN booking_code VARCHAR(30) UNIQUE AFTER booking_id;
ALTER TABLE PACKAGES ADD COLUMN package_code VARCHAR(30) UNIQUE AFTER package_id;
ALTER TABLE REPORTS ADD COLUMN report_code VARCHAR(30) UNIQUE AFTER report_id;

-- Ensure indexes for fast lookups
CREATE INDEX idx_booking_code ON BOOKINGS(booking_code);
CREATE INDEX idx_package_code ON PACKAGES(package_code);
CREATE INDEX idx_report_code ON REPORTS(report_code);
