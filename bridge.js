const { spawn } = require('child_process');
const WebSocket = require('ws');
const fs = require('fs');
const http = require('http');

// --- 1. HTTP Server for Frontend (Port 3000) ---
// Isso permite acessar http://localhost:3000 em vez de abrir arquivo local
const server = http.createServer((req, res) => {
    // Serve index.html for any request for simplicity
    fs.readFile('./index.html', (err, data) => {
        if (err) {
            fs.readFile('./vd/index.html', (err2, data2) => {
                if (err2) {
                    res.writeHead(404);
                    res.end('index.html not found in root or vd/');
                    return;
                }
                res.writeHead(200, { 'Content-Type': 'text/html' });
                res.end(data2);
            });
            return;
        }
        res.writeHead(200, { 'Content-Type': 'text/html' });
        res.end(data);
    });
});

server.listen(3000, () => {
    console.log('Frontend Web Server running at http://localhost:3000');
});

// --- 2. WebSocket Server for Telemetry (Port 8080) ---
const wss = new WebSocket.Server({ port: 8080 });

console.log("Starting Watchtower Engine...");

// --- 3. Spawn Emulator ---
// Executa o binário compilado
const emulator = spawn('./tesser_tower', [], { cwd: './' });

console.log("Emulator process spawned. PID:", emulator.pid);

emulator.stdout.on('data', (data) => {
    // Process stdout chunks
    const lines = data.toString().split('\n');
    lines.forEach(line => {
        const cleanLine = line.trim();
        if (cleanLine.startsWith('{') && cleanLine.endsWith('}')) {
            try {
                // Quick validation
                JSON.parse(cleanLine);
                // Broadcast to UI
                wss.clients.forEach(client => {
                    if (client.readyState === WebSocket.OPEN) {
                        client.send(cleanLine);
                    }
                });
            } catch (e) {
                // Ignore parsing errors (partial lines)
            }
        } else if (cleanLine.length > 0) {
            // Log non-JSON output to console for debug
            // console.log("[EMU]", cleanLine);
        }
    });
});

emulator.stderr.on('data', (data) => {
    console.error(`[EMU ERR]: ${data}`);
});

emulator.on('close', (code) => {
    console.log(`Emulator exited with code ${code}`);
    process.exit(0);
});
