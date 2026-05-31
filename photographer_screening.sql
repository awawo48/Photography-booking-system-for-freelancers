CREATE TABLE IF NOT EXISTS photographer_applications (
    application_id INT AUTO_INCREMENT PRIMARY KEY,
    user_id INT NOT NULL,
    portfolio_link VARCHAR(255) NOT NULL,
    camera_setup VARCHAR(255) NOT NULL,
    experience_years VARCHAR(50) NOT NULL,
    coverage_areas VARCHAR(255) NOT NULL,
    rejection_reason TEXT DEFAULT NULL,
    applied_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES USERS(user_id) ON DELETE CASCADE
);
