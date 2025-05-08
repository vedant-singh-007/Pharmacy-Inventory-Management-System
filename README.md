# Pharmacy Inventory Management System

## 📦 Project Overview

This **Pharmacy Inventory Management System** is a comprehensive C-based application designed to manage a pharmacy's medications and supplier data using **B-Trees** for efficient searching and organization. The system includes advanced features such as:

- B-Tree-based storage for medications and suppliers
- Expiry date-based medication sorting
- Detection of expired medicines
- Sales tracking and updates
- Reorder alerts for low stock
- Supplier analysis (e.g., "all-rounder" suppliers)
- Integrated file handling for data persistence

---

## 🛠️ Features

### ✅ Inventory Management
- Add, update, and delete medications
- Track medication batches uniquely by `medID + batchNo`
- Store and update quantities from multiple suppliers
- Maintain reorder levels for each medication

### 🌲 B-Tree Integration
- Efficient storage and retrieval using B-Trees
- B-Trees implemented for both medications and suppliers
- Handles insertion, deletion, and traversal operations

### 📋 Sorting & Expiry Handling
- Sort medications by **expiry date**
- Identify and list **expired medicines**
- Prevent sale of expired items

### 📊 Sales Tracking
- Update sales records with each transaction
- Maintain cumulative sales per medication
- Generate basic sales summaries

### 🚨 Stock Alerts
- Alerts generated when stock is below reorder level
- Can trigger reordering suggestions

### 👥 Supplier Management
- Maintain supplier details (ID, name, contact info)
- Track quantity supplied per medication
- Identify “**all-rounder**” suppliers — those supplying multiple batches/medications

### 💾 File Handling
- Load and save medication and supplier data to files
- Persistent storage across program runs
- Separate files for inventory and supplier databases

---

## 📁 File Structure

