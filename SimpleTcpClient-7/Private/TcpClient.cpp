#include "TcpClient.h"

#include "Common/TcpSocketBuilder.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "Interfaces/IPv4/IPv4Endpoint.h"

UTcpClient::UTcpClient()
{
	ServerMessage = "";
	ClientMessage = "";
	Sender = "";
	CawfeData = "";
	UnrealEngineMessagingAlias = "";
	ClientSocket = nullptr; // init Client socket pointer to nullptr
	bIsConnected = false;
}

UTcpClient* UTcpClient::CreateTcpClient(UObject* WorldContextObject)
{
	return NewObject<UTcpClient>(WorldContextObject, TEXT("UTcpClient"));
}

void UTcpClient::Connect(FString ServerHostingIP, int ServerPort)
{
	// check if socket is already open so there are not a million open sockets loose for connection
	// if socket already created and socket is not connected, disconnect and make a new one. no harm no foul
	Disconnect();

	uint16 port = ServerPort;
	FIPv4Address IpAddress; //
	FIPv4Address::Parse(ServerHostingIP, IpAddress); // Converts a string to an IPv4 address. So taking the server ip added inside of BP and making it a IPv4 address
	FIPv4Endpoint Endpoint(IpAddress, port); // create end point that will connect at the following port number. Creates and initializes a new IPv4 endpoint with the specified NetID and port.

	// Build a TCP socket for the client connection.
	// - Name: "Client Socket"
	// - AsReusable: allow reusing the socket
	// - WithReceiveBufferSize: set custom buffer size
	// - AsNonBlocking: makes operations asynchronous
	ClientSocket = FTcpSocketBuilder(TEXT("Client Socket"))
	.AsReusable()
	.WithReceiveBufferSize(BufferSize)
	// .AsBlocking(); // returns after connection has been verified. but if the server never responds it will sit on this forever....
	.AsNonBlocking(); // returns immediately and is meant for non async 
	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM); // retrieve windows socket
	// const ESocketErrors Err = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLastErrorCode(); // was recommended but not quite sure where the error codes come from and not useful enough documentation to currently use 
	

	// https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Sockets/ISocketSubsystem
	TSharedPtr<FInternetAddr> InternetAddress = SocketSubsystem->CreateInternetAddr(); // create internet address object to hold the servers IP and port 
	// TSharedPtr<FInternetAddr> InternetAddress = SocketSubsystem->GetHostByName

	// give internet address the IP and port values 
	InternetAddress->SetIp(Endpoint.Address.Value);
	InternetAddress->SetPort(Endpoint.Port);

	
	// Places the socket into a state to listen for incoming connections.
	// Connects a socket to a network byte ordered address.
	// Params:
	// MaxBacklog — The number of connections to queue before refusing them.
	// Returns:
	// true if successful, false otherwise.
	ClientSocket->Connect(*InternetAddress); // this returns true, but there is a possibility to receive the errors on the socket
	
	// GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Green, "Socket Created", false, FVector2D(1.0f, 1.0f) );
	// UE_LOG(LogTemp, Display, TEXT("Socket Created"));

	if(ClientSocket->Wait(ESocketWaitConditions::WaitForWrite, FTimespan::FromSeconds(4)))
	{
		ClientSocket->SetNoDelay(true);
		
		// GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Green, "Socket Connected after the wait for write", false, FVector2D(1.0f, 1.0f) );
		GetClientConnectionStatus();
		bIsConnected = true;
	}else
	{
		// GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Green, "Socket Connection timed out", false, FVector2D(1.0f, 1.0f) );
		Disconnect();
		GetClientConnectionStatus();
	}
	
}

void UTcpClient::Disconnect()
{
	if (ClientSocket != nullptr)
	{
		// UE_LOG(LogTemp, Display, TEXT("Socket closed"));
		// GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Red, "Socket closed", false, FVector2D(1.5f, 1.5f));
		ClientSocket->Close();
		// Get the singleton socket subsystem for the given named subsystem, then Cleans up a socket class
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ClientSocket); 
		ClientSocket = nullptr; // remove any possible dangle
	}

	bIsConnected = false;
}


void UTcpClient::StartPollAsynchronously()
{
	// https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Core/FGenericPlatformProcess
	// https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Core/EAsyncExecution
	if(!bIsConnected || !ClientSocket)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not start polling"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Is going to start polling for file data"));
	if(this)
	{
		Async(EAsyncExecution::Thread, [this]()
		{
			while(bIsConnected)
			{

				if(ClientSocket)
				{
					PollSocket();
					FPlatformProcess::Sleep(0.01); // pause operations inside while loop for .1 second so other ops can run on thread
					// FGenericPlatformProcess::Sleep() // why does this not work? this makes no sense
				}

			}
				
			if(!bIsConnected) 	UE_LOG(LogTemp, Warning, TEXT("Stopped polling "));

		});
	}


	
}

void UTcpClient::GetClientConnectionStatus()
{
	FString connectionStatus;
	
	if (ClientSocket != nullptr)
	{
		// ESocketConnectionState state = ClientSocket->GetConnectionState();
		// SCS_NotConnected,
		// SCS_Connected,
		// /** Indicates that the end point refused the connection or couldn't be reached */
		// SCS_ConnectionError
		
		switch(ClientSocket->GetConnectionState()) // Determines the connection state of the socket.
		{
		case SCS_NotConnected:
			connectionStatus = TEXT("Not Connected");
			break;
		case SCS_Connected:
			connectionStatus = TEXT("Connected");
			break;
		case SCS_ConnectionError:
			connectionStatus = TEXT("Connection Error");
			break;
		default:
			connectionStatus = TEXT("Error, ClientSocket should be connected");
			break;
		}
	}
	
	// UE_LOG(LogTemp, Display, TEXT("Connection Status: %s"), *connectionStatus);
	GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Yellow, FString::Printf(TEXT("Socket State: %s"), *connectionStatus), false, FVector2D(2.0f, 1.5f));
}


void UTcpClient::SendMessage(FString Message, FString Type, FString RequestID)
{
	if (!ClientSocket || ClientSocket->GetConnectionState() != SCS_Connected)
	{
		return;
	}
	
	// MakeShared utility function. Allocates a new ObjectType and reference controller in a single memory block. Equivalent to std::make_shared.
	// TSharedPtr<FJsonObject> MyJsonObject = MakeShared<FJsonObject>();
	TSharedPtr<FJsonObject> MyJsonObject = MakeShareable(new FJsonObject());

	
	if (Type == TEXT("updateName"))
	{
		MyJsonObject->SetStringField("name", Message);
		UnrealEngineMessagingAlias = Message;
	}
	else if (Type == TEXT("chat"))
	{
		MyJsonObject->SetStringField("message", Message);
	}
	else if (Type == TEXT("request"))
	{
		MyJsonObject->SetStringField("requestID", RequestID);
		MyJsonObject->SetStringField("filename", Message);
	}

	MyJsonObject->SetStringField("type", Type);


	// convert to string  https://github.com/KellanM/OpenAI-Api-Unreal/blob/dd840428d34aa247e43e34b2a94e1605eb1ff69a/Source/OpenAIAPI/Private/OpenAICallChat.cpp#L108
	FString MessageStringToSend;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<TCHAR>::Create(&MessageStringToSend); // why cant i just pass by reference normally?
	FJsonSerializer::Serialize(MyJsonObject.ToSharedRef(), Writer); // returns true if serialize worked 

	// Code snippet from https://dev.epicgames.com/documentation/en-us/unreal-engine/character-encoding-in-unreal-engine?
	// https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Core/Containers/FTCHARToUTF8_Convert
	// FString String;
	

	// UE_LOG(LogTemp, Warning, TEXT("JSON object being sent: '%s'"), *MessageStringToSend); // for debug

	MessageStringToSend.AppendChar('\n'); // append new line as a delimiter for the end of the json message
	// FTCHARToANSI Convert(*String);
	// Ar->Serialize((ANSICHAR*)Convert.Get(), Convert.Length());
	// // FTCHARToANSI::Length() returns the number of bytes for the encoded string, excluding the null terminator.
	FTCHARToUTF8 Convert(*MessageStringToSend);
	
	int32 BytesSent = 0;
	ClientSocket->Send(
		(uint8*)Convert.Get(), 
		Convert.Length(), 
		BytesSent
	);
}

