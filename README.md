# OTApp - OpenThread Application Framework </br> STM32 Port

![Language](https://img.shields.io/badge/language-Embedded%20C-00599C.svg?style=flat&logo=c)
![Platform](https://img.shields.io/badge/platform-STM32-03234B.svg?style=flat&logo=stmicroelectronics&logoColor=white)
![Protocol](https://img.shields.io/badge/protocol-OpenThread%20%7C%20CoAP-4caf50.svg)
![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Status](https://img.shields.io/badge/Status-Active_Development-brightgreen.svg?style=flat&logo=github&logoColor=white)

<details><summary><b>🇵🇱 Wersja Polska</b></summary><br>

To repozytorium zawiera warstwę abstrakcji sprzętowej (HAL) oraz specyficzne optymalizacje platformowe dla frameworka **[OTApp](https://github.com/HareoPL/ot_app)**, dedykowane dla mikrokontrolerów serii **STM32WBA6**.

## 🛠 Specyfikacja Platformy
- **MCU:** STM32WBA6 (Cortex-M33).
- **Toolchain:** STM32CubeIDE / GNU Arm Embedded Toolchain.
- **RTOS:** FreeRTOS (zintegrowany poprzez CMSIS-OS2).
- **Stos bezprzewodowy:** Middleware STM32WPAN (OpenThread FTD/MTD).

---

## 🚀 Kluczowe Ulepszenia Platformy

W tym porcie zaimplementowano krytyczne poprawki stabilności i wydajności, których brakuje w standardowych przykładach SDK od producenta:

### 💾 Zoptymalizowana pamięć NVM z mechanizmem Wear-Levelingu
Standardowe implementacje implementacje nie zapisywała ustawień do pamieci trwałej. Ten port zapisuje ustawienia do pamieci FLASH oraz wprowadza inteligentny mechanizm zarządzania pamięcią nieulotną:
- **Mechanizm Slotów:** Strona Flash (8kB) została logicznie podzielona na bufory. Dane są dopisywane do kolejnych wolnych adresów zamiast nadpisywania tego samego miejsca.
- **Zwiększona Żywotność:** Dzięki rotacji miejsc zapisu, fizyczna strona Flash zużywa się znacznie wolniej, co teoretycznie zwiększa jej trwałość z bazowych 100k do nawet **400k cykli zapisu**.
- **Asynchroniczny Zapis:** Wykorzystano **FreeRTOS Timer** (mechanizm debounce) oraz dedykowany zadanie (Task) o niskim priorytecie. Timer budzi zadanie zapisu dopiero po określonym czasie bezczynności, co eliminuje zbędne operacje przy serii szybkich zmian ustawień.

### 🎲 Stabilny sprzętowy generator liczb losowych (HW RNG)
Rozwiązano problem zawieszania się generatora RNG przy intensywnym korzystaniu ze stosu OpenThread:
- **Problem:** Oryginalne sterowniki często wymuszały restart RNG po każdym użyciu, co przy dużej liczbie zapytań prowadziło do błędów zegara (Clock Error) i blokowania procesora w pętli `while`.
- **Rozwiązanie:** Zaimplementowano mechanizm opóźnionego wyłączania za pomocą FreeRTOS. Generator pozostaje aktywny przez krótki czas po ostatnim zapytaniu. Jeśli system poprosi o nową liczbę w tym oknie czasowym, generator jest natychmiast dostępny, co zapobiega błędom synchronizacji i oszczędza energię.

---

## 🔌 Konfiguracja Sprzętowa
Port domyślnie wspiera urządzenia typu `ot_device` w roli **Full Thread Device (FTD)**.

todo

## 🔌 Jak zacząć?
1. Sklonuj repozytorium wraz z submodułami:
```bash
   git clone --recursive [https://github.com/HareoPL/ot_app_stm.git](https://github.com/HareoPL/ot_app_stm.git)
```

2. Zaimportuj projekt do **STM32CubeIDE**.
3. Skompiluj i wgraj program na płytkę (np. NUCLEO-WBA65).

## 🔗 Framework i Zasoby

Więcej informacji o logice frameworka, API CoAP oraz mechanizmach parowania znajdziesz w głównym projekcie:

* **Główne Repozytorium:** 👉 **[github.com/HareoPL/ot_app](https://github.com/HareoPL/ot_app)**
* **Dokumentacja:** 👉 **[Hareo.pl/otapp](https://hareo.pl/otapp)**
* **inne platformy**:
</br> 👉 ESP32-C6: **[ github.com/HareoPL/ot_app_esp](https://github.com/HareoPL/ot_app_esp)**
</br> 👉 Control Panel (STM32H7 + ESP32-C6 + LCD): **[github.com/HareoPL/ot_app_cp](https://github.com/HareoPL/ot_app_cp)**

## 👨‍💻 Autor i Kontakt

**Jan Łukaszewicz**

* 📧 E-mail: plhareo@gmail.com
* 🔗 WWW: [hareo.pl](https://hareo.pl/)

---

## ⚖️ Licencja

Ten projekt jest udostępniany na licencji **MIT**.

Należy jednak pamiętać, że port zawiera kod abstrakcji sprzętowej (HAL/LL) oraz middleware od **STMicroelectronics**. Pliki te (szczególnie w folderach sterowników RNG i Flash) zachowują swoje oryginalne licencje producenta:

* **SLA0044** (ST Ultimate Liberty)
* **BSD-3-Clause**

</details>

## 🇺🇸 English

This repository provides the hardware abstraction layer (HAL) and specific platform optimizations for the **[OTApp Framework](https://github.com/HareoPL/ot_app)** on **STM32WBA6** series microcontrollers.

## 🛠 Platform Specifics
- **MCU:** STM32WBA6 (Cortex-M33).
- **Toolchain:** STM32CubeIDE / GNU Arm Embedded Toolchain.
- **RTOS:** FreeRTOS (integrated via CMSIS-OS2).
- **Wireless Stack:** STM32WPAN Middleware (OpenThread FTD/MTD).

---

## 🚀 Key Platform Improvements

This port implements critical stability and performance fixes that address limitations found in standard SDK examples:

### 💾 Optimized NVM with Wear-Leveling
Standard implementations did not save settings to persistent memory. This port saves settings to FLASH memory and introduces an intelligent non-volatile memory management mechanism:
- **Slot Mechanism:** A single 8kB Flash page is divided into 2kB slots. Data is appended to the next available slot instead of overwriting the same address.
- **Increased Longevity:** By rotating slots, the physical Flash page wears out 4 times slower, theoretically increasing durability from the standard 100k to **400k write cycles**.
- **Asynchronous Commits:** Utilizes a **FreeRTOS Timer** (debounce) and a dedicated **low-priority Task**. The timer triggers the actual Flash write only after a period of inactivity, eliminating redundant operations during rapid configuration changes.

### 🎲 Stable HW RNG (Random Number Generator)
Fixed stability issues of the hardware random number generator during high-intensity requests from the OpenThread stack:
- **The Problem:** The factory driver disabled the RNG immediately after every use. Under frequent access, this led to clock synchronization issues (Clock Error) and system hangs.
- **The Solution:** Implemented a **Delayed Disable Strategy** using a FreeRTOS timer and task. The RNG remains active for a short window after the last use. If a new number is requested within this window, the timer resets, avoiding the costly and risky peripheral restart.

---

## 🔌 Hardware Setup (Example: NUCLEO-WBA65)
The port supports `ot_device` implementation with the following configuration:

todo

## 🔌 Getting Started
1. Clone the repository with submodules:
```bash
   git clone --recursive [https://github.com/HareoPL/ot_app_stm.git](https://github.com/HareoPL/ot_app_stm.git)

```

2. Import the project into **STM32CubeIDE**.
3. Build and flash using the integrated debugger.

## 🔗 Core Framework & Resources

For detailed documentation on the framework logic, CoAP API, and pairing mechanisms, visit:

* **Main Repository:** 👉 [github.com/HareoPL/ot_app](https://github.com/HareoPL/ot_app)
* **Documentation:** 👉 [Hareo.pl/otapp](https://hareo.pl/otapp)
* **Other Platforms:** 
</br> 👉 ESP32-C6: **[ github.com/HareoPL/ot_app_esp](https://github.com/HareoPL/ot_app_esp)**
</br> 👉 Control Panel (STM32H7 + ESP32-C6 + LCD): **[github.com/HareoPL/ot_app_cp](https://github.com/HareoPL/ot_app_cp)**
## 👨‍💻 Author and Contact

**Jan Łukaszewicz**

* 📧 E-mail: plhareo@gmail.com
* 🔗 WWW: [hareo.pl](https://hareo.pl/)

---

## ⚖️ License

This project is licensed under the **MIT License**.

However, this port includes hardware abstraction code (HAL/LL) and middleware from **STMicroelectronics**. Specific files (notably in the NVM and RNG drivers) maintain their original licenses:

* **SLA0044** (ST Ultimate Liberty)
* **BSD-3-Clause**

Please refer to individual file headers for the full license texts.
