CREATE DATABASE IF NOT EXISTS resqflow
  CHARACTER SET utf8mb4
  COLLATE utf8mb4_0900_ai_ci;
USE resqflow;

CREATE TABLE IF NOT EXISTS users (
    user_id INT AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(60) NOT NULL UNIQUE,
    password_hash VARCHAR(255) NOT NULL,
    full_name VARCHAR(120) NOT NULL,
    email VARCHAR(150) UNIQUE,
    contact_number VARCHAR(25),
    role ENUM('ADMIN', 'COORDINATOR', 'RESPONDER') NOT NULL DEFAULT 'COORDINATOR',
    is_active BOOLEAN NOT NULL DEFAULT TRUE,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS resources (
    resource_id INT PRIMARY KEY,
    resource_name VARCHAR(120) NOT NULL,
    category VARCHAR(80) NOT NULL,
    description TEXT,
    unit VARCHAR(30) NOT NULL,
    minimum_stock_level INT NOT NULL DEFAULT 0,
    is_active BOOLEAN NOT NULL DEFAULT TRUE,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    CONSTRAINT chk_resource_min_stock CHECK (minimum_stock_level >= 0)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS warehouses (
    warehouse_id INT PRIMARY KEY,
    warehouse_name VARCHAR(120) NOT NULL,
    address VARCHAR(255) NOT NULL,
    latitude DECIMAL(10,7) NOT NULL DEFAULT 0,
    longitude DECIMAL(10,7) NOT NULL DEFAULT 0,
    contact_number VARCHAR(25),
    is_active BOOLEAN NOT NULL DEFAULT TRUE,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    CONSTRAINT chk_warehouse_lat CHECK (latitude BETWEEN -90 AND 90),
    CONSTRAINT chk_warehouse_lon CHECK (longitude BETWEEN -180 AND 180)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS inventory (
    inventory_id INT AUTO_INCREMENT PRIMARY KEY,
    resource_id INT NOT NULL,
    warehouse_id INT NOT NULL,
    available_quantity INT NOT NULL DEFAULT 0,
    reserved_quantity INT NOT NULL DEFAULT 0,
    minimum_stock_level INT NOT NULL DEFAULT 0,
    updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    CONSTRAINT uq_inventory_resource_warehouse UNIQUE (resource_id, warehouse_id),
    CONSTRAINT chk_inventory_available CHECK (available_quantity >= 0),
    CONSTRAINT chk_inventory_reserved CHECK (reserved_quantity >= 0),
    CONSTRAINT chk_inventory_minimum CHECK (minimum_stock_level >= 0),
    CONSTRAINT fk_inventory_resource FOREIGN KEY (resource_id) REFERENCES resources(resource_id),
    CONSTRAINT fk_inventory_warehouse FOREIGN KEY (warehouse_id) REFERENCES warehouses(warehouse_id)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS emergencies (
    emergency_id INT PRIMARY KEY,
    emergency_type VARCHAR(100) NOT NULL,
    location_name VARCHAR(255) NOT NULL,
    description TEXT,
    severity TINYINT NOT NULL,
    affected_people INT NOT NULL DEFAULT 1,
    latitude DECIMAL(10,7) NOT NULL,
    longitude DECIMAL(10,7) NOT NULL,
    status ENUM('REPORTED', 'VERIFIED', 'IN_PROGRESS', 'RESOLVED', 'CANCELLED') NOT NULL DEFAULT 'REPORTED',
    registration_order BIGINT NOT NULL,
    reported_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    created_by INT NULL,
    CONSTRAINT chk_emergency_severity CHECK (severity BETWEEN 1 AND 5),
    CONSTRAINT chk_emergency_affected CHECK (affected_people >= 1),
    CONSTRAINT chk_emergency_lat CHECK (latitude BETWEEN -90 AND 90),
    CONSTRAINT chk_emergency_lon CHECK (longitude BETWEEN -180 AND 180),
    CONSTRAINT fk_emergency_creator FOREIGN KEY (created_by) REFERENCES users(user_id),
    INDEX idx_emergency_status_severity (status, severity),
    INDEX idx_emergency_reported_at (reported_at)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS emergency_status_history (
    status_history_id BIGINT AUTO_INCREMENT PRIMARY KEY,
    emergency_id INT NOT NULL,
    old_status ENUM('REPORTED', 'VERIFIED', 'IN_PROGRESS', 'RESOLVED', 'CANCELLED') NULL,
    new_status ENUM('REPORTED', 'VERIFIED', 'IN_PROGRESS', 'RESOLVED', 'CANCELLED') NOT NULL,
    changed_by INT NULL,
    notes VARCHAR(500),
    changed_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT fk_status_history_emergency FOREIGN KEY (emergency_id) REFERENCES emergencies(emergency_id),
    CONSTRAINT fk_status_history_user FOREIGN KEY (changed_by) REFERENCES users(user_id),
    INDEX idx_status_history_emergency_time (emergency_id, changed_at)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS emergency_requirements (
    requirement_id BIGINT AUTO_INCREMENT PRIMARY KEY,
    emergency_id INT NOT NULL,
    resource_id INT NOT NULL,
    required_quantity INT NOT NULL,
    fulfilled_quantity INT NOT NULL DEFAULT 0,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    CONSTRAINT uq_emergency_requirement UNIQUE (emergency_id, resource_id),
    CONSTRAINT chk_requirement_quantity CHECK (required_quantity > 0),
    CONSTRAINT chk_requirement_fulfilled CHECK (fulfilled_quantity >= 0 AND fulfilled_quantity <= required_quantity),
    CONSTRAINT fk_requirement_emergency FOREIGN KEY (emergency_id) REFERENCES emergencies(emergency_id),
    CONSTRAINT fk_requirement_resource FOREIGN KEY (resource_id) REFERENCES resources(resource_id)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS allocations (
    allocation_id INT PRIMARY KEY,
    emergency_id INT NOT NULL,
    resource_id INT NOT NULL,
    warehouse_id INT NOT NULL,
    requested_quantity INT NOT NULL,
    allocated_quantity INT NOT NULL DEFAULT 0,
    dispatched_quantity INT NOT NULL DEFAULT 0,
    returned_quantity INT NOT NULL DEFAULT 0,
    status ENUM('PENDING', 'PARTIALLY_ALLOCATED', 'ALLOCATED', 'DISPATCHED', 'COMPLETED', 'CANCELLED') NOT NULL DEFAULT 'PENDING',
    created_by INT NULL,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    CONSTRAINT chk_allocation_requested CHECK (requested_quantity > 0),
    CONSTRAINT chk_allocation_allocated CHECK (allocated_quantity >= 0 AND allocated_quantity <= requested_quantity),
    CONSTRAINT chk_allocation_dispatched CHECK (dispatched_quantity >= 0 AND dispatched_quantity <= allocated_quantity),
    CONSTRAINT chk_allocation_returned CHECK (returned_quantity >= 0 AND returned_quantity <= dispatched_quantity),
    CONSTRAINT fk_allocation_emergency FOREIGN KEY (emergency_id) REFERENCES emergencies(emergency_id),
    CONSTRAINT fk_allocation_resource FOREIGN KEY (resource_id) REFERENCES resources(resource_id),
    CONSTRAINT fk_allocation_warehouse FOREIGN KEY (warehouse_id) REFERENCES warehouses(warehouse_id),
    CONSTRAINT fk_allocation_user FOREIGN KEY (created_by) REFERENCES users(user_id),
    INDEX idx_allocation_emergency (emergency_id),
    INDEX idx_allocation_status (status)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS responders (
    responder_id INT PRIMARY KEY,
    responder_name VARCHAR(120) NOT NULL,
    role VARCHAR(80) NOT NULL,
    contact_number VARCHAR(25) NOT NULL,
    specialization VARCHAR(120),
    city VARCHAR(100) NULL,
    area VARCHAR(100) NULL,
    base_address VARCHAR(255) NULL,
    latitude DECIMAL(10,7) NULL,
    longitude DECIMAL(10,7) NULL,
    status ENUM('AVAILABLE', 'ASSIGNED', 'OFF_DUTY') NOT NULL DEFAULT 'AVAILABLE',
    user_id INT NULL UNIQUE,
    is_active BOOLEAN NOT NULL DEFAULT TRUE,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    CONSTRAINT chk_responder_lat CHECK (latitude IS NULL OR latitude BETWEEN -90 AND 90),
    CONSTRAINT chk_responder_lon CHECK (longitude IS NULL OR longitude BETWEEN -180 AND 180),
    CONSTRAINT fk_responder_user FOREIGN KEY (user_id) REFERENCES users(user_id)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS emergency_responder_assignments (
    assignment_id BIGINT AUTO_INCREMENT PRIMARY KEY,
    responder_id INT NOT NULL,
    emergency_id INT NOT NULL,
    assigned_by INT NULL,
    assigned_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    released_at DATETIME NULL,
    assignment_status ENUM('ACTIVE', 'RELEASED', 'CANCELLED') NOT NULL DEFAULT 'ACTIVE',
    notes VARCHAR(500),
    CONSTRAINT fk_assignment_responder FOREIGN KEY (responder_id) REFERENCES responders(responder_id),
    CONSTRAINT fk_assignment_emergency FOREIGN KEY (emergency_id) REFERENCES emergencies(emergency_id),
    CONSTRAINT fk_assignment_user FOREIGN KEY (assigned_by) REFERENCES users(user_id),
    INDEX idx_assignment_responder_status (responder_id, assignment_status),
    INDEX idx_assignment_emergency (emergency_id)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS warehouse_transfers (
    transfer_id BIGINT AUTO_INCREMENT PRIMARY KEY,
    source_warehouse_id INT NOT NULL,
    destination_warehouse_id INT NOT NULL,
    status ENUM('PENDING', 'COMPLETED', 'CANCELLED') NOT NULL DEFAULT 'PENDING',
    requested_by INT NULL,
    notes VARCHAR(500),
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    completed_at DATETIME NULL,
    CONSTRAINT chk_transfer_different_warehouses CHECK (source_warehouse_id <> destination_warehouse_id),
    CONSTRAINT fk_transfer_source FOREIGN KEY (source_warehouse_id) REFERENCES warehouses(warehouse_id),
    CONSTRAINT fk_transfer_destination FOREIGN KEY (destination_warehouse_id) REFERENCES warehouses(warehouse_id),
    CONSTRAINT fk_transfer_user FOREIGN KEY (requested_by) REFERENCES users(user_id),
    INDEX idx_transfer_status_time (status, created_at)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS warehouse_transfer_details (
    transfer_detail_id BIGINT AUTO_INCREMENT PRIMARY KEY,
    transfer_id BIGINT NOT NULL,
    resource_id INT NOT NULL,
    quantity INT NOT NULL,
    CONSTRAINT uq_transfer_resource UNIQUE (transfer_id, resource_id),
    CONSTRAINT chk_transfer_quantity CHECK (quantity > 0),
    CONSTRAINT fk_transfer_detail_transfer FOREIGN KEY (transfer_id) REFERENCES warehouse_transfers(transfer_id),
    CONSTRAINT fk_transfer_detail_resource FOREIGN KEY (resource_id) REFERENCES resources(resource_id)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS resource_transactions (
    transaction_id BIGINT AUTO_INCREMENT PRIMARY KEY,
    resource_id INT NOT NULL,
    warehouse_id INT NOT NULL,
    transaction_type ENUM('STOCK_IN', 'RESERVE', 'RELEASE', 'DISPATCH', 'RETURN', 'TRANSFER_OUT', 'TRANSFER_IN', 'ADJUSTMENT') NOT NULL,
    quantity INT NOT NULL,
    reference_type VARCHAR(40),
    reference_id BIGINT,
    performed_by INT NULL,
    notes VARCHAR(500),
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT chk_transaction_quantity CHECK (quantity > 0),
    CONSTRAINT fk_transaction_resource FOREIGN KEY (resource_id) REFERENCES resources(resource_id),
    CONSTRAINT fk_transaction_warehouse FOREIGN KEY (warehouse_id) REFERENCES warehouses(warehouse_id),
    CONSTRAINT fk_transaction_user FOREIGN KEY (performed_by) REFERENCES users(user_id),
    INDEX idx_transaction_resource_time (resource_id, created_at),
    INDEX idx_transaction_warehouse_time (warehouse_id, created_at),
    INDEX idx_transaction_reference (reference_type, reference_id)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS activity_history (
    activity_id BIGINT AUTO_INCREMENT PRIMARY KEY,
    activity_type VARCHAR(50) NOT NULL,
    description VARCHAR(1000) NOT NULL,
    emergency_id INT NULL,
    allocation_id INT NULL,
    transfer_id BIGINT NULL,
    responder_id INT NULL,
    warehouse_id INT NULL,
    destination_warehouse_id INT NULL,
    resource_id INT NULL,
    quantity INT NOT NULL DEFAULT 0,
    performed_by VARCHAR(120) NOT NULL DEFAULT 'SYSTEM',
    actor_user_id INT NULL,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,

    CONSTRAINT fk_activity_emergency
        FOREIGN KEY (emergency_id) REFERENCES emergencies(emergency_id),
    CONSTRAINT fk_activity_allocation
        FOREIGN KEY (allocation_id) REFERENCES allocations(allocation_id),
    CONSTRAINT fk_activity_transfer
        FOREIGN KEY (transfer_id) REFERENCES warehouse_transfers(transfer_id),
    CONSTRAINT fk_activity_responder
        FOREIGN KEY (responder_id) REFERENCES responders(responder_id),
    CONSTRAINT fk_activity_warehouse
        FOREIGN KEY (warehouse_id) REFERENCES warehouses(warehouse_id),
    CONSTRAINT fk_activity_destination_warehouse
        FOREIGN KEY (destination_warehouse_id) REFERENCES warehouses(warehouse_id),
    CONSTRAINT fk_activity_resource
        FOREIGN KEY (resource_id) REFERENCES resources(resource_id),
    CONSTRAINT fk_activity_actor
        FOREIGN KEY (actor_user_id) REFERENCES users(user_id),

    INDEX idx_activity_type_time (activity_type, created_at),
    INDEX idx_activity_emergency_time (emergency_id, created_at)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS locations (
    location_id INT AUTO_INCREMENT PRIMARY KEY,
    location_name VARCHAR(150) NOT NULL,
    latitude DECIMAL(10,7) NOT NULL,
    longitude DECIMAL(10,7) NOT NULL,
    is_active BOOLEAN NOT NULL DEFAULT TRUE,
    CONSTRAINT chk_location_lat CHECK (latitude BETWEEN -90 AND 90),
    CONSTRAINT chk_location_lon CHECK (longitude BETWEEN -180 AND 180)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS route_edges (
    route_edge_id BIGINT AUTO_INCREMENT PRIMARY KEY,
    source_location_id INT NOT NULL,
    destination_location_id INT NOT NULL,
    distance_km DECIMAL(12,3) NOT NULL,
    is_active BOOLEAN NOT NULL DEFAULT TRUE,
    CONSTRAINT uq_route_pair UNIQUE (source_location_id, destination_location_id),
    CONSTRAINT chk_route_different_locations CHECK (source_location_id <> destination_location_id),
    CONSTRAINT chk_route_distance CHECK (distance_km > 0),
    CONSTRAINT fk_route_source FOREIGN KEY (source_location_id) REFERENCES locations(location_id),
    CONSTRAINT fk_route_destination FOREIGN KEY (destination_location_id) REFERENCES locations(location_id)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS audit_logs (
    audit_id BIGINT AUTO_INCREMENT PRIMARY KEY,
    user_id INT NULL,
    action VARCHAR(100) NOT NULL,
    entity_type VARCHAR(60) NOT NULL,
    entity_id BIGINT,
    details JSON NULL,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT fk_audit_user FOREIGN KEY (user_id) REFERENCES users(user_id),
    INDEX idx_audit_entity (entity_type, entity_id),
    INDEX idx_audit_time (created_at)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS responder_registration_requests (
    registration_id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(60) NOT NULL,
    password_hash VARCHAR(255) NOT NULL,
    full_name VARCHAR(120) NOT NULL,
    email VARCHAR(150) NULL,
    contact_number VARCHAR(25) NOT NULL,
    responder_role VARCHAR(80) NOT NULL,
    specialization VARCHAR(120) NOT NULL,
    city VARCHAR(100) NOT NULL,
    area VARCHAR(100) NOT NULL,
    base_address VARCHAR(255) NOT NULL,
    status ENUM('PENDING','APPROVED','REJECTED') NOT NULL DEFAULT 'PENDING',
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    reviewed_at TIMESTAMP NULL DEFAULT NULL,
    INDEX idx_registration_status_created (status, created_at),
    INDEX idx_registration_username (username)
) ENGINE=InnoDB;
