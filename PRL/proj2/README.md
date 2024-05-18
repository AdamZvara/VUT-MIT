### Hra Life

Hra Life reprezentuje příklad tzv. celulárního automatu. Hrací pole se skládá z buněk, které se v každém kroku přepínají mezi dvěma stavy:

- živá (značíme 1)
- mrtvá (značíme 0)

Stavy buněk se v průběhu hry mění pomocí definované sady pravidel. Základní sada pravidel, kterou budete implementovat v projektu je následující:

- každá živá buňka s méně než dvěma živými sousedy umírá
- každá živá buňka se dvěma nebo třemi živými sousedy zůstává žít
- každá živá buňka s více než třemi živými sousedy umírá
- každá mrtvá buňka s právě třemi živými sousedy ožívá

Např. tato mřížka:
```
00000000
00111000
01110000
00000000
```
bude mít po třech krocích hry s pravidly viz výše tvar:
```
01000000
00001000
01000000
00001000
```
pro implementaci typu wrap-around.
<br>
<br>

Program bude implementován v jazyce C++ s použitím MPI. Jako první argument bude program akceptovat název souboru s definicí hracího pole pro hru Life, jako druhý počet kroků, který má hra Life provést. Při implementaci zvolte okolí buňky jako tzv. osmi okolí (Moorovo okolí).

### Výstup
Program na standardní výstup (stdout) vypíše stav hracího pole pro hru Life po provedení zadaného počtu kroků. Ve výpisu budou označeny části mřížky vypočítané procesory jako `ID: <část mřížky>`, kde:
- ID: rank procesoru
- <část mřížky>: část mřížky vyřešená procesorem s rankem ID

Tedy např.:
```
0: 00010000
0: 01001000
1: 01001000
1: 00100000
```