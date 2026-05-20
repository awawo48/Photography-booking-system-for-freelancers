-- ============================================================
-- PHOTOGRAPHY BOOKING SYSTEM — Dummy Data Script
-- Database: photography_db
-- Currency: Ringgit Malaysia (RM)
-- Generated: 2026-05-12
-- ============================================================
-- IMPORTANT: Run this AFTER creating the schema (CREATE TABLE).
-- This script uses DELETE to clear existing data first.
-- ============================================================

USE photography_db;

-- ======================== CLEAN SLATE ========================
-- Delete in reverse dependency order to respect FK constraints
DELETE FROM REPORTS;
DELETE FROM REVIEWS;
DELETE FROM PAYMENTS;
DELETE FROM BOOKINGS;
DELETE FROM SCHEDULES;
DELETE FROM PACKAGES;
DELETE FROM PORTFOLIOS;
DELETE FROM USERS;

-- Reset AUTO_INCREMENT counters
ALTER TABLE USERS        AUTO_INCREMENT = 1;
ALTER TABLE PORTFOLIOS   AUTO_INCREMENT = 1;
ALTER TABLE PACKAGES     AUTO_INCREMENT = 1;
ALTER TABLE SCHEDULES    AUTO_INCREMENT = 1;
ALTER TABLE BOOKINGS     AUTO_INCREMENT = 1;
ALTER TABLE PAYMENTS     AUTO_INCREMENT = 1;
ALTER TABLE REVIEWS      AUTO_INCREMENT = 1;
ALTER TABLE REPORTS      AUTO_INCREMENT = 1;

-- ============================================================
-- 1. USERS (11 total: 1 Admin + 5 Photographers + 5 Customers)
-- ============================================================
-- user_id 1  : Admin
-- user_id 2-6: Photographers (user_id 6 is BANNED)
-- user_id 7-11: Customers
-- ============================================================

INSERT INTO USERS (name, email, password, phone_no, role, account_status) VALUES
-- Admin
('Admin',              'admin@photobooking.com',     '240be518fabd2724ddb6f04eeb1da5967448d7e831c08c8fa822809f74c720a9',    '012-0000000', 'Admin',        'Active'),

-- Photographers
('Ahmad Faris',        'ahmad.faris@gmail.com',      'df30144ce60871874949d292da445f339b12791a70b0220d86119147f223fa1d',    '012-3456789', 'Photographer', 'Active'),
('Nurul Aisyah',       'nurul.aisyah@gmail.com',     'df30144ce60871874949d292da445f339b12791a70b0220d86119147f223fa1d',    '013-9876543', 'Photographer', 'Active'),
('Daniel Lim',         'daniel.lim@gmail.com',        'df30144ce60871874949d292da445f339b12791a70b0220d86119147f223fa1d',    '014-5551234', 'Photographer', 'Active'),
('Siti Aminah',        'siti.aminah@gmail.com',       'df30144ce60871874949d292da445f339b12791a70b0220d86119147f223fa1d',    '015-1112233', 'Photographer', 'Active'),
('Ravi Kumar',         'ravi.kumar@gmail.com',        'df30144ce60871874949d292da445f339b12791a70b0220d86119147f223fa1d',    '016-7778899', 'Photographer', 'Banned'),  -- BANNED photographer

-- Customers
('Tan Wei Ming',       'tan.weiming@gmail.com',       '4f21b18a4c743a5da01bb3a4955dea0a0294a0b4f7977b454c7259e37b2e6c19',     '017-1234567', 'Customer',     'Active'),
('Farah Hanim',        'farah.hanim@gmail.com',       '4f21b18a4c743a5da01bb3a4955dea0a0294a0b4f7977b454c7259e37b2e6c19',     '018-2345678', 'Customer',     'Active'),
('Lim Jia Yi',         'lim.jiayi@gmail.com',         '4f21b18a4c743a5da01bb3a4955dea0a0294a0b4f7977b454c7259e37b2e6c19',     '019-3456789', 'Customer',     'Active'),
('Muhammad Haziq',     'haziq.muhammad@gmail.com',    '4f21b18a4c743a5da01bb3a4955dea0a0294a0b4f7977b454c7259e37b2e6c19',     '011-4567890', 'Customer',     'Active'),
('Priya Devi',         'priya.devi@gmail.com',        '4f21b18a4c743a5da01bb3a4955dea0a0294a0b4f7977b454c7259e37b2e6c19',     '012-5678901', 'Customer',     'Active');

-- ============================================================
-- 2. PORTFOLIOS (Sample portfolio items for photographers)
-- ============================================================

