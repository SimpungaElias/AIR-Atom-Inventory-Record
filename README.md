AIR — Atom Inventory Record

**AIR** is a secure, cross-platform desktop application for **Nuclear Material Accountancy and Control (NMAC)**. Built with C++ and the Qt Framework, it is designed to help nuclear facilities maintain accurate, auditable, and IAEA-compliant records of all nuclear material under their custody — while also serving as an educational platform for safeguards training.

> 🎓 **Open Source — Educational Tool** | IAEA Safeguards Compliant | INFCIRC/153 & Subsidiary Arrangements Code 10

---

## ✨ Features

### 🔐 Security Architecture
- **Zero Trust Design** — Identity re-verification is required for every sensitive operation, not just at login.
- **Tamper-Evident Audit Log** — Every transaction is stored with a SHA-256 cryptographic hash. Any external modification to the database is immediately detected and visually flagged in red on the Home Dashboard.
- **Role-Based Access Control (RBAC)** — Three user roles: Administrator, Read-Write, and Read-Only, each with enforced permissions throughout the application.
- **Air-Gapped Operation** — No network connection required. All data stays on the local machine.
- **Login Protection** — CAPTCHA challenge after 3 failed attempts; 5-minute lockout after 5 failures.

### 📋 Operations Modules
| Module | Description |
|---|---|
| **ICR** — Inventory Change Report | Record all nuclear material receipts, shipments, and transfers |
| **LII** — List of Inventory Items | Physical Inventory Listing (PIL) for all items in a Material Balance Area |
| **NLI** — Nuclear Loss Items | Record nuclear losses (fission, discard, accidental loss) |
| **MBR** — Material Balance Report | Periodic IAEA submission summarizing material balance |
| **General Ledger** | Master chronological accountancy record with running balance |

### 🎓 Safeguards Training Simulator
A fully isolated training environment that injects realistic safeguards scenarios into a sandboxed database — your operational data is never affected.

| Level | Module | Scenario |
|---|---|---|
| Beginner | 1.1 | The Baseline Receipt |
| Beginner | 1.2 | The Reactor Cycle |
| Intermediate | 2.1 | Shipper/Receiver Differences (SRD) |
| Intermediate | 2.2 | Physical Inventory Taking (PIT) |
| Intermediate | 2.3 | MUF Evaluation |
| Advanced | 3.1 | Protracted Diversion Detection |
| Advanced | 3.2 | The Substituted Dummy Assembly |

### 📄 PDF Reporting
Generate IAEA-compliant reports directly from the application — ICR, LII, NLI, MBR, and General Ledger — ready for submission to national or international regulatory bodies.

---

## 🚀 Getting Started

### Installation

**🍎 macOS (.dmg)**
Download `AIR-Simulator-Mac.dmg` and double-click to open it. Drag the AIR icon into your Applications folder. On first launch, macOS may show a security warning because the app is not from the App Store — this is expected for open-source tools. To open it, right-click the AIR app and select **Open**, then click **Open Again** in the dialog. If the warning persists, go to **System Preferences → Security & Privacy** and click **Open Anyway**. After this one-time step, AIR opens normally with a double-click.

**🪟 Windows (.zip)**
Download `AIR-Simulator-Windows.zip`. Before running, you must extract it — right-click the file and select **Extract All**, then open the extracted folder. Double-click `AIR.exe` to launch. If Windows Defender SmartScreen shows a blue warning, click **More info** then **Run anyway**. This prompt appears because the app is not yet code-signed, not because it is harmful.

**🐧 Linux (.AppImage)**
Download `AIR-Simulator-Linux.AppImage`. Before running, mark it as executable by right-clicking → **Properties → Permissions** → check **Allow executing file as program**. Alternatively, open a terminal in the download folder and run:
```bash
chmod +x AIR-Simulator-Linux.AppImage
./AIR-Simulator-Linux.AppImage
```

---

### First Launch

1. On first launch, AIR automatically creates a fresh database. Log in with the default credentials:

   | Field | Value |
   |---|---|
   | Username | `admin` |
   | Password | `Admin@123` |

2. **Change the default password immediately.** Go to **Administration → User Management**, select the admin account, and set a strong password.

3. **Create operator accounts.** Navigate to **Administration → User Management** and create profiles for each user, assigning the appropriate role:
   - **Administrator** — Full access including user management and backup/restore
   - **Read-Write** — Can enter data and generate reports
   - **Read-Only** — Can view data and reports only

4. Assign each user their Material Balance Area (MBA) during account creation to control data access scope.

---

## 📚 Documentation

The full **AIR User Manual** is bundled inside the application under **Resources & Documentation** on the intro screen, and covers all modules, security architecture, IAEA compliance, training scenarios, and troubleshooting.

Key reference documents also included:
- **INFCIRC/153** — The legal basis for IAEA comprehensive safeguards agreements
- **Subsidiary Arrangements (Code 10)** — Detailed safeguards implementation procedures

---

## 🏗️ Built With

- **C++17** with the **Qt 6 Framework** (Widgets, SQL, PrintSupport)
- **SQLite** — Local encrypted database, no server required
- **SHA-256** cryptographic hashing via Qt's `QCryptographicHash`
- **GitHub Actions** — Automated CI/CD builds for Linux (AppImage), Windows (.exe), and macOS (.dmg)

---

## 🌍 Compliance

AIR is designed in alignment with:
- **IAEA INFCIRC/153** — Comprehensive Safeguards Agreements
- **Subsidiary Arrangements, General Part (Code 10)**
- **IAEA Safeguards Glossary** — Material types, change codes, and reporting formats

---

## ⚠️ Disclaimer

AIR is an **educational and research tool**. It is not a certified IAEA system and should not be used as the sole accountancy system for a licensed nuclear facility without appropriate validation and regulatory approval.

---
