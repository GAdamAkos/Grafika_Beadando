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
5. **Árnyékszerű hatás**

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

A végleges beadásnál az `assets` mappa külön ZIP-ben kerül megosztásra.  
Az assets letöltési linkje ide kerül majd:

**Assets ZIP link:**  
Ide kerül a végleges link.

---

## Fordítás és futtatás

Lépj be az `app` mappába, majd futtasd:

```bash
make
```

Windows alatt a létrejövő futtatható állomány jellemzően:

```bash
substation.exe
```

---

## Külső komponensek

A projekt külső OBJ modellbetöltő kódot használ az `app/ext/obj` könyvtárban.

---

## Összegzés

A program teljesíti a féléves grafika beadandó minimális követelményeit, és 5 többletfunkciót is tartalmaz.