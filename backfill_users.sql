UPDATE USERS SET user_code = CONCAT('USR-', LPAD(user_id, 3, '0')) WHERE user_code IS NULL;
