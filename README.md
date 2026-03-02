# Substation Night Patrol — Féléves feladat specifikáció (Grafika / C)

## 1) Koncepció

A **Substation Night Patrol** egy kisméretű, belső nézetes (FPS) 3D bejárható mini-játék, amely egy **ködös, éjszakai alállomás / trafóudvar** környezetében játszódik.  
A játékos bejárja a területet, **interaktív kapcsolószekrényeket** (control box) keres, majd ezek kapcsolóit kezelve **szektoronként visszakapcsolja a világítást** (reflektorok, jelzőfények).

A projekt célja, hogy a valós időben futó 3D grafikus programozási ismereteimet bizonyítsam **C nyelven**, **SDL2 + OpenGL** használatával, tiszta, jól strukturált kódbázissal és reprodukálható build folyamattal.

---

## 2) Játékmenet (core loop)

1. **Bejárás**: a trafóudvar felfedezése FPS kamerával (billentyűzet + egér).
2. **Kijelölés**: interaktív objektum kijelölése egérkattintással.
3. **Interakció**: a kijelölt kapcsoló aktiválása, amely:
   - egy vagy több fényt kapcsol (reflektor, jelzőfény),
   - vizuális visszajelzést ad (pl. LED villogás),
   - opcionálisan esemény-effektet indít (pl. szikra/füst részecske).
4. A környezetben **időfüggő animációk** futnak (pl. ventilátor forgása, jelzőfény pulzálása), amelyek **deltaTime** alapján frissülnek (nem képkockaszám alapján).

---

## 3) Kötelező elvárások (oktatói specifikáció szerint)

A program az alábbi minimális követelményeket valósítja meg:

### 3.1 Kamerakezelés
- A virtuális tér bejárható **billentyűzettel és/vagy egérrel** (FPS kamera).

### 3.2 Térbeli objektumok betöltése fájlból
- A jelenetben **3D modellek** szerepelnek, amelyek **külön fájlokból** tölthetők be (pl. OBJ).
- A modellek az assets csomagban: `assets/models/`.

### 3.3 Interaktivitás + animáció
- A program **interaktív**.
- Modellek és/vagy fények billentyűzettel/egérrel változtathatók.
- Vannak **animált** részek (időfüggő frissítéssel).

### 3.4 Textúrák
- A modellek **textúrázottak** (PNG/JPG), az assets csomagban: `assets/textures/`.

### 3.5 Fények állítása
- A fények paraméterei **`+`** és **`-`** billentyűkkel állíthatók (intenzitás / komponensek).

### 3.6 Használati útmutató
- **`F1`** megnyomására megjelenik egy in-program leírás (help overlay) a kezelésről és funkciókról.

---

## 4) Többletfunkciók (jobb jegyért)

Tervezett / megvalósított plusz funkciók (mindegyik +1 jegy, max. 5-ig):

1. **Köd (dinamikus):** ködsűrűség valós időben állítható.
2. **Ütközésvizsgálat (AABB):** a kamera nem mehet át kerítésen/objektumokon.
3. **Objektum kijelölése egérrel (picking):** kattintással kijelölhető objektum (ray casting / color picking).
4. *(Opcionális)* **Részecskerendszer:** szikra vagy füst effekt eseményhez kötve.

> Cél: legalább **3** többletfunkció megvalósítása.

---

## 5) Irányítás (Controls)

### Kamera
- **W/A/S/D** — mozgás
- **Egér** — körbenézés
- **Shift** — gyorsabb mozgás (opcionális)
- **Space / Ctrl** — fel / le (opcionális, ha implementálva)

### Interakció
- **Bal egérgomb** — objektum kijelölése
- **E** — interakció a kijelölt objektummal (kapcsoló)
- **R** — kijelölés törlése (opcionális)

### Fények / Effektek
- **+ / -** — fényintenzitás állítása
- **[ / ]** — ködsűrűség csökkentése / növelése

### Súgó és kilépés
- **F1** — súgó ki/be
- **Esc** — kilépés

---

## 6) Technikai terjedelem

### 6.1 Megjelenítés (render)
- SDL2 ablak + OpenGL kontextus
- Ablak átméretezés támogatása helyes képaránnyal
- Shader-alapú megjelenítés (ajánlott)
- Textúrázott mesh-ek (OBJ) alap fényeléssel (ambient + diffuse + specular vagy hasonló)

### 6.2 Adatvezérelt jelenet (repetitív kód kerülése)
- A jelenet objektumlistája és elhelyezése konfigurációs fájlban legyen (pl. `assets/config/scene.csv`)
- A program innen tölti be az objektumokat, transzformációkat és anyag/texture hozzárendeléseket

### 6.3 Kódminőségi elvárások
- Angol elnevezések a forráskódban
- Minimális globális változók
- Moduláris felépítés (header fájlokban dokumentált interfészek)
- `make`-kel forduljon, törekvés **warning nélkül**
- Külső kód/asset esetén licenc + forrás feltüntetése

---

## 7) Repository struktúra

A repository **csak a forráskódot** tartalmazza. Az assets külön ZIP-ben van kezelve.

```
/app
  /src
  /include
  /shaders
  main.c
  Makefile
/demos
README.md
.gitignore
LICENSE
```

---

## 8) Assets csomag (külső ZIP)

A repository **nem tartalmazza** az `assets/` mappát.  
Az assets csomag ZIP-ben kerül feltöltésre külső tárhelyre, és a link itt szerepel:

**Assets ZIP link:** majd itt lesz a ink

Elvárt struktúra a ZIP-ben:
```
/assets
  /models
    *.obj
  /textures
    *.png
  /config
    scene.csv
```

Megjegyzések:
- Hordozható útvonalak (kerülendő a backslash a tárolt path-okban).
- Következetes fájlnév-kis/nagybetű használat (más rendszereken számít).

---

## 9) Fordítás és futtatás (Windows + C SDK)

A projekt a tantárgyhoz adott **C SDK** (MinGW + SDL2 + OpenGL) környezettel fordul.

### Build lépések
1. A C SDK mappában indítsd el a `shell.bat` fájlt (beállítja a PATH-ot a gcc/make-hez).
2. A megnyíló terminálban lépj a projektre:
   ```
   cd path\to\your_repo\app
   ```
3. Fordítás:
   ```
   make
   ```
4. Futtatás terminálból:
   ```
   .\main.exe
   ```

---

## 10) Külső források / attribúció

Ha külső asset vagy kódrészlet kerül felhasználásra, itt fogom feltüntetni.

### Modellek / textúrák
- Forrás:
- Licenc:
- Attribúció: