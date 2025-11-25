/**
 * LSP Bridge Worker - Runs clangd WASM and bridges LSP protocol to Monaco
 */

let clangdModule = null;
let isInitialized = false;
let messageId = 1;

// Virtual filesystem for C files
const virtualFS = new Map();

// Initialize clangd WASM module
async function initClangd(scriptUrl, wasmBinary) {
    return new Promise((resolve, reject) => {
        // Load the Emscripten module
        importScripts(scriptUrl);

        // Configure the Module
        self.Module = {
            wasmBinary: wasmBinary,
            noInitialRun: true,
            print: (text) => {
                // clangd writes LSP responses to stdout
                handleLSPResponse(text);
            },
            printErr: (text) => {
                console.error('[clangd]', text);
            },
            onRuntimeInitialized: () => {
                console.log('[LSP Worker] clangd WASM initialized');

                // Start clangd in LSP mode
                const args = ['clangd', '--log=error'];

                try {
                    Module.callMain(args);
                    resolve();
                } catch (e) {
                    reject(e);
                }
            }
        };
    });
}

// Handle LSP JSON-RPC responses from clangd
function handleLSPResponse(jsonText) {
    try {
        const message = JSON.parse(jsonText);

        // Forward LSP message to main thread
        postMessage({
            type: 'lsp-response',
            message: message
        });

    } catch (e) {
        // Not JSON, probably a log message
        console.log('[clangd]', jsonText);
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

    const jsonStr = JSON.stringify(request);
    const contentLength = jsonStr.length;

    // LSP uses Content-Length header protocol
    const message = `Content-Length: ${contentLength}\r\n\r\n${jsonStr}`;

    // Write to clangd's stdin
    if (clangdModule && clangdModule.FS) {
        const buffer = new TextEncoder().encode(message);
        clangdModule.FS.writeFile('/dev/stdin', buffer);
    }
}

// Handle messages from main thread
onmessage = async function(e) {
    const { type, data } = e.data;

    if (type === 'init') {
        // Initialize clangd WASM
        const { scriptUrl, wasmBinary } = data;

        try {
            clangdModule = await initClangd(scriptUrl, wasmBinary);
            isInitialized = true;

            // Send initialize request
            sendLSPRequest('initialize', {
                processId: null,
                clientInfo: {
                    name: 'null-safe-playground',
                    version: '1.0.0'
                },
                rootUri: 'file:///',
                capabilities: {
                    textDocument: {
                        publishDiagnostics: {}
                    }
                }
            });

            postMessage({ type: 'ready' });

        } catch (error) {
            postMessage({
                type: 'error',
                error: error.message
            });
        }
    }

    else if (type === 'textDocument/didOpen') {
        // Notify clangd that a document was opened
        const { uri, languageId, version, text } = data;

        sendLSPRequest('textDocument/didOpen', {
            textDocument: {
                uri: uri,
                languageId: languageId,
                version: version,
                text: text
            }
        });
    }

    else if (type === 'textDocument/didChange') {
        // Notify clangd of changes
        const { uri, version, contentChanges } = data;

        sendLSPRequest('textDocument/didChange', {
            textDocument: {
                uri: uri,
                version: version
            },
            contentChanges: contentChanges
        });
    }

    else if (type === 'textDocument/didClose') {
        // Notify clangd that document was closed
        const { uri } = data;

        sendLSPRequest('textDocument/didClose', {
            textDocument: {
                uri: uri
            }
        });
    }
};
