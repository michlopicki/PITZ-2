document.addEventListener('DOMContentLoaded', async () => {
    const stopsList = document.getElementById('stops-list');
    const form = document.getElementById('search-form');

    const elLoading = document.getElementById('loading');
    const elResults = document.getElementById('results');
    const elError = document.getElementById('error');
    const elTimeline = document.getElementById('timeline');
    const elArrivalVal = document.getElementById('arrival-val');
    const elErrorVal = document.getElementById('error-val');

    let stopsData = [];

    // Pobieranie listy przystanków dla Autocomplete
    try {
        const response = await fetch('/api/stops');
        stopsData = await response.json();
        
        // Wypełnianie datalist
        const fragment = document.createDocumentFragment();
        stopsData.forEach(stop => {
            const option = document.createElement('option');
            option.value = stop.name;
            option.dataset.id = stop.id;
            fragment.appendChild(option);
        });
        stopsList.appendChild(fragment);
    } catch (err) {
        console.error("Błąd podczas pobierania przystanków:", err);
    }

    // znajdź ID przystanku po nazwie
    function getStopIdByName(name) {
        const stop = stopsData.find(s => s.name === name);
        return stop ? stop.id : null;
    }

    // Obsługa wysłania formularza
    form.addEventListener('submit', async (e) => {
        e.preventDefault();
        
        const sourceName = document.getElementById('source').value;
        const targetName = document.getElementById('target').value;
        const timeVal = document.getElementById('time').value;

        const sourceId = getStopIdByName(sourceName);
        const targetId = getStopIdByName(targetName);

        if (sourceId === null || targetId === null) {
            showError("Nie znaleziono takiego przystanku w bazie.");
            return;
        }

        // Przygotowanie widoku
        elError.classList.add('hidden');
        elResults.classList.add('hidden');
        elLoading.classList.remove('hidden');

        try {
            const res = await fetch(`/api/route?source=${sourceId}&target=${targetId}&time=${timeVal}`);
            const data = await res.json();
            
            elLoading.classList.add('hidden');

            if (data.error) {
                showError(data.error);
                return;
            }

            renderRoute(data);

        } catch (err) {
            elLoading.classList.add('hidden');
            showError("Błąd serwera podczas wyszukiwania trasy.");
        }
    });

    function showError(msg) {
        elErrorVal.textContent = msg;
        elError.classList.remove('hidden');
    }

    function renderRoute(data) {
        if (!data.journeys || data.journeys.length === 0) {
            showError("Brak połączeń spełniających kryteria.");
            return;
        }

        elResults.innerHTML = '<h2>Znalezione opcje:</h2>';

        data.journeys.forEach((journey, index) => {
            const numTransfers = journey.legs.filter(l => l.type === 'transit').length - 1;
            const transfersText = numTransfers > 0 ? `${numTransfers} przesiadki` : `Bez przesiadek`;

            const panel = document.createElement('div');
            panel.className = 'glass-panel result-panel';
            panel.innerHTML = `
                <div style="display: flex; justify-content: space-between; align-items: center; border-bottom: 1px solid rgba(255,255,255,0.1); padding-bottom: 15px; margin-bottom: 15px;">
                    <h3 style="color: #10B981;">Opcja ${index + 1}</h3>
                    <span style="background: rgba(79, 70, 229, 0.2); color: #4F46E5; padding: 4px 10px; border-radius: 8px; font-weight: bold;">
                        ${transfersText}
                    </span>
                </div>
                <p class="arrival-time">Przyjazd o: <span style="color: #4F46E5; font-size: 1.5rem;">${journey.arrival_time}</span></p>
                <div class="timeline"></div>
            `;

            const timeline = panel.querySelector('.timeline');

            journey.legs.forEach(leg => {
                const div = document.createElement('div');
                div.className = `leg ${leg.type}`;
                
                const routeClass = leg.type === 'walk' ? 'walk' : 'transit';
                
                div.innerHTML = `
                    <div class="leg-time">${leg.start_time} - ${leg.end_time}</div>
                    <div class="leg-desc">
                        Z: ${leg.from_stop} <br>
                        Do: ${leg.to_stop}
                        <span class="route ${routeClass}">${leg.route_name}</span>
                    </div>
                `;
                timeline.appendChild(div);
            });

            elResults.appendChild(panel);
        });

        elResults.classList.remove('hidden');
    }
});
