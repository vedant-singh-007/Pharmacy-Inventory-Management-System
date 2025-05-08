# Pharmacy Inventory Management System

##  Project Overview

This **Pharmacy Inventory Management System** is a C-based application designed to manage a pharmacy's medications and supplier data using **B-Trees** for efficient storage and lookup. It supports advanced functionality like sorting by expiry, tracking sales, identifying expired medicines, sending stock alerts, and linking supplier and medication records. The system also uses **file handling** for persistent data storage.

---

##  Features

###  Inventory Management
- Add, update, and delete medication records.
- Unique medications identified by `medID + batchNo`.
- Track supplier-specific quantities for each batch.
- Automatically generates stock alerts when below reorder level.

###  B-Tree Integration
- Four separate B-Trees store the same medication records but sorted differently:
  - By **medID**
  - By **medName**
  - By **expiry date**
  - By **supplier ID**
- All four trees reference the same dynamically allocated medication record to avoid data duplication.
- Supplier and medication records are **linked** — enabling quick tracing from a medicine to its supplier(s) and vice versa.

###  Sorting & Expiry Handling
- Sort medications by **expiry date** using B-Tree traversal.
- Identify and list all **expired medicines**.
- Prevent sales of expired inventory.

###  Sales Tracking
- Records total units sold per medication.
- Updates sales upon each transaction.
- Enables review of sales performance.

###  Stock Alerts
- Notifies when medication stock falls below its defined reorder level.
- Helps avoid stock-outs and ensures timely restocking.

###  Supplier Management
- Stores supplier ID, name, contact info.
- Tracks medications supplied by each supplier.
- Detects **"all-rounder" suppliers** who provide multiple medicines across batches.

###  File Handling
- Loads data from files at startup.
- Saves updated records back to files on modification or exit.
- Ensures **persistent storage** across sessions.

---

##  Technical Highlights

- **Memory efficiency**: A single medication record is created and pointed to by all four B-Trees.
- **Two-way linkage**: Medications point to their suppliers, and suppliers maintain lists of medications they supply.
- **Modular code**: Organized in components for medications, suppliers, B-Tree logic, file handling, and utility functions.

---

