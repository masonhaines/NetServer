// Fill out your copyright notice in the Description page of Project Settings.


#include "MyTcpController.h"
#include "Engine/Engine.h"
// #include "Serialization/MemoryReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "Interfaces/IPv4/IPv4Endpoint.h"
#include "SocketTypes.h"

// Sets default values
AMyTcpController::AMyTcpController()
{
	ServerMessage = "";
	ClientMessage = "";
 	CurrentAddress = nullptr;
    ClientSocket = nullptr; // init Client socket pointer to nullptr
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AMyTcpController::BeginPlay()
{
	Super::BeginPlay();
	// this is if I would like to create a server instance of the game and 
	// if(GetNetMode() == NM_DedicatedServer) // runs only if a dedicated server build target
	// {
	// 	FIPv4Address IpAddress;
	// 	FIPv4Address::Parse(FString("0.0.0.0"), IpAddress);
	//
	// 	FIPv4Endpoint Endpoint(IpAddress, (uint16)2424);
	//
	// 	ServerListener = MakeUnique<FTcpListener>(Endpoint);
	//
	// 	ServerListener->OnConnectionAccepted().BindUObject(this, &ATCPController::ClientConnected);
	// }
}


// delegate callback for newly connected clients when Unreal itself is the TCP server.
// bool ATCPController::ClientConnected(FSocket* Socket, const FIPv4Endpoint& FIPV4Endpoint)
// {
// 	GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Red, "ClientConnected");
// 	
// 	UE_LOG(LogTemp, Display, TEXT("Client connected"));
// 	return true;
// }


void AMyTcpController::Connect(FString ServerHostingIP)
{
	// check if socket is already open so there are not a million open sockets loose for connection
	// if socket already created and socket is not connected, disconnect and make a new one. no harm no foul
	Disconnect();
	
	FIPv4Address IpAddress; //
	FIPv4Address::Parse(ServerHostingIP, IpAddress); // Converts a string to an IPv4 address. So taking the server ip added inside of BP and making it a IPv4 address
	FIPv4Endpoint Endpoint(IpAddress, (uint16)7777); // create end point that will connect at the following port number. Creates and initializes a new IPv4 endpoint with the specified NetID and port.

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
	
	GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Green, "Socket Created", false, FVector2D(1.0f, 1.0f) );
	UE_LOG(LogTemp, Display, TEXT("Socket Created"));

	if(ClientSocket->Wait(ESocketWaitConditions::WaitForWrite, FTimespan::FromSeconds(4)))
	{
		ClientSocket->SetNoDelay(true);
		
		GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Green, "Socket Connected after the wait for write", false, FVector2D(1.0f, 1.0f) );
		GetClientConnectionStatus();
	}else
	{
		GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Green, "Socket Connection timed out", false, FVector2D(1.0f, 1.0f) );
		Disconnect();
		GetClientConnectionStatus();
	}
	// CurrentAddress = InternetAddress;
	
}

void AMyTcpController::Disconnect()
{
	if (ClientSocket != nullptr)
	{
		UE_LOG(LogTemp, Display, TEXT("Socket closed"));
		GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Red, "Socket closed", false, FVector2D(1.5f, 1.5f));
		ClientSocket->Close();
		// Get the singleton socket subsystem for the given named subsystem, then Cleans up a socket class
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ClientSocket); 
		ClientSocket = nullptr; // remove any possible dangle
	}
}

void AMyTcpController::GetClientConnectionStatus()
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
	
	UE_LOG(LogTemp, Display, TEXT("Connection Status: %s"), *connectionStatus);
	GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Yellow, FString::Printf(TEXT("Socket State: %s"), *connectionStatus), false, FVector2D(2.0f, 1.5f));
}

void AMyTcpController::sendMessage(FString Message)
{
	
}

