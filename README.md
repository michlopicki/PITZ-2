# PITZ 2 @ TCS

---
## Opis Projektu
Poniższy projekt to pełna aplikacja z webową warstwą graficzną.
Obsługuje wyszukiwanie najszybszych połączeń pomiędzy dwoma przystankami z wybranych GTFSów.
Dodatkowo pokaże również przesiadki i przejścia piesze pomiędzy przystankami.
...

## Cel
Użycie algorytmu RAPTOR z różnymi wariantami do stworzenia wyszukiwarki najlepszego połączenia pomiędzy przystankami.

## Przed uruchomieniem
Wszystkie używane biblioteki są nagłówkowe wiec nie potrzeba nic instalować.
Jedynie pobrać potrzebne pliki GTFS:
```shell
make data
```
to pobierze pliki i rozpakuje je do podfolderów w folderze data.

## Run
Abu uruchomić wyszukiwarkę należy uruchomić:
```shell
make run
```
A następnie wejść na 
[localhost](http://localhost:8080/)

## Użycie LLM
Z założenia użyte tylko jako narzędzie do szukania informacji/dokumentacji i narzędzie do refaktoryzacji,
jeżeli czegoś jest autorem, lub współautorem to piszę o tym w dzienniku prac.