void UTcpClient::RequestData()
{
	if (!ClientSocket || ClientSocket->GetConnectionState() != SCS_Connected)
	{
		return;
	}
	
	// MakeShared utility function. Allocates a new ObjectType and reference controller in a single memory block. Equivalent to std::make_shared.
	TSharedPtr<FJsonObject> MyJsonObject = MakeShareable(new FJsonObject());

	MyJsonObject->SetStringField("type", "request");

	// convert to string  https://github.com/KellanM/OpenAI-Api-Unreal/blob/dd840428d34aa247e43e34b2a94e1605eb1ff69a/Source/OpenAIAPI/Private/OpenAICallChat.cpp#L108
	FString MessageStringToSend;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<TCHAR>::Create(&MessageStringToSend); // why cant i just pass by reference normally?
	FJsonSerializer::Serialize(MyJsonObject.ToSharedRef(), Writer); // returns true if serialize worked 

	UE_LOG(LogTemp, Warning, TEXT("JSON object being sent: '%s'"), *MessageStringToSend); // for debug

	MessageStringToSend.AppendChar('\n'); // append new line as a delimiter for the end of the json message
	// FTCHARToANSI Convert(*String);
	// Ar->Serialize((ANSICHAR*)Convert.Get(), Convert.Length());
	// // FTCHARToANSI::Length() returns the number of bytes for the encoded string, excluding the null terminator.
	FTCHARToUTF8 Convert(*MessageStringToSend);
	
	int32 BytesSent = 0;
	ClientSocket->Send(
		(uint8*)Convert.Get(), 
		Convert.Length(), 
		BytesSent
	);
}



TArray<FString> UTcpClient::ReadStringFromFile(FString FilePath, bool& bWasReadSuccessful)
{

	if(!FPlatformFileManager::Get().GetPlatformFile().FileExists(*FilePath))
	{
		bWasReadSuccessful = false;
		// UE_LOG(LogTemp, Warning, TEXT("File does not exist: %s"), *FilePath);
		// GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Yellow, FString::Printf(TEXT("File does not exist: %s"), *FilePath), false, FVector2D(1.5f, 1.5f));
		return {};
	}

	TArray<FString> ReturnString = {};
	FString FirstLine = ReturnString[0];

	// Try to read the file. Output then is stored in return string
	if(!FFileHelper::LoadFileToStringArray(ReturnString, *FilePath)){
		bWasReadSuccessful = false;
		// UE_LOG(LogTemp, Log, TEXT("File Content: %s"), *FirstLine);
		// GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Cyan, FString::Printf(TEXT("File Content: %s"), *FirstLine), false, FVector2D(1.5f, 1.5f));
		return {};
	}

	bWasReadSuccessful = true;
	// UE_LOG(LogTemp, Log, TEXT("read was successful: %s"), *FirstLine);
	// GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Green, FString::Printf(TEXT("read was successful: %s"), *FirstLine), false, FVector2D(1.5f, 1.5f));
	return ReturnString;
}

