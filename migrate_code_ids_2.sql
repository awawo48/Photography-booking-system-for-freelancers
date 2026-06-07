ALTER TABLE PAYMENTS ADD COLUMN payment_code VARCHAR(30) UNIQUE AFTER payment_id;
ALTER TABLE PORTFOLIOS ADD COLUMN portfolio_code VARCHAR(30) UNIQUE AFTER portfolio_id;
ALTER TABLE REVIEWS ADD COLUMN review_code VARCHAR(30) UNIQUE AFTER review_id;

CREATE INDEX idx_payment_code ON PAYMENTS(payment_code);
CREATE INDEX idx_portfolio_code ON PORTFOLIOS(portfolio_code);
CREATE INDEX idx_review_code ON REVIEWS(review_code);
