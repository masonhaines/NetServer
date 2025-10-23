// Mason Haines 8/05/2025

// const { ClientRequest } = require('http');
const net = require('net');
const fs = require('fs');
const path = require("path");
// const { clear } = require('console');

// Storage for connected clients
const clientsArray = [];
const pendingRequests = new Map();



const portNumber = 7777;

function sendJson(socket, obj) {

    return new Promise((resolve, reject) => {

        const stringy = JSON.stringify(obj);
        if (stringy.includes("<END>")) {
            return reject(new Error("JSON contains <END>"));
            // throw new Error("JSON contains <END>");
        }
        const writeResult = socket.write(stringy + "<END>", "utf8", resolve);

        if (!writeResult) {
            socket.once('drain', () => {
                resolve();
            });
        }

    });
}

// Broadcast message to all clients except the sender
async function broadcast(message, sender) {

        // clientSocketElement is just the parameter name for each element in the array
        // Convert message to JSON and append newline
        try {
        
            const json = JSON.stringify(message) + "<END>";
            
            for (const clientSocketElement of clientsArray) {
                if (clientSocketElement !== sender) {
                    await sendJson(clientSocketElement, json);
                }
            }

        } catch (error) {
            console.error('Error broadcasting message:', error);
        }
}






// Create a TCP server called server and recieve data from the 'socket' ie the client 
const server = net.createServer((socket) => {
    console.log('Client connected');


    // create a client ID based off of the sockets remote address and the remote port
    // let clientID = `${socket.remoteAddress}:${socket.remotePort}`;
    const clientID = new Map(); // https://www.w3schools.com/js/js_maps.asp
    // console.log(`Client ID: ${clientID}`);

    // set encoding to utf8 to handle strings instead of buffers
    // socket is the object representing the connection to the client
    // socket.setEncoding('utf8');

    // push client to the clients array 
    clientsArray.push(socket);


    // Buffer for incoming data or a queue to hold incoming data
    // This is used to handle cases where data arrives in chunks
    // and we need to wait for a complete message before processing it
    let pendingData = '';


    sendJson(socket, {
        type: 'serverMessage', 
        message: 'Hello from the server',
        timestamp: Date.now()

    });
    
    // Handle data
    socket.on('data', (data) => {
        // Add the new data to our buffer/ pending data
        // essentially we are collecting packets and waiting for a complete message and as 
        // we collect packets we APPEND them to the pendingData string
        pendingData += data.toString();

        // boundary is the spot where we find a newline character
        let boundary = pendingData.indexOf('}\n'); 
        while (boundary !== -1) {
            // Extract the complete message
            //Gets the first full message
            const message = pendingData.substring(0, boundary + 1).trim(); 
            // deletes the message from the pendingData string
            pendingData = pendingData.substring(boundary + 2);

            // if (pendingData.endsWith('}')) {

            
                // Process the message
                try {
                    const parsedMessage = JSON.parse(message.trim()); // turns the string into a JSON object
                    // console.log('Received message:', parsedMessage);
                    
                    // Handle different message types
                    switch (parsedMessage.type) { 
                    case 'updateName': // if the type is greeting from the client reply with json that is type welcome

                        if(parsedMessage.name !== null) {

                            clientID.set(socket,
                                {username: `${parsedMessage.name}`, Ip: socket.remoteAddress, Port: socket.remotePort}
                            )
                            console.log(`Client ID: ${clientID.get(socket).username}`);
                            
                        } else if (parsedMessage.name === '') {

                            clientID.set(socket,
                                {username: `secretName`, Ip: socket.remoteAddress, Port: socket.remotePort}
                            )
                            console.log(`Client ID: ${clientID.get(socket).username}`);

                        } else {

                            clientID.set(socket,
                                {username: `NULLNAME`, Ip: socket.remoteAddress, Port: socket.remotePort}
                            )
                            console.log(`Client ID: ${clientID.get(socket).username}`);

                        }


                        sendJson(socket, {
                            type: 'serverMessage',
                            message: `Hello, ${parsedMessage.name}! Welcome to my chat server homie!`,
                            timestamp: Date.now(),
                            clientID: clientID.get(socket).username
                        });
                        break;
                        
                    case 'request': {
                        GiveRequestedFiles();
                        break;
                    }

                    case 'chat': {
                        const info = clientID.get(socket);
                        let senderName;
                        if (info && info.username) { // check to make sure client actually has a name 
                            senderName = info.username;
                        } else {
                            senderName = `NULLNAME`;
                        }
                        broadcast({
                            type: 'chat',
                            sender: senderName, // equiv to clientID.get(socket).username
                            message: parsedMessage.message,
                            timestamp: Date.now()
                        }, socket);
                        break;
                    }
                        
                    default:
                        sendJson(socket, {
                            type: 'error',
                            message: 'Unknown message type',
                            timestamp: Date.now()
                        });
                    }


                } catch (err) {
                    console.error('Error processing message:', err);
                    socket.write(JSON.stringify({
                    type: 'error',
                    message: 'Invalid JSON format',
                    timestamp: Date.now()
                    }) + '<END>', 'utf8');
                }
                
                // Look for the next message
                boundary = pendingData.indexOf('}\n');
            // }
           
        }

    });


    // notify all the clients about new connection
    // broadcast(`New client connected: ${clientID}\n`, socket);
    broadcast({
        type: 'notification',
        message: `New client connected: ${clientID}`
    }, socket);


    // handle client disconnection
    socket.on('end', () => {
        console.log('-----------Client disconnected-----------');
        const index = clientsArray.indexOf(socket);
        if (index !== -1) {
            clientsArray.splice(index, 1); // removes an element from the array 
        }

        // clearInterval(DataInterval);
    });

    socket.on('error', (err) => {
        if (err.code === 'ECONNRESET') {
            console.log('Client forcefully closed the connection');
        } else {
            console.error('Socket error:', err);
        }
    });
    


});

