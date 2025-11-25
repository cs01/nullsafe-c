/**
 * LSP Bridge Worker - Runs clangd WASM and bridges LSP protocol to Monaco
 */

let isReady = false;
let messageId = 1;

// Handle messages from main thread
self.onmessage = async function(e) {
    const { type, ...data } = e.data;

    if (type === 'load') {
        try {
            console.log('[LSP Worker] Loading clangd from', data.scriptUrl);

            // Configure Module BEFORE loading clangd.js
            self.Module = {
                print: function(text) {
                    console.log('[clangd stdout]', text);
                    // Parse LSP responses from stdout
                    handleLSPOutput(text);
                },
                printErr: function(text) {
                    console.log('[clangd stderr]', text);
                },
                noInitialRun: true,
                wasmBinary: data.wasmBinary,
                postRun: [function() {
                    console.log('[LSP Worker] clangd initialized');
                    isReady = true;
                    self.postMessage({ type: 'ready' });

                    // Send LSP initialize request
                    sendLSPRequest('initialize', {
                        processId: null,
                        clientInfo: {
                            name: 'null-safe-playground',
                            version: '1.0.0'
                        },
                        rootUri: null,
                        capabilities: {
                            textDocument: {
                                publishDiagnostics: {},
                                completion: {},
                                hover: {}
                            }
                        }
                    });
                }]
            };

            // Load the Emscripten-generated JavaScript
            importScripts(data.scriptUrl);
            console.log('[LSP Worker] clangd.js loaded');

        } catch (error) {
            console.error('[LSP Worker] Failed to load clangd:', error);
            self.postMessage({
                type: 'error',
                error: `Failed to load clangd: ${error.message}`
            });
        }
    }
    else if (type === 'textDocument/didOpen') {
        const { uri, languageId, version, text } = data;
        sendLSPNotification('textDocument/didOpen', {
            textDocument: {
                uri: uri,
                languageId: languageId,
                version: version,
                text: text
            }
        });
    }
    else if (type === 'textDocument/didChange') {
        const { uri, version, contentChanges } = data;
        sendLSPNotification('textDocument/didChange', {
            textDocument: {
                uri: uri,
                version: version
            },
            contentChanges: contentChanges
        });
    }
    else if (type === 'textDocument/didClose') {
        const { uri } = data;
        sendLSPNotification('textDocument/didClose', {
            textDocument: {
                uri: uri
            }
        });
    }
};

// Buffer for incomplete LSP messages
let lspBuffer = '';

// Handle LSP output from clangd stdout
function handleLSPOutput(text) {
    lspBuffer += text;

    // LSP messages are separated by Content-Length headers
    while (true) {
        const headerMatch = lspBuffer.match(/Content-Length: (\d+)\r?\n\r?\n/);
        if (!headerMatch) break;

        const contentLength = parseInt(headerMatch[1]);
        const headerEnd = headerMatch.index + headerMatch[0].length;
        const messageEnd = headerEnd + contentLength;

        if (lspBuffer.length < messageEnd) break; // Incomplete message

        const messageJson = lspBuffer.substring(headerEnd, messageEnd);
        lspBuffer = lspBuffer.substring(messageEnd);

        try {
            const message = JSON.parse(messageJson);
            console.log('[LSP Worker] Received LSP message:', message);

            // Forward to main thread
            self.postMessage({
                type: 'lsp-message',
                message: message
            });
        } catch (e) {
            console.error('[LSP Worker] Failed to parse LSP message:', e, messageJson);
        }
    }
}

// Send LSP request to clangd
function sendLSPRequest(method, params) {
    const request = {
        jsonrpc: '2.0',
        id: messageId++,
        method: method,
        params: params
    };

    sendLSPMessage(request);
}

// Send LSP notification (no id) to clangd
function sendLSPNotification(method, params) {
    const notification = {
        jsonrpc: '2.0',
        method: method,
        params: params
    };

    sendLSPMessage(notification);
}

// Send LSP message to clangd stdin
function sendLSPMessage(message) {
    const jsonStr = JSON.stringify(message);
    const contentLength = jsonStr.length;
    const lspMessage = `Content-Length: ${contentLength}\r\n\r\n${jsonStr}`;

    console.log('[LSP Worker] Sending to clangd:', message.method || message.id);

    // Write to clangd's stdin using Module
    if (self.Module && self.Module.callMain) {
        // clangd reads from stdin, so we need to provide input
        // For now, we'll use a workaround - write to a virtual file that clangd monitors
        // This is a simplified approach; real LSP would use stdin/stdout pipes
        console.warn('[LSP Worker] LSP communication not yet fully implemented');
    }
}
