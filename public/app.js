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
        elArrivalVal.textContent = data.arrival_time;
        elTimeline.innerHTML = '';

        data.legs.forEach(leg => {
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
            elTimeline.appendChild(div);
        });

        elResults.classList.remove('hidden');
    }
});
