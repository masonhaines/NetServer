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

// Create interface for reading from the terminal
const rl = readline.createInterface({
  input: process.stdin,
  output: process.stdout
});

// Create a client connection
const client = net.createConnection({ port: 7777 }, () => {
  console.log('Connected to chat server');
  console.log('Type a message and press Enter to send');
  
  // Start reading user input
  rl.prompt();
});

// Set encoding
client.setEncoding('utf8');

// Handle data from server
client.on('data', (data) => {
  // Move cursor to beginning of line and clear it
  process.stdout.write('\r\x1b[K');
  
  // Print the server message
  console.log(data.trim());
  
  // Re-display the prompt
  rl.prompt();
});

// Handle connection end
client.on('end', () => {
  console.log('Disconnected from server');
  rl.close();
  process.exit(0);
});

// Handle errors
client.on('error', (err) => {
  console.error('Connection error:', err);
  rl.close();
  process.exit(1);
});

// Handle user input
rl.on('line', (input) => {
  // Send the user input to the server
  client.write(input);
  rl.prompt();
});

// Close the connection when the user exits
rl.on('close', () => {
  console.log('Exiting chat...');
  client.end();
});