// Simple function to handle queries
function handleQuery(query) {
    if (query === 'time') {
        return { time: new Date().toISOString() };
        
    } else if (query === 'stats') {
        return {
            uptime: process.uptime(),
            memory: process.memoryUsage(),
            platform: process.platform
        };
    } else {
        return { error: 'Unknown query' };
    }
}

server.listen(portNumber, () => {
    console.log('Im a net - Server and I am a good listener, listening on port ' + portNumber);
});

process.on('SIGINT', () => {
    console.log('Shutting down server... am I dying?');


    // remove all clients from the clients array ie from the server 
    clientsArray.forEach(client => {
        client.end();
        console.log('client removed')
    });

    server.close(() => {
        console.log('Server closed/dead');
        process.exit(0);
    });

    console.log('I didnt want to close ')
    return;
});


let fileName = '';

function ReadDataFromFile(data) {
    return new Promise((resolve, reject) => {

        fs.readFile(data, 'utf8', async (error, fileData) => {
            if (error) {
                console.error('Error reading file:', error);
                reject(error);
                return;
            }

            
            const clean = fileData.replace(/\r/g, ''); 
            fileName = path.basename(data);
            // console.log(`Reading file: ${fileName} and here is the data : 
            //     ${clean}
            //     `);
            // console.log(`has read file: ${fileName} with ${ChunkFileData(clean)} chunks`);
            // rebuildFileFromChunks([clean]); // for testing just pass the whole file as one chunk
           
            await broadcast({
                type: 'CawfeData',
                // message: fileName,
                sender: 'Server',
                data: clean,
                filename: fileName,
                timestamp: Date.now()
            });

            resolve();
        });

        
    });
}






// read files from the cawfeData folder every ** seconds and broadcast the data to all connected clients
// const folder = "C:\\Users\\demo\\servers\\NetServer\\cawfeData";
const folder = "E:\\servers\\NodeJS_Net\\NodeJS_NetServer\\data";


async function GiveRequestedFiles() {

    try {
        const filesInFolder = await fs.promises.readdir(folder);

        for (const file of filesInFolder) {
            console.log(`Sending file: ${file}`);
            await ReadDataFromFile(path.join(folder, file));
            // ReadDataFromFile(path.join(folder, files[0])); // only send the second file in the directory for testing, it is the smallest file and most diverse
        }
        
    } catch (error) {
        console.error('Error reading directory:', error);
    }
   
}
// GiveRequestedFiles();  //// ----------------------------------------------- comment me out to stop sending files on server start ----------------------------------------------- //




// ************************************************************ this is for testing file chunking and rebuilding files from chunks ************************************************************//
function ChunkFileData(filedata) {
    // const chunkSize = 1024; // size of each chunk in bytes
    const chunkSize = 8192; // size of each chunk in bytes
    const chunks = [];
    for (let i = 0; i < filedata.length; i += chunkSize) {
        const chunk = filedata.slice(i, i + chunkSize);
        chunks.push(chunk);
    }
    return chunks.length;
}

function rebuildFileFromChunks(chunks) {
    const fileData = chunks.join('');
    console.log(`Rebuilt file data: 
        ${fileData}
        `); 
    // return fileData;
}


// ***********************************************************************************************************************//