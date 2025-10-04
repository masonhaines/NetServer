const net = require('net');
const readline = require('readline');

let clientID = 'NoFace-Username';


// Setup readline to get input from terminal
const readLine = readline.createInterface({
  input: process.stdin,
  output: process.stdout,
  prompt: '> '
});

// Connect to the server with the new client object 
// const client = net.createConnection({host: '100.120.166.48', port: 7777 }, () => {
  const client = net.createConnection({port: 7777 }, () => {

  console.log('Connected to server');
  console.log('Type a message and press Enter to send');
  console.log('--------------------------------------------------------------------------------------------------------------------')
  readLine.prompt();

  readLine.question("Please enter your username: ", (username) => {
    clientID = username;

    client.write(JSON.stringify({
      type: 'updateName',
      name: username
    }) + '\n')

  })

});


client.setEncoding('utf8');
let pendingData = '';

//////////////////////////////////---------------------------GET
// Handle incoming data from server
client.on('data', (data) => {

  pendingData += data.toString();
  let boundary;

  while ((boundary = pendingData.indexOf('\n')) !== -1) {

    // message is the data send from the server
    const message = pendingData.substring(0, boundary);
    pendingData = pendingData.substring(boundary + 1);

    try {

      // create a parsed variable set it equal to the parsed data from the server
      const parsed = JSON.parse(message);
      readLine.setPrompt('');

      // if the parsed
      if (parsed.type === 'welcome') {
        clientID = parsed.clientID;
        console.log(`[Server] ${parsed.message}`);

      } else if (parsed.type === 'chat') {

        // display the parsed message that was broadcasted from the server 
        // let SplitUsername = parsed.sender.split(':::') // this removes all the information that the server needs, this should really by done on the server end...... like saving the name // yah i  just fixed it in server side 
        // let SenderUserName = SplitUsername[0];
        console.log(`[${parsed.sender}] ${parsed.message}`);
      }
      else if (parsed.type === 'CawfeData') {
        console.log(`\n[Server CawfeData Update] ${parsed.message}`);
        console.log(parsed.data);
      }

    } catch (err) {
      console.error('\n[Error] Failed to parse JSON:', message);
    }

    readLine.setPrompt(`> `);
    readLine.prompt();
    
  }
});


////////////////////////////////////------------------------------------------------------------- POST
// handle out going data to the server
readLine.on('line', (input) => {
  const message = {
    type: 'chat',
    sender: clientID, // or username
    message: input.trim(),
    timestamp: Date.now()
  };

  process.stdout.moveCursor(0, -1); // Move cursor up one line 

  client.write(JSON.stringify(message) + '\n'); // send message to the server ie to the other clients 
  
  console.log(`> [${clientID}] ${message.message}`);
  readLine.setPrompt(`> `);
  readLine.prompt();
});


// shutdown, without closing the server
client.on('end', () => {
  console.log('\nDisconnected from server');
  readLine.close();
});


// error for instance when the server disconnects from the client
client.on('error', (err) => {
  console.error('Connection error:', err);
  readLine.close();
});


// closing statement
readLine.on('close', () => {
  console.log('Exiting...');
  client.end();
});
