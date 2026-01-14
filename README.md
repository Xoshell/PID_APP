# PID Simulator

![PID Simulator Screenshot](screenshot.png)

**Popis projektu:**  
Aplikácia simuluje PID regulátor systému 1. rádu s grafickým zobrazením výstupov v reálnom čase.  
Používateľ môže meniť parametre systému a PID regulátora, sledovať priebeh výstupu, ukladať nastavenia a grafy.  
Cieľom projektu bolo implementovať funkčný PID simulátor v jazyku C++ s GUI v Qt.

---

## Požiadavky a inštalácia

### Platforma
- Windows 10/11 (aplikácia bola testovaná na Windows 11, Qt 6.10.1, MinGW-w64)

### Inštalácia Qt a knižníc
1. Stiahnuť [Qt Online Installer](https://www.qt.io/download-qt-installer)  
2. Vybrať verziu Qt 6.x s MinGW-w64 (ak používate Windows)  
3. Pri inštalácii zvoliť moduly:
   - **Qt Widgets**
   - **Qt Network** (ak sa používa TCP klient)  
4. Skontrolovať, že máte C++11 kompatibilný kompilátor (MinGW-w64 alebo MSVC)

### Ďalšie nástroje
- **QCustomPlot** – knižnica pre vykresľovanie grafov (už zahrnutá v projekte)

---

## Štruktúra projektu

/PID-Regulator-first-order-QT
│-- main.cpp
│-- mainwindow.h
│-- mainwindow.cpp
│-- mainwindow.ui
│-- pid.pro
│-- qcustomplot.cpp
│-- qcustomplot.h