// Called every frame
void AMyTcpController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
    // GetClientConnectionStatus(); // for testing mainly
	
	if (!ClientSocket || ClientSocket->GetConnectionState() != SCS_Connected)
	{
		return;
	}
	
	uint32 bufferSize = 0;
	while(ClientSocket->HasPendingData(bufferSize)) // Queries the socket to determine if there is pending data on the queue and saves it in variable ref
	{
		TArray<uint8> Bytes;
		Bytes.SetNumUninitialized(bufferSize);

		int32 bytesRead = 0;
		// Reads a chunk of data from a connected socket
		// A return value of 'true' does not necessarily mean that data was returned.
		// Callers must check the 'bytesRead' parameter for the actual amount of data returned.
		// A value of zero indicates that there was no data available for reading.
		if(ClientSocket->Recv(
			Bytes.GetData(), // received data is written into the Bytes variable
			bufferSize,
			bytesRead))
		{

			// convert UTF8 to TCHAR
			// FString RawTCPChunk = FString(StringCast<TCHAR>(reinterpret_cast<const char*>(Bytes.GetData()), bytesRead).Get()); // this is where the data is actually piling up for use 
			// PartialJsonMessage += RawTCPChunk;
			PartialJsonBytes.Append(Bytes.GetData(), bytesRead);

			while(true)
			{
				int32 IndexOfNewLine = -1;
				for (int32 i = 0; i < PartialJsonBytes.Num(); i++)
				{
					if(PartialJsonBytes[i] == '\n')
					{
						IndexOfNewLine = i;
						break;
					}
				}
				if (IndexOfNewLine == -1)
				{
					break;
				}
				
				FString CompleteJsonBytes = FString(StringCast<TCHAR>(reinterpret_cast<const UTF8CHAR*>(PartialJsonBytes.GetData()), IndexOfNewLine).Get());

				PartialJsonBytes.RemoveAt(0, IndexOfNewLine + 1);
				if (!CompleteJsonBytes.IsEmpty())
				{
					MessageQueue.Enqueue(FString(CompleteJsonBytes));
				}
			}

			
			
			// // create a way for a delimiter to separate the multiple json objects being sent 
			// int32 indexOfFoundNewLine;
			// while (PartialJsonMessage.FindChar('\n', indexOfFoundNewLine)) // will not run if false, there is no '\n' found 
			// {
			// 	GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Blue, FString::Printf(TEXT("I made it inside of the first while loop ")));
			//
			// 	FString completePartialJsonString = PartialJsonMessage.Left(indexOfFoundNewLine);
			//
			// 	// if (!completePartialJsonString.StartsWith("{"))
			// 	// {
			// 	// 	int32 pos;
			// 	// 	if (completePartialJsonString.FindChar('{', pos))
			// 	// 	{
			// 	// 		completePartialJsonString = completePartialJsonString.RightChop(pos);
			// 	// 	}
			// 	// }
			// 	
			// 	// completePartialJsonString.TrimEndInline(); // remove ending \ or whitespace that is lingering
			// 	if (completePartialJsonString.EndsWith("\r"))
			// 	{
			// 		completePartialJsonString.RemoveAt(completePartialJsonString.Len() - 1);
			// 	}
			//
			// 	if (!completePartialJsonString.IsEmpty())
			// 	{
			// 		TSharedPtr<FJsonObject> TestForValidJsonObject;
			// 		const auto TestReading = TJsonReaderFactory<>::Create(completePartialJsonString);
			//
			// 		if(FJsonSerializer::Deserialize(TestReading, TestForValidJsonObject))
			// 		{
			// 			MessageQueue.Enqueue(completePartialJsonString);// ----------------------make sure all packets have arrived, hopefully, maybe?
			// 			PartialJsonMessage.RemoveAt(0, indexOfFoundNewLine + 1);
			//
			// 		}else
			// 		{
			// 			GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Blue, FString::Printf(TEXT("I broke after the test parse ")));
			// 			UE_LOG(LogTemp, Warning, TEXT("Test parse failed (waiting for more data). Len=%d Text='%s'"),completePartialJsonString.Len(), *completePartialJsonString);
			// 			GEngine->AddOnScreenDebugMessage(
			// 				-1, // key (-1 = always add new)
			// 				10.f, // time on screen
			// 				FColor::Red, // color
			// 				FString::Printf(TEXT("Test parse failed. Len=%d Text='%s'"),
			// 					completePartialJsonString.Len(),
			// 					*completePartialJsonString)
			// 			);
			// 			break;
			// 		}
			// 	}
			// 	else
			// 	{
			// 		GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Blue, FString::Printf(TEXT("the string was empty and i didnt see anything so i cleared the partial json message ")));
			// 		PartialJsonMessage.RemoveAt(0, indexOfFoundNewLine + 1); // if the partial json message is empty just clean it out 
			// 	}
			// }

			// GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Blue, FString::Printf(TEXT("I made it OUTSIDE of the first while loop ")));

			
			FString CompleteJsonObject;
			while(MessageQueue.Dequeue(CompleteJsonObject)) // returns false once queue is empty 
			{
				if (!CompleteJsonObject.EndsWith("}") || !CompleteJsonObject.StartsWith("{"))
				{
					// Trim 1 char from the end until it looks valid
					while (!CompleteJsonObject.IsEmpty() &&
						   (!CompleteJsonObject.EndsWith("}") || !CompleteJsonObject.StartsWith("{")))
					{
						CompleteJsonObject.RemoveAt(CompleteJsonObject.Len() - 1);
					}
				}

				UE_LOG(LogTemp, Warning, TEXT("Dequeued JSON: '%s'"), *CompleteJsonObject);
				GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Yellow,
					FString::Printf(TEXT("Dequeued JSON: %s"), *CompleteJsonObject));

				
				TSharedPtr<FJsonObject> JsonObject; // this the new json object that is instantiated after the parsing fromm deserialize is done
				const auto Reader = TJsonReaderFactory<>::Create(CompleteJsonObject);
				// https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Json/Serialization/FJsonSerializer/Deserialize/3
				if (FJsonSerializer::Deserialize(Reader, JsonObject)) // parse the json string received from the server
				{
					FString JsonMessage, JsonType, JsonSender;
					JsonObject->TryGetStringField(TEXT("message"), JsonMessage);
					JsonObject->TryGetStringField(TEXT("type"), JsonType);
					JsonObject->TryGetStringField(TEXT("sender"), JsonSender);

					if (JsonType == "serverMessage")
					{
					
						GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Blue, FString::Printf(TEXT("Message from server: %s"), *JsonMessage));
						UE_LOG(LogTemp, Display, TEXT("Connection Status: %s"), *JsonMessage);
					}
					else if (JsonType == "chat")
					{
						ClientMessage = JsonMessage;
					}

					sender = JsonSender;
				}
				else
				{

					GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Blue, FString::Printf(TEXT("Was unable to parse")));
					// PartialJsonMessage = CompleteJsonObject + PartialJsonMessage;
					break;
				}
			}
		}
		else
		{
			break;
		}

		// if(bytesRead <= 0) break;
	}
}

// has pending data -> allocate buffer -> receive -> convert to f string
// https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Json/Serialization?utm_source=chatgpt.com
// https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Json/Serialization/TJsonReader
// https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Json/Serialization/TJsonWriter
// https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Json/Serialization/FJsonSerializer
// https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/JsonUtilities/FJsonObjectConverter?utm_source=chatgpt.com
// https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Sockets/FSocket/Recv?utm_source=chatgpt.com