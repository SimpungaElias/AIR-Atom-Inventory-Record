# AIR: Atom Inventory Record

AIR is a cross-platform desktop application designed for Nuclear Material Accounting and Control (NMAC). It features operational accounting tools (MBR, ICR, LII, General Ledger) alongside an embedded **Safeguards Training Simulator** to teach anomaly detection and insider threat mitigation.

## Features
* **Zero Trust Architecture:** Identity verification required for sensitive operations.
* **Tamper-Evident Logs:** Cryptographic SHA-256 hashing to detect database tampering.
* **Simulator Engine:** Safely injects complex puzzles (SRD, MUF, Protracted Diversion) into a sandboxed environment without altering operational data.
* **PDF Reporting:** Generate IAEA-compliant accounting reports.

## Getting Started
1. Download the latest release for your OS (Windows, macOS, Linux) from the **Releases** tab.
2. Launch the application.
3. On the first launch, the system will generate a fresh database. Log in using the default credentials:
   * **Username:** admin
   * **Password:** Admin@123
4. **Important:** Navigate to the Administration tab immediately to create profiles for your operators based on priveleges.
