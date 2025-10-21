// Mason Haines 8/05/2025

// const { ClientRequest } = require('http');
const net = require('net');
const fs = require('fs');
const path = require("path");
const { EOF } = require('dns');
// const { clear } = require('console');

// Storage for connected clients
const clientsArray = [];
const pendingRequests = new Map();



const portNumber = 7777;

// Broadcast message to all clients except the sender
function broadcast(message, sender) {

    // clientSocketElement is just the parameter name for each element in the array
    // Convert message to JSON and append newline
    const json = JSON.stringify(message) + "<END>";
    // const json = JSON.stringify(message) + '\n';
    clientsArray.forEach(clientSocketElement => {
        if (clientSocketElement !== sender) {
            clientSocketElement.write(json, 'utf8' );
        }
    });
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
    socket.setEncoding('utf8');

    // push client to the clients array 
    clientsArray.push(socket);


    // Buffer for incoming data or a queue to hold incoming data
    // This is used to handle cases where data arrives in chunks
    // and we need to wait for a complete message before processing it
    let pendingData = '';


    socket.write(JSON.stringify({
        type: 'serverMessage', 
        message: 'Hello from the server',
        timestamp: Date.now()

    }) + '<END>', 'utf8');
    
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
                            
                        
                        socket.write(JSON.stringify({
                        type: 'serverMessage',
                        message: `Hello, ${parsedMessage.name}! Welcome to my chat server homie!`,
                        timestamp: Date.now(),
                        clientID: clientID.get(socket).username
                        }) + '<END>', 'utf8');
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
                        socket.write(JSON.stringify({
                        type: 'error',
                        message: 'Unknown message type',
                        timestamp: Date.now()
                        }) + '<END>', 'utf8');
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

// Buffer is just a byte array -------------------------------------------- 
// length-prefixed frames 
// a frame is delim that is currently 4 bytes long that tells us how long the body is
// the body is made up of a header and a payload
// the header is a json object that tells us about the payload
// the payload is the actual data we want to send
// [frameLen][headerLen][header][payload]
function createFrame(headerObj, payloadBuf) {
    // 1) Encode header JSON to bytes
    const headerBuf = Buffer.from(JSON.stringify(headerObj), 'utf8'); // [header]

    // 2) Encode header length as 2 bytes (big-endian)
    const headerLenBuf = Buffer.alloc(2); // [headerLen]
    headerLenBuf.writeUInt16BE(headerBuf.length, 0);

    // 3) Frame body = [headerLen][header][payload]
    const frameBodyBuf = Buffer.concat([headerLenBuf, headerBuf, payloadBuf]); // add payloadBuf at the end

    // 4) Encode total body length as 4 bytes (big-endian)
    const frameLenBuf = Buffer.alloc(4); // [frameLen]
    frameLenBuf.writeUInt32BE(frameBodyBuf.length, 0);

    // 5) Final frame = [frameLen][frameBody]
    return Buffer.concat([frameLenBuf, frameBodyBuf]);
}



async function WriteDataToSocket(filename) {
    
    return new Promise((resolve, reject) => {

        const readStream = fs.createReadStream(filename, {encoding: 'utf8', highWaterMark: 64 * 1024 }); // 64KB chunk size
        let sequenceNumber = 0;

        readStream.on('data', (chunk) => {
            const header = {filename, sequenceNumber: sequenceNumber++, EOF: false };
            let cleanChunk = chunk.replace(/\r/g, ''); // remove carriage return characters
            const frame = createFrame(header, Buffer.from(cleanChunk, 'utf8'));
            if (!socket.write(frame)) {
            readStream.pause();
            socket.once('drain', () => readStream.resume());
            }
        });

        readStream.once('end', () => {
            const header = {filename, sequenceNumber, EOF: true };
            const frame = createFrame(header, Buffer.alloc(0)); // header-only EOF
            socket.write(frame, (err) => err ? reject(err) : resolve());
        });


        readStream.on('error', (error) => {
            console.error('Error reading file:', error);
            reject(error);
            return;
        });

        resolve();

    });

}

// read files from the cawfeData folder every ** seconds and broadcast the data to all connected clients
// const folder = "C:\\Users\\demo\\servers\\NetServer\\cawfeData";
const folder = "E:\\servers\\NodeJS_Net\\NodeJS_NetServer\\data";


async function GiveRequestedFiles() {


    const files = await fs.promises.readdir(folder);
    
    for (let fileIndex = 0; fileIndex < files.length; fileIndex++) {
        const file = files[fileIndex];
        const absoluteFilePath = path.join(folder, file);
        console.log(`Sending file (${fileIndex + 1}/${files.length}): ${file}`);
        await WriteDataToSocket(absoluteFilePath);
    }

    // WriteDataToSocket(path.join(folder, files[0]));
}