void UTcpClient::PollSocket()
{
	// GetClientConnectionStatus(); // for testing mainly

	// if the socket is not open and the socket is not connected please return
	if (!ClientSocket || ClientSocket->GetConnectionState() != SCS_Connected)
	{
		return;
	}

	bIsConnected = true;
	uint32 PendingDataSize = 0;
	// Fsocket - Queries the socket to determine if there is pending data on the queue and saves it in variable ref
	while(ClientSocket->HasPendingData(PendingDataSize)) 
	{
		TArray<uint8> ReceivedChunkOfBytes; // make array for bytes that incoming to pool 
		ReceivedChunkOfBytes.SetNumUninitialized(PendingDataSize); // resize the ReceivedChunkOfBytes array to the size of the pending data coming through the socket

		int32 NumberOfBytesRead = 0;
		// Reads a chunk of data from a connected socket, Number of bytes is set from function call, true maybe also return with 0 == NumberOfBytesRead
		/*received data is written into the Bytes variable*/
		bool bIsReadSuccessful = ClientSocket->Recv(ReceivedChunkOfBytes.GetData() , PendingDataSize,NumberOfBytesRead);
		if(!(bIsReadSuccessful && NumberOfBytesRead > 0))
		{
			break;
		}
		
		// combine all the packets that have been received. 
		PartialJsonBytes.Append(ReceivedChunkOfBytes.GetData(), NumberOfBytesRead);

        const int32 DelimiterLength = 5;
		const int32 MaxBufferBytes = 8 * 1024 * 1024;// "<END>"

		// inside your receive block, right after appending:
		if (PartialJsonBytes.Num() > MaxBufferBytes) {
			UE_LOG(LogTemp, Error, TEXT("Framing overflow. Reset buffer."));
			PartialJsonBytes.Reset();
		}
		
        while(true)
        {
            // Check for the start of the delimiter
            if (!PartialJsonBytes.Contains((uint8)'<') || PartialJsonBytes.Num() < DelimiterLength) {
                break; // no chance the delimiter is present yet
            }
            
            const int32 limit = PartialJsonBytes.Num() - DelimiterLength;
            int32 IndexOfDelimiter = INDEX_NONE;

            // Begin searching through all the bytes, essentially through the {...}<END>
            for (int32 i = 0; i <= limit; ++i)
            {
                // skip until '<'
                if (PartialJsonBytes[i] != (uint8)'<') break;

                // verify for  "<END>"
                if (PartialJsonBytes[i] == (uint8)'<' && PartialJsonBytes[i+1] == (uint8)'E' && PartialJsonBytes[i+2] == (uint8)'N'
                    && PartialJsonBytes[i+3] == (uint8)'D' && PartialJsonBytes[i+4] == (uint8)'>')
                {
                    IndexOfDelimiter = i;
                    break;
                }
            }
            if (IndexOfDelimiter == INDEX_NONE)
            {
                break;
            }
            
            int32 messagelength = IndexOfDelimiter;
            if (messagelength <= 0)
            {
                // UE_LOG(LogTemp, Warning, TEXT("Delimiter found at start, no message to process."));
                // just remove the delim and continue
                PartialJsonBytes.RemoveAt(0, DelimiterLength);
                continue;
            }
            
            FString JsonStringFromBytes = FString(StringCast<TCHAR>
                (reinterpret_cast<const UTF8CHAR*>(PartialJsonBytes.GetData()), messagelength).Get());

            PartialJsonBytes.RemoveAt(0, messagelength + DelimiterLength);
			
			TSharedPtr<FJsonObject> ParsedJsonObject; // this the new json object that is instantiated after the parsing fromm deserialize is done
			const auto Reader = TJsonReaderFactory<>::Create(JsonStringFromBytes); // builds a JSON reader from string for parsing

			if (!FJsonSerializer::Deserialize(Reader, ParsedJsonObject) || !ParsedJsonObject.IsValid())
			{
				UE_LOG(LogTemp, Error, TEXT("Bad JSON frame. Dropping."));
				continue;
			}

			FString JsonMessageField, JsonTypeField, JsonSenderField, JsonDataField, JsonFilenameField;
			// These fields can be thought of as the keys for the json object
			// once the values of these are retrieved they are stored in the FString variables above
			// then they are checked for what the data was sent for that field
			// then the data is stored in the public FString variables ClientMessage and Sender for use in blueprints
			// if more fields are added to the json object on the server side they can be added here
			// and then checked for what the data is for and stored in new public FString variables
			ParsedJsonObject->TryGetStringField(TEXT("message"), JsonMessageField);
			ParsedJsonObject->TryGetStringField(TEXT("type"), JsonTypeField);
			ParsedJsonObject->TryGetStringField(TEXT("sender"), JsonSenderField);
			ParsedJsonObject->TryGetStringField(TEXT("data"), JsonDataField);
			ParsedJsonObject->TryGetStringField(TEXT("filename"), JsonFilenameField);
			// data: fileData,
			// filename: fileName,	

			if (JsonTypeField == "chat")
			{
				ClientMessage = JsonMessageField;
			}
			else if (JsonTypeField == "serverMessage") // this is largely for me and can be commented out later. Should be ***
			{
			
				// GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Blue, FString::Printf(TEXT("Message from server: %s"), *JsonMessageField));
				// UE_LOG(LogTemp, Display, TEXT("Connection Status: %s"), *JsonMessageField);
			}
			else if (JsonTypeField == TEXT("CawfeData"))
            {
                
				CawfeData = JsonDataField;
				FileName = JsonFilenameField;
				CawfeData.ReplaceInline(TEXT("\r"), TEXT(""));

				const FString CaptureCawfeData = CawfeData;
				const FString CaptureFilename = FileName;
				AsyncTask(ENamedThreads::GameThread, [this, CaptureFilename, CaptureCawfeData]()
				{
					AddFileEntry(CaptureFilename, CaptureCawfeData);
				});
            }

			Sender = JsonSenderField;
            
        } // end while loop
		
	}
}



