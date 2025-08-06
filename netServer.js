/** Mason Haines*/

const { ClientRequest } = require('http');
const net = require('net');

// Storage for connected clients
const clientsArray = [];



// Create a TCP server called server
const server = net.createServer((socket) => {
    console.log('Client connected');


    // Buffer for incoming data
    let buffer = '';
    
    // Handle data
    socket.on('data', (data) => {
        // Add the new data to our buffer
        buffer += data.toString();
        
        // Process complete messages
        let boundary = buffer.indexOf('\n');
        while (boundary !== -1) {
        // Extract the complete message
        const message = buffer.substring(0, boundary);
        buffer = buffer.substring(boundary + 1);
        
        // Process the message
        try {
            const parsedMessage = JSON.parse(message);
            console.log('Received message:', parsedMessage);
            
            // Handle different message types
            switch (parsedMessage.type) {
            case 'greeting':
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
        boundary = buffer.indexOf('\n');
        }
    });

    // create a client ID based off of the sockets remote address and the remote port
    const clientID = `${socket.remoteAddress}:${socket.remotePort}`;
    console.log(`Client ID: ${clientID}`);

    // set encoding to utf8 to handle strings instead of buffers
    // socket is the object representing the connection to the client
    socket.setEncoding('utf8');

    // push client to the clients array 
    // send a welcome message to the client
    clientsArray.push(socket);
    socket.write("Hello and welcome to my server homie!\n"); 

    // Broadcast message to all clients except the sender
    function broadcast(message, sender) {
        // clientSocketElement is just the parameter name for each element in the array
        clientsArray.forEach(clientSocketElement => {
            if (clientSocketElement !== sender) {
                clientSocketElement.write(message);
            }
        });
    }

    // notify all the clients about new connection
    broadcast(`New client connected: ${clientID}\n`, socket);


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

    // handle data received from the client
    socket.on('data', (data) => {
        console.log('trim Received:', data.trim());

        // Broadcast the message to all other clients
        broadcast(`${clientID}: ${data}`, socket);
    });

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

server.listen(7777, () => {
    console.log('Im a net - Server and I am a good listener, listening on port 7777');
});

process.on('SIGINT', () => {
    console.log('Shutting down server... am I dying?');
    server.close(() => {
        console.log('Server closed/dead');
        process.exit(0);
    });
});

