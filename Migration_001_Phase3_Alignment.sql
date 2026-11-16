USE resqflow;

DROP PROCEDURE IF EXISTS resqflow_add_column;
DELIMITER //
CREATE PROCEDURE resqflow_add_column(IN t VARCHAR(64), IN c VARCHAR(64), IN ddl VARCHAR(400))
BEGIN
  IF NOT EXISTS (SELECT 1 FROM information_schema.columns
                 WHERE table_schema = DATABASE() AND table_name = t AND column_name = c) THEN
    SET @s = CONCAT('ALTER TABLE ', t, ' ADD COLUMN ', c, ' ', ddl);
    PREPARE st FROM @s; EXECUTE st; DEALLOCATE PREPARE st;
  END IF;
END//
DELIMITER ;

CALL resqflow_add_column('responders', 'city', 'VARCHAR(100) NULL');
CALL resqflow_add_column('responders', 'area', 'VARCHAR(100) NULL');
CALL resqflow_add_column('responders', 'base_address', 'VARCHAR(255) NULL');
CALL resqflow_add_column('responders', 'user_id', 'INT NULL');
ALTER TABLE responders MODIFY latitude DECIMAL(10,7) NULL, MODIFY longitude DECIMAL(10,7) NULL;

CALL resqflow_add_column('responder_registration_requests', 'city', "VARCHAR(100) NOT NULL DEFAULT ''");
CALL resqflow_add_column('responder_registration_requests', 'area', "VARCHAR(100) NOT NULL DEFAULT ''");
CALL resqflow_add_column('responder_registration_requests', 'base_address', "VARCHAR(255) NOT NULL DEFAULT ''");
ALTER TABLE responder_registration_requests
  MODIFY latitude DECIMAL(10,7) NULL, MODIFY longitude DECIMAL(10,7) NULL;

ALTER TABLE warehouses
  MODIFY latitude DECIMAL(10,7) NOT NULL DEFAULT 0,
  MODIFY longitude DECIMAL(10,7) NOT NULL DEFAULT 0;

DROP PROCEDURE resqflow_add_column;
