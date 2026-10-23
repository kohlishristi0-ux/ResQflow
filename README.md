# Resource Planning and Emergency Coordination System

## 📌 Project Overview

The Resource Planning and Emergency Coordination System is a C++-based application designed to manage emergency situations, maintain resource inventories, prioritize emergencies based on severity, and allocate available resources efficiently.

The project demonstrates the practical implementation of Object-Oriented Programming (OOP) and Data Structures to support resource management and emergency coordination.

## 🎯 Objectives

- Maintain an inventory of emergency resources.
- Register and manage emergencies based on severity.
- Prioritize critical emergencies for processing.
- Allocate resources according to availability.
- Track allocated resources and outstanding requirements.
- Demonstrate modular programming using C++ classes and data structures.

## 🛠️ Technologies Used

- **Programming Language:** C++
- **Concepts:** Object-Oriented Programming (OOP)
- **Data Structures:** Singly Linked List and Priority Queue
- **Compiler:** GCC / MinGW
- **Development Environment:** Visual Studio Code

## 🧩 Project Modules

### 1. Resource Management
Manages the inventory of resources such as ambulances, medical supplies, food, and water.

Features:
- Add new resources.
- Display available resources.
- Search for resources using their IDs.
- Update resource quantities.

**Data Structure:** Singly Linked List

### 2. Emergency Management
Manages emergency registrations and processes them according to their severity.

Features:
- Register new emergencies.
- Assign unique emergency IDs during the current program execution.
- Record emergency type, location, description, and severity.
- Display pending emergencies.
- Process the highest-priority emergency first.

**Data Structure:** Array-Based Priority Queue

Severity levels:
- Critical
- High
- Medium
- Low

Emergencies with equal severity are processed in their registration order.

### 3. Resource Allocation
Allocates available resources to emergency requirements.

Features:
- Accept emergency and resource IDs.
- Record the requested quantity.
- Allocate resources according to available stock.
- Update resource quantities after allocation.
- Record outstanding requirements when resources are insufficient.
- Display allocation records.

### 4. Main Menu and Integration
Connects the individual modules through a menu-driven interface.

Features:
- Navigate between resource management and emergency management.
- Perform resource allocation.
- View allocation records.
- Exit the application.

## 🧠 Object-Oriented Programming Concepts

The project applies the following OOP concepts where implemented:

- **Classes and Objects:** Represent resources, emergencies, and allocation records.
- **Encapsulation:** Organizes data members and member functions within classes.
- **Abstraction:** Provides public methods to perform operations without exposing all internal implementation details.
- **Constructors and Destructors:** Initialize objects and manage dynamically allocated memory where implemented.
- **Modular Design:** Separates functionality into multiple header and implementation files.

## 📚 Data Structures Used

| Data Structure | Application | Purpose |
|---|---|---|
| Singly Linked List | Resource Management | Stores and manages resource records dynamically |
| Array-Based Priority Queue | Emergency Management | Processes emergencies according to severity |
| Linked List | Resource Allocation | Maintains allocation records, if implemented in the allocation manager |

## ⚙️ Installation and Execution

### Prerequisites

- A C++ compiler such as GCC / MinGW.
- Visual Studio Code or another C++ development environment.
