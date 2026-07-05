# Podsumowanie
_wnioski, ograniczenia rozwiązania,
pomysły na dalszy rozwój_

----
Poniżej przedstawiam wypunktowane podsumowanie projektu

## Wnioski
- Wyszukiwarka działa natychmiastowo nawet dla dużej ilości przystanków, dzięki dobrej optymalizacji pamięci.
- Dzięki zbudowaniu aplikacji webowej można bardzo wygodnie przetestować aplikację na różnych trasach.

## Ograniczenia
- Brak obsługi tych samych przystanków z różnych gtfs. 
Są traktowane jako różne ale można pomiędzy nimi przejść pieszo, bo wygenerowałem siatkę przejść pieszych.
- Krawędzie piesze są wyliczane tylko z odległości co przy skrajnych przypadkach (np. rzeka lub tory) może być zypełnie nieprawdziwe.
- Zbiory pareto są tylko pod kątem czasu i liczby przesiadek, nie rozważam np rodzaju lini, albo ceny biletu.

## Pomysły na dalszy rozwój
- rMcRAPTOR - dodanie zakresu czasu do wyszukiwania. użytkownik wybiera okno czasowe i otrzymuje zbiór pareto optymalnych połąćzeń.
- Integracja z OSM: 
  - Zamiast wzoru Harvesine liczenie rzeczywistej trasy pieszej
  - Wyświetlanie wyniku na mapie w aplikacji (wszystkie punkty już mam wystarczy wyświetlić).
- Scalanie przystanków z różnych gtfs np. po lokalizacji lub nazwie.
