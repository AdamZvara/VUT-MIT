## Popis projektu
Cílem projektu je implementovat systém pro analýzu logovacích souborů a detekci anomálií v logovacích datech pomocí metod strojového učení. Projekt zahrnuje následující části:
- Výběr logovacího souboru - lze využít vlastní logovací soubory nebo doporučené datasety.
- Analýza datasetu, popis typů logovacích událostí, četnost jejich výskytu, rozložení.
- Analýza a výběr rysů (features) pro reprezentaci logovacích událostí pro strojové učení.
- Předzpracování a úprava dat pro strojové zpracování.
- Ohodnocení a výběr rysů (features), normalizace, kategorizace, redukce.
- Výběr modelu pro reprezentaci logovacích událostí.
- Výběr modelu a metody zpracování bude záviset na typu dat - zda obsahují anotaci či nikoliv (metody učení s učitelem či bez učitele).
- Implementace nástroje pro klasifikaci dat za použití strojového učení.
- Nástroj bude sloužit k vytvoření modelu na základě trénovacích dat a k detekci.
- Formát vstupu a výstupu je upřesněn v přiložené prezentaci.
- Implementace v jazyce C/C++/Python spustitelná z příkazové řádky na systému CentOS (merlin).
- Experimenty s datasety - validace, optimalizace parametrů modelu, testování.
- Vytvoření dokumentace (5-10 stran). Popis formátu a obsahu, viz přiložená prezentace.

Odevzdání projektu přes IS VUT
- Součástí projektu je archiv ZIP obsahující zdrojové kódy nástroje, Makefile, použití datové sobory, soubor Readme.txt a dokumentace ve formátu PDF. V případě že celková velikost souboru je více jak 50 MB, nahrajte datové soubory na sdílené úložište (Google Drive, VUT drive) a vložte odkaz na soubory do Readme.txt.