const { spawn } = require('child_process');
const WebSocket = require('ws');
const fs = require('fs');

const wss = new WebSocket.Server({ port: 8080 });

console.log("Starting Bridge...");

// Spawn the C Emulator
// Adjust path if necessary. currently expecting compiled binary in same dir or reachable.
const emulator = spawn('./tesser_tower.exe', [], { cwd: './' }); 

console.log("Emulator process spawned. PID:", emulator.pid);

emulator.stdout.on('data', (data) => {
    const lines = data.toString().split('\n');
    lines.forEach(line => {
        const cleanLine = line.trim();
        if (cleanLine.startsWith('{') && cleanLine.endsWith('}')) {
            // Potential JSON
            try {
                // Validate JSON
                JSON.parse(cleanLine); 
                // Broadcast to all clients
                wss.clients.forEach(client => {
                    if (client.readyState === WebSocket.OPEN) {
                        client.send(cleanLine);
                    }
                });
                // console.log("Sent:", cleanLine);
            } catch (e) {
                console.error("Invalid JSON:", cleanLine);
            }
        } else {
             if (cleanLine) console.log("[EMU]", cleanLine);
        }
    });
});

emulator.stderr.on('data', (data) => {
    console.error(`[EMU ERR]: ${data}`);
});

emulator.on('close', (code) => {
    console.log(`Emulator process exited with code ${code}`);
});

wss.on('connection', ws => {
    console.log('Client connected');
    ws.send(JSON.stringify({msg: "Connected to Tesser Watchtower"}));
});