INSERT INTO PORTFOLIOS (user_id, title, media_link, upload_date) VALUES
(2, 'Sunset Wedding at Port Dickson',       'https://portfolio.my/ahmad/sunset-wedding.jpg',      '2025-06-15'),
(2, 'Corporate Headshots Collection',        'https://portfolio.my/ahmad/corporate-headshots.jpg',  '2025-08-20'),
(3, 'Hari Raya Family Portraits',           'https://portfolio.my/nurul/raya-family.jpg',          '2025-05-01'),
(3, 'Nature & Landscape Series',            'https://portfolio.my/nurul/nature-landscape.jpg',     '2025-09-10'),
(4, 'Street Photography KL',                'https://portfolio.my/daniel/street-kl.jpg',           '2025-07-22'),
(5, 'Traditional Malay Wedding',            'https://portfolio.my/siti/malay-wedding.jpg',         '2025-04-18'),
(6, 'Product Photography Samples',          'https://portfolio.my/ravi/product-samples.jpg',       '2025-03-05');

-- ============================================================
-- 3. PACKAGES (At least 2 per photographer = 10+ packages)
-- ============================================================
-- Photographer user_id mapping:
--   Ahmad Faris   = 2
--   Nurul Aisyah  = 3
--   Daniel Lim    = 4
--   Siti Aminah   = 5
--   Ravi Kumar    = 6 (Banned, but packages remain in DB)
-- ============================================================

INSERT INTO PACKAGES (user_id, package_name, category, price, details) VALUES
-- Ahmad Faris (user_id = 2)
(2, 'Wedding Basic',         'Wedding',      2500.00, '4 hours coverage, 100 edited photos, 1 photographer'),
(2, 'Wedding Premium',       'Wedding',      5500.00, '8 hours coverage, 300 edited photos, 2 photographers, drone shots'),
(2, 'Corporate Event',       'Corporate',    1800.00, '3 hours coverage, 80 edited photos, on-site printing'),

-- Nurul Aisyah (user_id = 3)
(3, 'Family Portrait',       'Portrait',     800.00,  '1 hour session, 30 edited photos, indoor/outdoor'),
(3, 'Graduation Shoot',      'Portrait',     600.00,  '1.5 hours session, 25 edited photos, convocation attire'),

-- Daniel Lim (user_id = 4)
(4, 'Street Photography',    'Lifestyle',    1200.00, '2 hours walkabout, 50 edited photos, candid style'),
(4, 'Food & Product',        'Commercial',   1500.00, '3 hours studio session, 40 product shots, white background'),

-- Siti Aminah (user_id = 5)
(5, 'Nikah Ceremony',        'Wedding',      3500.00, '6 hours coverage, 200 edited photos, cinematic video'),
(5, 'Engagement Session',    'Pre-Wedding',  1200.00, '2 hours outdoor session, 60 edited photos'),

-- Ravi Kumar (user_id = 6) — Banned, but data remains
(6, 'Product Catalog',       'Commercial',   2000.00, '4 hours, 100 product shots, lifestyle angles'),
(6, 'Event Coverage',        'Event',        1600.00, '3 hours coverage, 80 edited photos');

-- ============================================================
-- 4. SCHEDULES (Photographer availability)
-- ============================================================

INSERT INTO SCHEDULES (user_id, target_date, status) VALUES
-- Ahmad Faris availability
(2, '2026-06-01', 'Available'),
(2, '2026-06-15', 'Available'),
(2, '2026-06-20', 'Blocked'),    -- Not available on this date

-- Nurul Aisyah availability
(3, '2026-06-05', 'Available'),
(3, '2026-06-10', 'Available'),
(3, '2026-07-01', 'Blocked'),

-- Daniel Lim availability
(4, '2026-06-08', 'Available'),
(4, '2026-06-12', 'Blocked'),

-- Siti Aminah availability
(5, '2026-06-01', 'Available'),
(5, '2026-06-25', 'Available'),

-- Ravi Kumar (Banned, schedule still exists)
(6, '2026-06-15', 'Available');

-- ============================================================
-- 5. BOOKINGS (Various scenarios)
-- ============================================================
-- Booking scenarios:
--   booking_id 1: Pending        (Customer 7 -> Package 1)
--   booking_id 2: Approved       (Customer 8 -> Package 4)
--   booking_id 3: Deposit Paid   (Customer 9 -> Package 6) — has PAYMENT
--   booking_id 4: Completed      (Customer 10 -> Package 8) — has REVIEW
--   booking_id 5: Completed      (Customer 7 -> Package 5)  — has REVIEW
--   booking_id 6: Rejected       (Customer 11 -> Package 2)
--   booking_id 7: Completed      (Customer 8 -> Package 9)  — has REPORT
-- ============================================================

