# Substation Night Patrol — Féléves grafika beadandó

## Projekt leírása

A **Substation Night Patrol** egy bejárható, belső nézetes 3D grafikus program C nyelven, **SDL2 + OpenGL** használatával.

A program egy éjszakai, ködös alállomás környezetét jeleníti meg. A játékosnak a terület bejárása közben hibás kapcsolókat kell megjavítania, hogy helyreállítsa a rendszer működését, aktiválja a világítást, majd elérje a végső terminált.

---

## Feladatkövetelmények teljesülése

A program teljesíti a beadandó alapkövetelményeit:

- bejárható 3D tér billentyűzettel és egérrel
- külső fájlból betöltött 3D modellek
- textúrázott objektumok
- interaktív elemek
- időfüggő animációk
- állítható megvilágítás
- F1 súgó

---

## Megvalósított többletfunkciók

A projekt az alábbi többletfunkciókat tartalmazza:

1. **Köd**
2. **Ütközésvizsgálat**
3. **Objektumkijelölés / picking**
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
- **Esc** — kilépés

---

## Technikai háttér

- **Nyelv:** C
- **Könyvtárak:** SDL2, OpenGL, GLU
- **Build rendszer:** `make`
- **Jelenetkezelés:** CSV alapú, adatvezérelt objektumlista
- **Animáció:** időfüggő frissítés (`delta_time`)
- **Ablak:** átméretezhető

---

## Kódszerkezet és refaktorálás

A projekt a beadandó véglegesítésénél refaktorálásra került, hogy a forráskód áttekinthetőbb és modulárisabb legyen.

A korábban hosszú `main.c` és `scene.c` fájlok felelősségi körök szerint szét lettek bontva.

### Főbb modulok

- `app/src/main.c`  
  Az alkalmazás belépési pontja, az SDL/OpenGL inicializálás, az eseménykezelés és a fő ciklus.

- `app/src/app_render.c`  
  A globális renderelési beállítások kezelése: vetítés, fények, köd.

- `app/src/ui_hud.c`  
  A HUD és a 2D overlay elemek kirajzolása: célkereszt, alsó státuszsor, győzelmi kijelzés.

- `app/src/scene_loader.c`  
  A jelenet beolvasása CSV-ből, modellek és textúrák regisztrálása, objektumok létrehozása.

- `app/src/scene_update.c`  
  A jelenet logikája és állapotfrissítése: kapcsolók, kapunyitás, animációk, interakciók.

- `app/src/scene_query.c`  
  Lekérdezések és számítások: ütközésvizsgálat, objektumkijelölés, AABB számítások, dinamikus fények pozíciója.

- `app/src/scene_render.c`  
  A jelenet objektumainak kirajzolása.

- `app/src/scene_effects.c`  
  Speciális vizuális effektek: kapcsolószikrák, lámpafény, egyéb megjelenítési kiegészítések.

- `app/include/scene.h`  
  A publikus scene API.

- `app/include/scene_internal.h`  
  A scene modul belső adatai és segédfüggvényei.

Ez a bontás a C nyelv keretei között objektumszerűbb, modulárisabb szerkezetet eredményez.

---

## Projektstruktúra

```text
app/
├── Makefile
├── include/
│   ├── app_render.h
│   ├── camera.h
│   ├── collision.h
│   ├── help.h
│   ├── scene.h
│   ├── scene_internal.h
│   ├── texture.h
│   └── ui_hud.h
└── src/
    ├── app_render.c
    ├── camera.c
    ├── help.c
    ├── main.c
    ├── scene_effects.c
    ├── scene_loader.c
    ├── scene_query.c
    ├── scene_render.c
    ├── scene_update.c
    ├── texture.c
    └── ui_hud.c

assets/
├── models/
├── textures/
└── scene.csv
```

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

A projekt a `c_sdk_220203` környezetben készült.

Ajánlott mappaszerkezet:

```text
C:\c_sdk_220203\Beadando_Substation\Grafika_Beadando
```

A fordítás az `app` mappában történik:

```bash
make
```

Futtatás ugyaninnen:

```bash
substation.exe
```

A program relatív útvonalakkal éri el az asseteket (`../assets/...`), ezért célszerű az `app` könyvtárból futtatni.

---

## Külső komponensek

A projekt külső OBJ modellbetöltő kódot használ az `app/ext/obj` könyvtárban.

Ez a segédkönyvtár a tantárgyhoz kiadott `me-courses` anyagban található `grafika/utils/obj` alapjaira épül, és a projektbe beépítve, részben igazítva került felhasználásra.


---

## Összegzés

A program teljesíti a féléves grafika beadandó minimális követelményeit, és több kiegészítő funkciót is megvalósít.

A végleges változatban a forráskód szerkezete refaktorálásra került, így a projekt átláthatóbb, karbantarthatóbb és a C nyelv keretei között modulárisabb felépítésű lett.
