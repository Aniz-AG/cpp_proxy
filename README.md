# Auto Proxy Switcher (Windows · C++)

A lightweight Windows utility that **automatically configures proxy settings** for:

- ✅ Windows UI / Browser (WinINET)
- ✅ Git
- ✅ npm

based on the **currently connected network** (college vs non-college).

This tool eliminates the need to manually:
- toggle Windows proxy settings,
- remember git / npm proxy commands,
- reconfigure things every time you switch networks.

---

## ✨ Why this exists

On many college or enterprise networks:
- Internet access works **only through an authenticated proxy**
- Browsers work after login, but
- `git`, `npm`, and CLI tools **break unless manually configured**
- Switching back to hotspot/home Wi-Fi requires undoing everything again

This project solves that **once and for all**, automatically.

---

## 🧠 How it works (high level)

1. **Detects the current network**
   - Uses a stable identifier (gateway IP) from `ipconfig`
2. **Loads proxy credentials securely**
   - From **Windows Credential Manager**
   - No hardcoding, no plaintext files
3. **If on college network**
   - Enables Windows proxy (WinINET)
   - Sets git proxy
   - Sets npm proxy
4. **If NOT on college network**
   - Disables Windows proxy
   - Removes git proxy
   - Removes npm proxy
5. **Runs automatically** on network change using Task Scheduler

The program runs for **milliseconds** and exits — no background process.

---

## 🔐 Security model

- Proxy credentials are stored **once** in **Windows Credential Manager**
- Credentials are:
  - Encrypted by Windows
  - Never printed to logs
  - Never stored in source code
- The executable only **reads** credentials at runtime

This is the same mechanism used by enterprise Windows applications.

---

## 🧱 Components used

| Component | Purpose |
|---------|--------|
Windows Credential Manager | Secure credential storage |
WinINET (Registry + API) | Windows UI / browser proxy |
Git config | CLI git proxy |
npm config | CLI npm proxy |
Task Scheduler | Automatic execution on network change |

---

## ⚙️ One-time setup

### 1️⃣ Store proxy credentials (one time)

Open **Credential Manager**:

Control Panel → User Accounts → Credential Manager->→ Windows Credentials → Add a generic credential


Fill in:

- **Internet or network address**: `COLLEGE_PROXY`
- **User name**: your proxy username
- **Password**: your proxy password

Save.

> ⚠️ Do NOT hardcode credentials in code.

---

### 2️⃣ Configure network identifier

In the source code, set your **college gateway IP**:

```cpp
constexpr const char* COLLEGE_GATEWAY = "172.**.*.*";
```
You can find this via cmd prompt:
```cmd
ipconfig
```
This value uniquely identifies your college network.

3️⃣ Build the executable

Compile using MinGW / g++:

`g++ code.cpp -o autoproxy.exe -ladvapi32 -lwininet`


Move the executable to a fixed location, for example:

`C:\Tools\AutoProxy\autoproxy.exe`

🚀 Automatic execution (recommended)
Why Task Scheduler?

- Event-driven (no polling)

- Zero background CPU usage

- Native Windows mechanism

- Safe and reversible

Create scheduled task

Press Win + R → type:

taskschd.msc


Click Create Task (not “Basic Task”)

General tab
- Name: Auto Proxy Switcher

- ✅ Run only when user is logged on

- ❌ Do NOT run with highest privileges

Triggers tab → New

- Begin the task: On an event

- Log: Microsoft-Windows-NetworkProfile/Operational

- Source: NetworkProfile

- Event ID: 10000

- Actions tab → New

- Action: Start a program

- Program/script: `C:\Tools\AutoProxy\autoproxy.exe`

- Start in: `C:\Tools\AutoProxy`

Conditions tab

❌ Uncheck “Start only if on AC power”

Click OK.

No password prompt if running only when logged in.


## 🧪 Expected Behavior

### On college network
- Windows proxy → **ON**
- Git proxy → **ON**
- npm proxy → **ON**
- Browser opens proxy login page once *(expected)*

### On hotspot / home Wi-Fi
- Windows proxy → **OFF**
- Git proxy → **OFF**
- npm proxy → **OFF**
- Internet works normally

**No manual steps required.**

---

## 📉 Performance Impact

- ❌ No background service
- ❌ No polling
- ❌ No memory usage after execution
- ✅ Runs only on network change
- ⏱️ Execution time: **< 100 ms**

**This does not degrade system performance over time.**

---

## 🧹 Cleanup / Removal

To completely remove the tool:

1. Delete the scheduled task
2. Delete the executable
3. *(Optional)* Remove stored credentials: Credential Manager → Windows Credentials → COLLEGE_PROXY


The system returns to its original state.

---

## 📝 Notes & Limitations

- Windows-only *(by design)*
- WinINET proxy authentication must go through the browser UI *(cannot be automated)*
- Designed for **personal use on managed networks**