INSERT INTO BOOKINGS (user_id, package_id, booking_date, job_status) VALUES
(7,  1,  '2026-06-01', 'Pending'),         -- Booking 1: Tan Wei Ming -> Ahmad's Wedding Basic
(8,  4,  '2026-06-05', 'Approved'),        -- Booking 2: Farah Hanim -> Nurul's Family Portrait
(9,  6,  '2026-06-08', 'Deposit Paid'),    -- Booking 3: Lim Jia Yi -> Daniel's Street Photography
(10, 8,  '2026-05-10', 'Completed'),       -- Booking 4: Muhammad Haziq -> Siti's Nikah Ceremony
(7,  5,  '2026-04-20', 'Completed'),       -- Booking 5: Tan Wei Ming -> Nurul's Graduation Shoot
(11, 2,  '2026-06-20', 'Rejected'),        -- Booking 6: Priya Devi -> Ahmad's Wedding Premium (rejected)
(8,  9,  '2026-04-15', 'Completed');       -- Booking 7: Farah Hanim -> Siti's Engagement Session

-- ============================================================
-- 6. PAYMENTS (Linked to bookings with 'Deposit Paid' status)
-- ============================================================
-- Booking 3 (Deposit Paid) must have a payment entry.
-- Bookings 4, 5, 7 (Completed) also have full payments.
-- ============================================================

INSERT INTO PAYMENTS (booking_id, amount, payment_type, payment_method, payment_date) VALUES
-- Deposit for Booking 3 (Lim Jia Yi -> Street Photography RM1200, deposit = RM600)
(3, 600.00,  'Deposit',       'Online Banking',  '2026-06-02'),

-- Full payment for Booking 4 (Muhammad Haziq -> Nikah Ceremony RM3500)
(4, 1750.00, 'Deposit',       'Online Banking',  '2026-04-25'),
(4, 1750.00, 'Final Payment', 'Online Banking',  '2026-05-10'),

-- Full payment for Booking 5 (Tan Wei Ming -> Graduation Shoot RM600)
(5, 300.00,  'Deposit',       'Credit Card',     '2026-04-10'),
(5, 300.00,  'Final Payment', 'Credit Card',     '2026-04-20'),

-- Full payment for Booking 7 (Farah Hanim -> Engagement Session RM1200)
(7, 600.00,  'Deposit',       'E-Wallet',        '2026-04-01'),
(7, 600.00,  'Final Payment', 'E-Wallet',        '2026-04-15');

-- ============================================================
-- 7. REVIEWS (Linked to bookings with 'Completed' status)
-- ============================================================

INSERT INTO REVIEWS (booking_id, rating, comment) VALUES
-- Review for Booking 4 (Muhammad Haziq -> Siti's Nikah Ceremony)
(4, 5, 'Absolutely stunning work! Siti captured every precious moment of our nikah. The cinematic video was breathtaking. Highly recommended!'),

-- Review for Booking 5 (Tan Wei Ming -> Nurul's Graduation Shoot)
(5, 4, 'Great photos and very professional. Nurul made me feel comfortable throughout the session. Minor delay in delivery but overall excellent.'),

-- Review for Booking 7 (Farah Hanim -> Siti's Engagement Session)
(7, 5, 'Perfect engagement photos! Siti has an amazing eye for detail. The outdoor shots at Taman Botani were absolutely gorgeous.');

-- ============================================================
-- 8. REPORTS (Customer reporting a booking issue)
-- ============================================================
-- Customer Farah Hanim (user_id = 8) reports an issue with
-- Booking 7 (Engagement Session) — admin_action is 'Pending'
-- ============================================================

INSERT INTO REPORTS (user_id, booking_id, description, admin_action) VALUES
(8, 7, 'The photographer delivered the edited photos 2 weeks later than the promised date. I would like a partial refund for the inconvenience.', 'Pending');

-- ============================================================
-- VERIFICATION QUERIES (Run these to confirm data integrity)
-- ============================================================

-- Check user counts by role
SELECT role, account_status, COUNT(*) AS total 
FROM USERS 
GROUP BY role, account_status;

-- Check packages per photographer
SELECT u.name AS photographer, COUNT(p.package_id) AS total_packages 
FROM PACKAGES p 
JOIN USERS u ON p.user_id = u.user_id 
GROUP BY u.name;

-- Check booking statuses
SELECT job_status, COUNT(*) AS total 
FROM BOOKINGS 
GROUP BY job_status;

-- Verify Deposit Paid booking has payment
SELECT b.booking_id, b.job_status, pay.amount, pay.payment_type 
FROM BOOKINGS b 
JOIN PAYMENTS pay ON b.booking_id = pay.booking_id 
WHERE b.job_status = 'Deposit Paid';

-- Verify Completed bookings have reviews
SELECT b.booking_id, b.job_status, r.rating, r.comment 
FROM BOOKINGS b 
JOIN REVIEWS r ON b.booking_id = r.booking_id 
WHERE b.job_status = 'Completed';

-- Verify report exists with pending admin action
SELECT rp.report_id, u.name AS reporter, rp.booking_id, rp.description, rp.admin_action 
FROM REPORTS rp 
JOIN USERS u ON rp.user_id = u.user_id;

-- ============================================================
-- END OF DUMMY DATA SCRIPT
-- ============================================================
