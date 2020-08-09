/* Alot of the service worker was written after watching the following tutorials / docs
https://pwa-workshop.js.org/
https://developer.mozilla.org/en-US/docs/Web/API/Service_Worker_API
https://developers.google.com/web/fundamentals/instant-and-offline/offline-cookbook
The Net Nijnja (Youtube) - https://www.youtube.com/playlist?list=PL4cUxeGkcC9gTxqJBcDmoi5Q2pzDusSL7
*/

// Set a name for the current cache
var STATIC_CACHE_NAME = 'site-static-v10';
var DYNAMIC_CACHE_NAME = 'site-dynamic-v10';

// Default files to always handle
var STATIC_CACHED_URLS = [
    './images/icons/icon-72x72.png',
    './images/icons/icon-96x96.png',
    './images/icons/icon-128x128.png',
    './images/icons/icon-144x144.png',
    './images/icons/icon-152x152.png',
    './images/icons/icon-192x192.png',
    './images/icons/icon-384x384.png',
    './images/icons/icon-512x512.png',
    './app.js',
    './manifest.webmanifest',
    './scripts.js',
    './style.css',
    './index.html',
    'https://images.ctfassets.net/3s5io6mnxfqz/3ys8X1VxIfHJWgUY0je6fx/bc3ffc9c76e342fe8d5e14f723739e32/AdobeStock_138628179.jpeg'
];

self.addEventListener('install', event => {
    console.log('[ServiceWorker] ' + STATIC_CACHE_NAME + ' installing...')
    skipWaiting(); // Kicks out the active worker
    // Perform install steps
    event.waitUntil(
        caches.open(STATIC_CACHE_NAME)
            .then(function (cache) {
                console.log('[ServiceWorker] caching static files');
                return cache.addAll(STATIC_CACHED_URLS);
            })
    );
});

// This event is called when an installed/waiting service worker is ready to become the active service worker and replace the old active one.
self.addEventListener('activate', event => {
    event.waitUntil(
        caches.keys().then(function (keys) {
            return Promise.all(keys
                .filter(key => key !== STATIC_CACHE_NAME && key !== DYNAMIC_CACHE_NAME)
                .map(key => {
                    console.log('[ServiceWorker] deleting old cache: ' + key)
                    caches.delete(key)
                })
            ).then(() => console.log('[ServiceWorker] ' + STATIC_CACHE_NAME + ' ready to handle fetches'))
        })
    );
});

self.addEventListener("fetch", function (event) {
    if (event.request.method !== 'GET') {
        //  No need to do anything
        event.respondWith(fetch(event.request));
    }
    else if (event.request.url.includes('/diktdb')) {
        // network first (put to dynamic cache), fallback to cache
        console.log('[ServiceWorker] ' + event.request.method + ' ' + event.request.url)
        event.respondWith(
            fetch(event.request)
                .then(fetchResponse => {
                    return caches.open(DYNAMIC_CACHE_NAME).then(cache => {
                        cache.put(event.request.url, fetchResponse.clone());
                        return fetchResponse;
                    })
                })
                .catch(() => {
                    return caches.match(event.request);
                })
        );
    }
    else {
        // Serving static pages from cache
        event.respondWith(caches.match(event.request));
    }

});