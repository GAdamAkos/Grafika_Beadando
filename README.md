# Substation Night Patrol — Féléves grafika beadandó

## 1. Projekt leírása

A **Substation Night Patrol** egy belső nézetes, bejárható 3D grafikus program C nyelven, **SDL2 + OpenGL** használatával.
A program egy ködös, éjszakai **alállomás / trafóudvar** környezetét jeleníti meg, ahol a játékosnak a terület bejárása közben különböző kapcsolókat kell aktiválnia, hogy visszaállítsa a világítást és elérje a végső terminált.

A beadandó célja a valós idejű 3D grafikai programozási alapismeretek bemutatása:
- kamerakezelés,
- modellbetöltés fájlból,
- textúrázás,
- fények kezelése,
- interaktív objektumok,
- időfüggő animációk,
- köd és ütközésvizsgálat.

---

## 2. Játékmenet és működés

A játékos FPS nézetben bejárja az alállomás területét. A képernyő közepén található célkereszt segítségével ki lehet jelölni az interaktív objektumokat. Az **E** billentyűvel aktiválhatók a kapcsolók.

A kapcsolók hatására:
- egyes lámpák bekapcsolnak,
- dinamikus fényforrások aktiválódnak,
- bizonyos objektumok animációja megváltozik,
- a küldetés fokozatosan előrehalad.

Ha a szükséges rendszerek helyreálltak, a játékos a végső terminálnál lezárhatja a küldetést.

---

## 3. Minimális követelmények teljesülése

### 3.1 Kamerakezelés
A virtuális tér billentyűzettel és egérrel bejárható.
- **W / A / S / D**: mozgás
- **Egér**: körbenézés

### 3.2 Objektumok külön fájlból
A jelenet 3D modelljei külön fájlból töltődnek be.
A projekt jelenlegi állapotában OBJ modellek használhatók, a jelenet elemei adatvezérelt módon kerülnek betöltésre.

### 3.3 Interaktivitás és animáció
A program interaktív:
- a játékos kapcsolókat aktiválhat,
- a világítás állapota változik,
- egyes elemek animáltak,
- az animációk **delta time** alapján frissülnek.

### 3.4 Textúrák
A modellek textúrázottak.
A jelenlegi verzió BMP textúrákat használ.

### 3.5 Fények állítása
A globális fényintenzitás állítható:
- **+ / -**: fényerő növelése / csökkentése

### 3.6 Használati útmutató
A program tartalmaz beépített súgót:
- **F1**: segítségképernyő megjelenítése / elrejtése

---

## 4. Megvalósított többletfunkciók

A kötelező követelményeken túl a program jelenleg az alábbi plusz funkciókat tartalmazza:

1. **Köd**
   - A jelenet ködhatást használ.
   - A köd sűrűsége futás közben állítható.

2. **Ütközésvizsgálat**
   - A kamera nem tud szabadon átmenni minden objektumon.
   - A program AABB alapú collision ellenőrzést használ.

3. **Dinamikus fények**
   - Az interaktív elemekhez kapcsolódóan dinamikus fényforrások aktiválódnak.
   - A lámpák intenzitása pulzáló hatást is használhat.

4. **Objektum kijelölés célkereszttel**
   - A program a kamera nézeti iránya alapján kijelöli a célkereszt alatt lévő objektumot.
   - Az interakció az **E** billentyűvel történik.

---

## 5. Irányítás

### Mozgás és nézet
- **W / A / S / D** — mozgás
- **Egér** — körbenézés

### Interakció
- **E** — interakció a kijelölt objektummal

### Fények és köd
- **+ / -** — fényintenzitás állítása
- **, / .** — ködsűrűség csökkentése / növelése

### Egyéb
- **F1** — súgó megjelenítése / elrejtése
- **G** — debug rács ki/be
- **Esc** — kilépés

---

## 6. Technikai jellemzők

- **Nyelv:** C
- **Könyvtárak:** SDL2, OpenGL, GLU
- **Build rendszer:** make
- **Ablakkezelés:** átméretezhető SDL ablak
- **Vetítés:** perspektivikus kamera
- **Jelenetkezelés:** adatvezérelt objektumlista CSV alapján
- **Textúrázás:** BMP alapú textúrák
- **Animáció:** időfüggő frissítés (`delta_time`)

---

## 7. Projektstruktúra

```text
/app
  /src
  /include
  /ext
  Makefile
/gyakorlatok vagy /demos
README.md
```

A program fő belépési pontja:
- `app/src/main.c`

A futtatáshoz szükséges assetek külön `assets` könyvtárban találhatók.
A végleges beadásnál az `assets` mappa külön ZIP-ben kerül megosztásra, és annak linkje a README-ben szerepel majd.

---

## 8. Assets

A projekt az alábbi asset típusokat használja:
- 3D modellek (`.obj`)
- textúrák (`.bmp`)
- jelenetleírás (`scene.csv`)

Ajánlott struktúra:

```text
/assets
  /models
    *.obj
  /textures
    *.bmp
  scene.csv
```

**Assets ZIP link:** ide kerül a végleges link

---

## 9. Fordítás és futtatás

A projekt a tantárgyhoz használt C/SDL2 fejlesztőkörnyezettel fordítható.

### Fordítás
Lépj be az `app` mappába, majd futtasd:

```bash
make
```

### Futtatás
A lefordított program futtatása az `app` mappából történik.
A jelenlegi projektben a futtatható állomány neve:

```bash
substation.exe
```

---

## 10. Jelenlegi állapot

A program jelenleg már teljesíti a beadandó minimális elvárásainak döntő részét:
- bejárható 3D tér,
- külön fájlból töltött modellek,
- textúrázott objektumok,
- interaktív elemek,
- animációk,
- fénykezelés,
- súgó,
- ütközésvizsgálat,
- köd.

A végleges leadás előtt még az assets csomag és a README véglegesítése szükséges.

---
