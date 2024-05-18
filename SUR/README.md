## Zadání projektu do SUR 2023/2024

Bodové ohodnocení:   25 bodů
<br>

Úkolem je natrénovat detektor jedné osoby z obrázku obličeje a
hlasové nahrávky. Trénovací vzory jsou k dispozici v archívu na adrese:
<br>
https://www.fit.vutbr.cz/study/courses/SUR/public/projekt_2023-2024/SUR_projekt2023-2024.zip
<br>

Tento archív obsahuje adresáře:
<br>

- target_train
- target_dev

kde jsou trénovací vzory pro detekovanou osobu ve formátu PNG a WAV,
v adresářích:

- non_target_train
- non_target_dev

jsou potom negativní příklady povolené pro trénování detektoru.
Rozdělení dat do adresářů *_train a *_dev je možné použít pro trénování
a vyhodnocování úspěšnosti vyvíjeného detektoru, toto rozdělení však
není závazné (např.  pomocí technik jako je cross-validation lze
efektivně trénovat i testovat na všech datech). Při pokusech o jiné
rozdělení dat může být užitečné respektovat informace o tom, které
trénovací vzory patří stejné osobě a zda-li byly pořízený v rámci
jednoho nahrávacího sezení. Jméno každého souboru je rozděleno do poli
pomocí podtržítek (např. f401_01_f21_i0_0.png), kde první pole (f401)
je identifikátor osoby a druhé pole je číslo nahrávacího sezení (01).
<br>

Ke trénování detektorů můžete použít pouze tyto dodané trénovací data.
NENÍ POVOLENO jakékoli využití jiných externích řečových či obrázkových
dat, jakožto i použití již předtrénovaných modelů (např. pro extrakci
reprezentací (embeddings) obličejů nebo hlasu). Tyto data ale mužete
"augmentovat" tak, že vytvoříte nové trénovací vzory např. přidáním šumu
do nahravek či obrázků, rotací, posunutím či změnou velikosti obrázků,
změnou rychlosti řeči apod.
<br>

Ostrá data, na kterých budou vaše systémy vyhodnoceny, budou k
dispozici v sobotu, 20. dubna ráno. Tato data budu obsahovat řádově
stovky souboru ke zpracování.  Vaším úkolem bude automaticky zpracovat
tato data vašimi systémy (věříme Vám že nebudete podvádět a dívat se
na obrázky čí poslouchat nahrávky) a uploadovat  soubory s výsledky do
WISu. Soubor s výsledky bude ASCII se třemi poli na řádku oddělenými
mezerou. Tato pole budou obsahovat popořadě následující údaje:

 - jméno segmentu (jméno souboru bez přípony .wav či .png)
 - číselné skóre, o kterém bude platit, že čím větší má hodnotu, tím si je
   systém jistější, že se jedná o hledanou osobu
 - tvrdé rozhodnutí: číslo 1 pro hledanou osobu jinak 0. Toto rozhodnutí
   proveďte pro předpoklad, že apriorní pravděpodobnost výskytu hledané
   osoby v každém testovaném vzoru je 0,5

V jakém programovacím jazyce budete implementovat váš detektor či
pomocí jakých nástrojů (spousta jich je volně k dispozici na
Internetu) budete data zpracovávat záleží jen na Vás. Odevzdat můžete
několik souborů s výsledky (např. pro systémy rozhodujícím se pouze na
základě řečové nahrávky či pouze obrázku). Maximálně však námi bude
zpracováno 5 takových souborů.
<br>

Soubory s výsledky můžete do pondělí 22. dubna 23:59 uploadovat do
WISu. Klíč se správnými odpověďmi bude zveřejněn 23. dubna. Na poslední
přednášce 24. dubna 2024 bychom měli analyzovat Vaše výsledky a řešení.
<br>

Na tomto projektu budete pracovat ve skupinách (1-2 lidí), do kterých
se můžete přihlásit v IS. Jména souborů s výsledky pro jednotlivé
systémy volte tak, aby se podle nich dalo poznat o jaký systém
se jedná (např. audio_GMM, image_linear). Každá skupina uploadne
všechny soubory s výsledky zabalené do jednoho ZIP archívu se
jménem login1_login2.zip či login1.zip, podle toho, kolik Vás
bude ve skupině. Kromě souborů s výsledky bude archív obsahovat
také adresář SRC/, do kterého uložíte soubory se zdrojovými kódy
implementovaných systémů. Dále bude archív obsahovat soubor dokumentace.pdf,
který bude v českém, slovenském nebo anglickém jazyce popisovat Vaše řešení
a umožní reprodukci Vaší práce. Důraz věnujte tomu, jak jste systémy během
jejich vývoje vyhodnocovali, a které techniky či rozhodnutí se pozitivně
projevily na úspěšnosti systému. Tento dokument bude také popisovat jak
získat Vaše výsledky pomocí přiloženého kódu. Bude tedy uvedeno jak Vaše
zdrojové kódy zkompilovat, jak vaše systémy spustit, kde hledat
výsledné soubory, jaké případné externí nástroje je nutné instalovat a
jak je přesně použít, atd. Očekávaný rozsah tohoto dokumentu jsou
3 strany A4. Do ZIP archívu prosím nepřikládejte evaluační data!
<br>

Inspiraci pro vaše systémy můžete hledat v archívu demonstračních příkladů
pro předmět SUR:
<br>
https://www.fit.vutbr.cz/study/courses/SUR/public/prednasky/demos/
<br>

Zvláště se podívejte na příklad detekce pohlaví z řeči: demo_genderID.py
Užitečné vám mohou být funkce pro načítaní PNG souborů (png2fea) a extrakci
MFCC příznaků z WAV souborů (wav16khz2mfcc).
<br>

Hodnocení:
- vše je odevzdáno a nějakým způsobem pracuje:
  - čtou se soubory,
  - produkuje se skóre
  - jsou správně implementovány a natrénovány nějaké "rozumné" detektory
    pro obrázky a pro nahrávky a/nebo kombinaci obou modalit (detektory
    nemusí pracovat se 100% úspěšností, jsou to reálná data!)
  - jsou odevzdány všechny požadované soubory v požadovaných formátech.
  - v dokumentaci vysvětlíte, co, jak a proč jste dělali a co by se ještě dalo zlepšit.
  ... plný počet 25 bodů.

- něco z výše uvedeného není splněno ? ... méně bodů.