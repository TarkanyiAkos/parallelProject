# parallelProject

Párhuzamos Programozás beadandó feladat forrásfájljai, futtathatói és maga a bemutató.


**_________________**


**Fájlok**:

Measurements.xlsx - A méréseket tartalmazó Excel munkafüzet

Párhuzamos Algoritmusok.pptx - Az előadást tartalmazó PowerPoint-bemutató


**_________________**

FELADATOK:

1) Megadott string előfordulásainak megkeresése és kilistázása megadott fájlból. A felhasználó be tudja állítani, hogy hány szálon fusson az algoritmus.
   
   a) POSIX
   
   b) OpenMP
   
3) Egy adott képhalmaz elemeinek értelmezése és a rajtuk lévő ruhadarabok megnevezése KNN algoritmussal.

**_________________**

**posix**

Az első feladat a) része.

Futtatási argumentumok: keresett szöveg, szállak száma, szövegfájl.
Példa futtatás: ./posix "person" 5 "testText.txt"

**_________________**

**openmp**

Az első feladat b) része.

Futtatási argumentumok: keresett szöveg, szállak száma, szövegfájl.
Példa futtatás: ./openmp "person" 5 "testText.txt"

**_________________**


**knn**

A második feladat.

Nem tartalmaz futtatási argumentumokat, helyette bekéri a kellő adatokat indításkor.

A felhasználó kiválaszthatja a keresett ruhadarab típusát, illetve a felhasznált szállak számát.
A keresés és a knn kiképzés csak az előre megadott ruha-kép halmazon működik jelenleg, de a program képes lenne más képekkel is dolgozni.
