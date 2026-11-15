#!/usr/bin/env python3
"""Translate database/ResQflow_Database_Schema.sql (MySQL) into SQLite DDL for the OFFLINE test shim."""
import re, sys
s = open(sys.argv[1]).read()
s = re.sub(r'--[^\n]*', '', s)
s = re.sub(r'CREATE DATABASE.*?;', '', s, flags=re.S)
s = re.sub(r'\bUSE \w+;', '', s)
s = re.sub(r'SHOW TABLES;', '', s)
s = s.replace('ENGINE=InnoDB', '')
s = re.sub(r'ON UPDATE CURRENT_TIMESTAMP', '', s)
s = re.sub(r'\b(?:BIGINT|INT) AUTO_INCREMENT PRIMARY KEY', 'INTEGER PRIMARY KEY AUTOINCREMENT', s)
s = re.sub(r'(\w+) INT NOT NULL AUTO_INCREMENT PRIMARY KEY', r'\1 INTEGER PRIMARY KEY AUTOINCREMENT', s)
s = re.sub(r'(\w+) ENUM\(([^)]*)\)', lambda m: f"{m.group(1)} TEXT CHECK({m.group(1)} IN ({m.group(2)}))", s)
s = re.sub(r'\n\s*INDEX \w+ \([^)]*\),?', '', s)
s = re.sub(r',\s*\)', '\n)', s)
s = s.replace('TINYINT', 'INTEGER').replace('BIGINT', 'INTEGER')
open(sys.argv[2], 'w').write(s)
