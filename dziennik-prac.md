# Dziennik prac
Luźne obserwacje, decyzje projektowe, ślepe uliczki.

***
### Plan na projekt

Myślałem nad różnymi opcjami projektu i zdecydowałem się na wyszukiwarkę połączeń z wykorzystaniem RAPTORa.
Jest to dobra opcja do wprowadzania nowych funkcjonalności (chociażby McRAPTOR i rRAPTOR).
Może zaimplementuje warstwę graficzną, albo zostanę przy wyszukiwaniu z terminala.

### GTFS

Oczywiście projekt zacząłem od sparsowania plików GTFS. 
Znalazłem nagłówkową bibliotekę just GTFS, która wygląda obiecująco.
Ma wszystkie potrzebe funkcje do sparsowania GTFSów.

Ucząc się na błędach z poprzedniego projektu, więcej poświęcam na dobre przechowywanie danych (cache friendly).
Dodatkowo całą strukturę grafu przechowuje w osobnym pliku, 
żeby w razie przyszłych zmian nie musieć refaktoryzować całego kodu.

Przechowuje płaskie tablice 1D, a zamiast trzymać wskaźniki na obiekty przechowuje offsety wektorów.
Powinno to zminimalizować cache misses. Zobaczymy.

### Struktura grafu i Graph builder

Cała struktura grafu siedzi w namespace raptor, w pliku graph_struct.hpp,
a w pliku builder zaimplementowałem budowanie grafu z gtfs.
Wprowadziłem następujące decyzje:

1. Mapuję identyfikatory przystanków na liczby 0...N-1
2. Grupuję kursy w unikalne trasy (różne warianty kursów)
3. Obliczam czas odjazdu i przyjazdu na przystanki
4. Na razie używam wzoru Heversine'a do liczenia footpaths.
Liczę podgraf przejść pieszych o promieniu 1,5 km
5. Graph działa na zasadzie append, dodaje kolejne pliki gtfs i każdy dodaje do grafu, po czym buduje go w całości
6. Ten sam przystanek z różnych GTFS jest traktowany jako dwa różne (na razie)


### RAPTOR

Zaimplementowałem podstawowy RAPTOR szukający najwcześniejszego możliwego przyjazdu dla danego czasu odjazdu.
Napotkane problemy:
- Nazwy przystanków - na początku straciłem w builderze nazwy przystanków i miałem tylko id
- Gubiłem czas odjechania połączenia, trzymając tylko czas przyjazdu na przystanek
- Czas przesiadki - miałem zerowy minimalny czas przesiadki -> ustawiłem go na 120 sekund domyślnie

Nierozwiązane:
- Nawet jak czas odjazdu pierwszego połączenia jest późniejszy to pokazuje wyjście piesze o zadanej godzinie
- Przejścia piesze pomiędzy tymi samymi przystankami (dodać numery przystanków czy złączyć w jeden)

### HTML i serwer
W tworzeniu warstwy graficznej LLM jest współautorem

Stworzyłem z pomocą LLM skrypt w js i stronę do wyświetlania wyszukiwarki połączeń w przeglądarce.
Dzięki temu można wybrać przystanki początkowy i końcowy (podpowiada dostępne), godzinę odjazdu i wyszukać połączenie.