void UTcpClient::AddFileEntry(const FString& Filename, const FString& Data)
{
	check(IsInGameThread());
	FileInformation TempFileInformation{Filename, Data};
	// TempFileInformation.SFileName = Filename;
	// TempFileInformation.SData = Data;
	ArrayOfReceivedFiles.Add(TempFileInformation);
}

bool UTcpClient::GetFileData(const FString& TargetFileName, FString& OutData)
{

	for (int32 i = 0; i < ArrayOfReceivedFiles.Num(); i++)
	{
		if (ArrayOfReceivedFiles[i].SFileName == TargetFileName)
		{
			OutData = ArrayOfReceivedFiles[i].SData;
			ArrayOfReceivedFiles.RemoveAt(i);
			// UE_LOG(LogTemp, Display, TEXT("FileData: %s -> %s"), *OutData, *OutData);
			return true;
		}
	}
	
	return false;
}

int UTcpClient::GetReceivedFileNames(TArray<FString>& OutFileNames)
{
	OutFileNames.Reset();
	if (ArrayOfReceivedFiles.IsEmpty()) return 0;

	for (int32 i = 0; i < ArrayOfReceivedFiles.Num(); i++)
	{
		OutFileNames.Add(ArrayOfReceivedFiles[i].SFileName);
	}
	
	return 	ArrayOfReceivedFiles.Num();

}



void UTcpClient::RunAsyncwrapper()
{
	// https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Core/ENamedThreads__Type
	// Create a dedicated thread in the background to run the async events
	// AsyncTask(ENamedThreads::GameThread, [this]()
	Async(EAsyncExecution::Thread, [this]()
	{
		// inside of the background thread, run the first event on the game thread
		AsyncTask(ENamedThreads::GameThread, [this]()
		{
			OnAsyncEvent.Broadcast();
		});

		FPlatformProcess::Sleep(.05f); 

		// after a short pause, run the completed event on the game thread
		AsyncTask(ENamedThreads::GameThread, [this]()
		{
			OnAsyncEventCompleted.Broadcast();
		});
	});
}

// has pending data -> allocate buffer -> receive -> convert to f string
// https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Json/Serialization
// https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Json/Serialization/TJsonReader
// https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Json/Serialization/TJsonWriter
// https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Json/Serialization/FJsonSerializer
// https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/JsonUtilities/FJsonObjectConverter
// https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Sockets/FSocket/Recv