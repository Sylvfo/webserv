// DOM elements
const endpointInput = document.getElementById('endpoint');
const payloadInput = document.getElementById('payload');
const idInput = document.getElementById('id');
const out = document.getElementById('output');
const status = document.getElementById('status');
const fileInput = document.getElementById('fileInput');
const filesList = document.getElementById('filesList');

// Utility functions
function show(txt) {
    out.textContent = txt;
}

function setStatus(text) {
    status.textContent = text;
}

// API testing functions
async function doFetch(method) {
    const base = endpointInput.value.trim();
    if (!base) {
        show('Entrez un endpoint valide');
        return;
    }
    
    let url = base;
    const opts = {
        method,
        headers: { 'Accept': 'application/json' }
    };
    
    if (method === 'POST') {
        try {
            opts.headers['Content-Type'] = 'application/json';
            opts.body = JSON.stringify(JSON.parse(payloadInput.value));
        } catch (e) {
            show('Payload JSON invalide');
            return;
        }
    }
    
    if (method === 'DELETE') {
        const id = idInput.value.trim();
        if (!id) {
            show('Entrez un ID pour DELETE');
            return;
        }
        if (!url.endsWith('/')) url += '/';
        url += encodeURIComponent(id);
    }
    
    setStatus(`statut: en cours (${method})`);
    
    try {
        const res = await fetch(url, opts);
        const text = await res.text();
        let pretty = text;
        try {
            const j = JSON.parse(text);
            pretty = JSON.stringify(j, null, 2);
        } catch (e) {}
        
        show(`${res.status} ${res.statusText}\n\n${pretty}`);
    } catch (err) {
        show('Erreur réseau : ' + err.message);
    }
    
    setStatus('statut: inactif');
}

// File upload functions
async function uploadFiles() {
    const files = fileInput.files;
    if (!files.length) {
        show('Aucun fichier sélectionné');
        return;
    }
    
    setStatus('statut: upload en cours');
    
    for (let i = 0; i < files.length; i++) {
        const file = files[i];
        const formData = new FormData();
        formData.append('file', file);
        
        try {
            const response = await fetch('/cgi-bin/upload.cgi', {
                method: 'POST',
                body: formData
            });
            
            const result = await response.text();
            show(`Upload ${file.name}: ${response.status} ${response.statusText}\n${result}`);
        } catch (err) {
            show(`Erreur upload ${file.name}: ${err.message}`);
        }
    }
    
    setStatus('statut: inactif');
    // Refresh file list after upload
    listFiles();
}

// File management functions
async function listFiles() {
    setStatus('statut: chargement des fichiers');
    
    try {
        const response = await fetch('/cgi-bin/list_files.cgi');
        const result = await response.text();
        
        if (response.ok) {
            try {
                const files = JSON.parse(result);
                displayFilesList(files);
            } catch (e) {
                // If not JSON, display as text
                filesList.innerHTML = `<pre>${result}</pre>`;
            }
        } else {
            filesList.innerHTML = `<div class="file-item">Erreur: ${response.status}</div>`;
        }
    } catch (err) {
        filesList.innerHTML = `<div class="file-item">Erreur réseau: ${err.message}</div>`;
    }
    
    setStatus('statut: inactif');
}

function displayFilesList(files) {
    if (!files || files.length === 0) {
        filesList.innerHTML = '<div class="file-item">Aucun fichier trouvé</div>';
        return;
    }
    
    const html = files.map(file => `
        <div class="file-item">
            <div style="flex: 1;">
                    <div class="file-name">${escapeHtml(file.name)}</div>
                <div class="file-size">${formatFileSize(file.size)}</div>
            </div>
            <button class="btn-delete" onclick="deleteFile('${escapeHtml(file.name).replace(/'/g, "\\'")}')">
                Supprimer
            </button>
        </div>
    `).join('');
    
    filesList.innerHTML = html;
}

async function deleteFile(filename) {
    if (!confirm(`Êtes-vous sûr de vouloir supprimer ${filename} ?`)) {
        return;
    }
    
    setStatus('statut: suppression en cours');
    
    try {
        const response = await fetch('/cgi-bin/delete_file.cgi', {
            method: 'POST',
            headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
            body: `filename=${encodeURIComponent(filename)}`
        });
        
        const result = await response.text();
        show(`Suppression ${filename}: ${response.status} ${response.statusText}\n${result}`);
        
        if (response.ok) {
            // Refresh file list after deletion
            listFiles();
        }
    } catch (err) {
        show(`Erreur suppression ${filename}: ${err.message}`);
    }
    
    setStatus('statut: inactif');
}

// Utility function to format file size
function formatFileSize(bytes) {
    if (bytes === 0) return '0 Bytes';
    const k = 1024;
    const sizes = ['Bytes', 'KB', 'MB', 'GB'];
    const i = Math.floor(Math.log(bytes) / Math.log(k));
    return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
}

// Utility function to escape HTML characters
function escapeHtml(text) {
    const div = document.createElement('div');
    div.textContent = text;
    return div.innerHTML;
}

// Event listeners
document.getElementById('btnGet').addEventListener('click', () => doFetch('GET'));
document.getElementById('btnPost').addEventListener('click', () => doFetch('POST'));
document.getElementById('btnDelete').addEventListener('click', () => doFetch('DELETE'));
document.getElementById('btnUpload').addEventListener('click', uploadFiles);
document.getElementById('btnListFiles').addEventListener('click', listFiles);

// Load file list on page load
document.addEventListener('DOMContentLoaded', listFiles);
