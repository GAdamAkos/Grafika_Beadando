# Substation Night Patrol — Féléves grafika beadandó

## Projekt leírása

A **Substation Night Patrol** egy bejárható, belső nézetes 3D grafikus program C nyelven, **SDL2 + OpenGL** használatával.

A program egy éjszakai, ködös alállomás környezetét jeleníti meg. A játékosnak a terület bejárása közben interaktív kapcsolókat kell aktiválnia, hogy helyreállítsa a rendszer működését, bekapcsolja a világítást, majd elérje a végső terminált.

---

## Kötelező elemek teljesülése

A program tartalmazza a beadandóhoz szükséges alapfunkciókat:

- bejárható 3D tér billentyűzettel és egérrel
- külön fájlból betöltött modellek
- textúrázott objektumok
- interaktív elemek
- időfüggő animációk
- fényerő állítás
- F1 súgó

---

## Megvalósított többletfunkciók

A projekt az alábbi 5 többletfunkciót tartalmazza:

1. **Köd**
2. **Ütközésvizsgálat**
3. **Objektumkijelölés**
4. **Dinamikus fények**
5. **Stencil buffer alapú outline kiemelés**

---

## Irányítás

- **W / A / S / D** — mozgás
- **Egér** — körbenézés
- **E** — interakció
- **+ / -** — fényintenzitás állítása
- **, / .** — ködsűrűség állítása
- **F1** — súgó megjelenítése / elrejtése
- **G** — debug rács ki / be
- **Esc** — kilépés

---

## Technikai jellemzők

- **Nyelv:** C
- **Könyvtárak:** SDL2, OpenGL, GLU
- **Build rendszer:** make
- **Jelenetkezelés:** CSV alapú, adatvezérelt objektumlista
- **Animáció:** időfüggő frissítés (`delta_time`)
- **Ablak:** átméretezhető

---

## Projektstruktúra

```text
/app
  /src
  /include
  /ext
  Makefile
/assets
README.md
```

Belépési pont:
- `app/src/main.c`

---

## Assets

A projekt az alábbi asseteket használja:

- 3D modellek (`.obj`)
- textúrák (`.bmp`)
- jelenetleírás (`scene.csv`)

**Assets ZIP link:**  
[Open link](https://drive.google.com/file/d/1P-zYNdb-dVqMzteyIKPh2GgfRCUkWvRz/view?usp=sharing)

---

## Fordítás és futtatás

A projektet a `c_sdk_220203.zip` csomagból kicsomagolt fejlesztői környezetben érdemes használni, és a repót ezen belül ajánlott elhelyezni.

Ajánlott mappaszerkezet:

```text
C:\c_sdk_220203\Beadando_Substation\Grafika_Beadando
```

A program fordítása az `app` mappában történik:

```bash
make
```

A futtatás szintén az `app` mappából ajánlott, mert a program relatív útvonalakkal tölti be az asseteket (`../assets/...`).

Windows alatt a futtatás:

```bash
substation.exe
```

---

## Külső komponensek

A projekt külső OBJ modellbetöltő kódot használ az `app/ext/obj` könyvtárban.

Ennek alapja a tantárgyhoz kiadott `me-courses` anyagban található
`grafika/utils/obj` segédkönyvtár, amely a féléves feladathoz
illesztve került beépítésre és részben módosításra.
---

## Összegzés

A program teljesíti a féléves grafika beadandó minimális követelményeit, és 5 többletfunkciót is tartalmaz.
