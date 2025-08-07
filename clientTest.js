// const net = require('net');

// const client = net.createConnection({ port: 7777 }, () => {
//     console.log('Connected to server!');
//     client.write('Hello from client!');
// });

// client.setEncoding('utf8');

// //handle data recieved from the server
// client.on('data', (data) => {
//     console.log('Received from server:', data);

//     client.write('Thanks for the message, server!');
//     // client.end(); // close the connection after receiving data
// });

// client.on('end', () => {
//     console.log('Disconnected from server, server passed away :(');
// });

// // Handle errors
// client.on('error', (err) => {
//   console.error('Connection error:', err);
// });

const net = require('net');
const readline = require('readline');


let clientID = null;


// Setup readline to get input from terminal
const readLine = readline.createInterface({
  input: process.stdin,
  output: process.stdout,
  prompt: '> '
});

// Connect to the server
const client = net.createConnection({ port: 7777 }, () => {
  console.log('Connected to server');
  console.log('Type a message and press Enter to send');
  console.log('--------------------------------------------------------------------------------------------------------------------')
  readLine.prompt();
});

client.setEncoding('utf8');

let buffer = '';

// Handle incoming data from server
client.on('data', (data) => {
  buffer += data.toString();
  let boundary;
  while ((boundary = buffer.indexOf('\n')) !== -1) {
    const message = buffer.substring(0, boundary);
    buffer = buffer.substring(boundary + 1);

    try {

      const parsed = JSON.parse(message);

      if (parsed.type === 'welcome') {
        clientID = parsed.clientID;
        console.log(`[Server] ${parsed.message}`);

      } else if (parsed.type === 'chat') {

        console.log(`[${parsed.sender}] ${parsed.message}`);

      }


    } catch (err) {
      console.error('\n[Error] Failed to parse JSON:', message);
    }

    readLine.prompt();
  }
});

// Handle user input
// readLine.on('line', (input) => {
//   const message = {
//     type: 'chat',
//     text: input.trim()
//   };
//   client.write(JSON.stringify(message) + '\n');
//   readLine.prompt();
// });

readLine.on('line', (input) => {
  const message = {
    type: 'chat',
    sender: clientID, // or username
    message: input.trim(),
    timestamp: Date.now()
  };
  client.write(JSON.stringify(message) + '\n');
  readLine.prompt();
});

// Graceful shutdown
client.on('end', () => {
  console.log('\nDisconnected from server');
  readLine.close();
});

client.on('error', (err) => {
  console.error('Connection error:', err);
  readLine.close();
});

readLine.on('close', () => {
  console.log('Exiting...');
  client.end();
});
