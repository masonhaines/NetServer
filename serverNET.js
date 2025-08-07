

const { ClientRequest } = require('http');
const net = require('net');

// Storage for connected clients
const clientsArray = [];



// Create a TCP server called server
const server = net.createServer((socket) => {
    console.log('Client connected');


    // create a client ID based off of the sockets remote address and the remote port
    const clientID = `${socket.remoteAddress}:${socket.remotePort}`;
    console.log(`Client ID: ${clientID}`);

    // set encoding to utf8 to handle strings instead of buffers
    // socket is the object representing the connection to the client
    socket.setEncoding('utf8');

    // push client to the clients array 
    // send a welcome message to the client
    clientsArray.push(socket);
    // socket.write("Hello and welcome to my server homie!\n"); 
    socket.write(JSON.stringify({ 
        type: 'welcome',
        message: 'Hello and welcome to my server homie!',
        clientID
    }) + '\n');


    // Buffer for incoming data or a queue to hold incoming data
    // This is used to handle cases where data arrives in chunks
    // and we need to wait for a complete message before processing it
    let pendingData = '';
    
    // Handle data
    socket.on('data', (data) => {
        // Add the new data to our buffer/ pending data
        // essentially we are collecting packets and waiting for a complete message and as 
        // we collect packets we APPEND them to the pendingData string
        pendingData += data.toString();

        // boundary is the spot where we find a newline character
        let boundary = pendingData.indexOf('\n'); 
        while (boundary !== -1) {
            // Extract the complete message
            //Gets the first full message
            const message = pendingData.substring(0, boundary); // (start: number, end?: number essentially the newline): string
            // deletes the message from the pendingData string
            pendingData = pendingData.substring(boundary + 1);

            // Process the message
            try {
                const parsedMessage = JSON.parse(message); // turns the string into a JSON object
                // console.log('Received message:', parsedMessage);
                
                // Handle different message types
                switch (parsedMessage.type) { // gets the type from message
                case 'greeting': // if the type is greeting from the client reply with json that is type welcome
                    socket.write(JSON.stringify({
                    type: 'welcome',
                    message: `Hello, ${parsedMessage.name}!`,
                    timestamp: Date.now()
                    }) + '\n');
                    break;
                    
                case 'query':
                    socket.write(JSON.stringify({
                    type: 'response',
                    queryId: parsedMessage.queryId,
                    result: handleQuery(parsedMessage.query),
                    timestamp: Date.now()
                    }) + '\n');
                    break;

                    // 
                case 'chat':
                    socket.write(JSON.stringify({
                    type: 'chat',
                    sender: clientID,
                    message: parsedMessage.message,
                    timestamp: Date.now()
                    }, socket) + '\n'); // send message to all clients on socket
                    break;
                    
                default:
                    socket.write(JSON.stringify({
                    type: 'error',
                    message: 'Unknown message type',
                    timestamp: Date.now()
                    }) + '\n');
                }
            } catch (err) {
                console.error('Error processing message:', err);
                socket.write(JSON.stringify({
                type: 'error',
                message: 'Invalid JSON format',
                timestamp: Date.now()
                }) + '\n');
            }
            
            // Look for the next message
            boundary = pendingData.indexOf('\n');
        }
    });



    // Broadcast message to all clients except the sender---------------------------------------------------------------------------------- needs to fixewd to be json
    function broadcast(message, sender) {
        // clientSocketElement is just the parameter name for each element in the array
        // clientsArray.forEach(clientSocketElement => {
        //     if (clientSocketElement !== sender) {
        //         clientSocketElement.write(message);
        //     }
        // });

        // Convert message to JSON and append newline
        const json = JSON.stringify(message) + '\n';
        clientsArray.forEach(clientSocketElement => {
            if (clientSocketElement !== sender) {
                clientSocketElement.write(json);
            }
        });
    }


    // notify all the clients about new connection
    // broadcast(`New client connected: ${clientID}\n`, socket);
    broadcast({
        type: 'notification',
        message: `New client connected: ${clientID}`
    }, socket);

    // handle data received from the client
    // socket.on('data', (data) => {
    //     // console.log('trim Received:', data.trim()); // this is a console log to see the data received from the client

    //     // Broadcast the message to all other clients
    //     broadcast(`${clientID}: ${data}`, socket);
    // });

    // handle client disconnection
    socket.on('end', () => {
        console.log('-----------Client disconnected-----------');
        const index = clientsArray.indexOf(socket);
        if (index !== -1) {
            clientsArray.splice(index, 1); // removes an element from the array 
        }
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

server.listen(7777, () => {
    console.log('Im a net - Server and I am a good listener, listening on port 7777');
});

process.on('SIGINT', () => {
    console.log('Shutting down server... am I dying?');


    // remove all clients from the clients array ie from the server 
    clientsArray.forEach(client => {
        client.end();
    });

    server.close(() => {
        console.log('Server closed/dead');
        process.exit(0);
    });
});

