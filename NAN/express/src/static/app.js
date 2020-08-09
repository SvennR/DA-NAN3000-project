if ('serviceWorker' in navigator) {
    navigator.serviceWorker.register('sw.js')
        .then(function (registration) {
            console.log('[ServiceWorker] Registration successful, scope is:', registration.scope);
        })
        .catch(function (error) {
            console.log('[ServiceWorker] registration failed, error:', error);
        });
}
