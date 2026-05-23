USE photography_db;

CREATE TABLE IF NOT EXISTS SYSTEM_SETTINGS (
    setting_key VARCHAR(50) PRIMARY KEY,
    setting_value VARCHAR(100)
);

CREATE TABLE IF NOT EXISTS PROMO_CODES (
    code VARCHAR(20) PRIMARY KEY,
    discount_pct DECIMAL(5,2),
    valid_until DATE
);

CREATE TABLE IF NOT EXISTS NOTIFICATIONS (
    notif_id INT AUTO_INCREMENT PRIMARY KEY,
    message TEXT,
    target_role VARCHAR(20),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

INSERT IGNORE INTO SYSTEM_SETTINGS (setting_key, setting_value) VALUES ('admin_commission', '0.05');
