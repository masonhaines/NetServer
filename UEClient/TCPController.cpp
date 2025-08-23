// #include "E:\Unreal Projects\TCP_DevEnv\MultiplayerShooter\Intermediate\Build\Win64\x64\MultiplayerShooterEditor\Development\UnrealEd\SharedPCH.UnrealEd.Project.ValApi.ValExpApi.Cpp20.InclOrderUnreal5_3.h"
#include "TCPController.h"
#include "Engine/Engine.h"


ATCPController::ATCPController()
{
	CurrentAddress = nullptr;
	ClientSocket = nullptr; // init Client socket pointer to nullptr
	PrimaryActorTick.bCanEverTick = true;
}

void ATCPController::BeginPlay()
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

void ATCPController::Connect(FString ServerHostingIP)
{
	
	FIPv4Address IpAddress; //
	FIPv4Address::Parse(ServerHostingIP, IpAddress); // Converts a string to an IPv4 address. So taking the server ip added inside of BP and making it a IPv4 address
	FIPv4Endpoint Endpoint(IpAddress, (uint16)2424); // create end point that will connect at the following port number 

	// Build a TCP socket for the client connection.
	// - Name: "Client Socket"
	// - AsReusable: allow reusing the socket
	// - WithReceiveBufferSize: set custom buffer size
	// - AsNonBlocking: makes operations asynchronous
	ClientSocket = FTcpSocketBuilder(TEXT("Client Socket"))
	.AsReusable()
	.WithReceiveBufferSize(BufferSize)
	.AsNonBlocking();
	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM); // retrieve windows socket

	TSharedPtr<FInternetAddr> InternetAddress = SocketSubsystem->CreateInternetAddr(); // create internet address object to hold the servers IP and port 

	// give internet address the IP and port values 
	InternetAddress->SetIp(Endpoint.Address.Value);
	InternetAddress->SetPort(Endpoint.Port);

	
	// Places the socket into a state to listen for incoming connections.
	// Connects a socket to a network byte ordered address.
	// Params:
	// MaxBacklog — The number of connections to queue before refusing them.
	// Returns:
	// true if successful, false otherwise.
	if (ClientSocket->Connect(*InternetAddress))
	{
		GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Green, "Socket Created", false, FVector2D(1.0f, 1.0f) );
		UE_LOG(LogTemp, Display, TEXT("Socket Created"));
		CurrentAddress = InternetAddress;
	} else
	{
		GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Red, "Socket could not be created", false, FVector2D(1.0f, 1.0f));
		UE_LOG(LogTemp, Display, TEXT("Socket could not be created"));
	}

	FString connectionStatus = GetClientConnectionStatus();
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, FString::Printf(TEXT("Socket State: %s"), *connectionStatus), false, FVector2D(1.0f, 1.5f));
}

void ATCPController::Disconnect()
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

FString ATCPController::GetClientConnectionStatus()
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
	GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Blue, FString::Printf(TEXT("Socket State: %s"), *connectionStatus));

	return connectionStatus;
}

void ATCPController::sendMessage(FString Message)
{
	
}

void ATCPController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// GetClientConnectionStatus(); // for testing mainly
	
	// if check is not fully necessary unless the build target is a client
	// if (GetNetMode() == NM_Client) // Get the network mode (dedicated server, client, standalone, etc) for this actor.
	// {
	
		TArray<uint8> Bytes;

		if (ClientSocket)
		{
			if (ClientSocket->GetConnectionState() == SCS_Connected)
			{
				uint32 pendingData = 0;
				if(ClientSocket->HasPendingData(pendingData)) // Queries the socket to determine if there is pending data on the queue and saves it in variable ref
				{
					Bytes.SetNum(pendingData);

					int32 bytesRead = 0;
					// Reads a chunk of data from a connected socket
					// A return value of 'true' does not necessarily mean that data was returned.
					// Callers must check the 'BytesRead' parameter for the actual amount of data returned.
					// A value of zero indicates that there was no data available for reading.
					if(ClientSocket->Recv(
						Bytes.GetData(), // received data is written into the Bytes variable
						pendingData,
						bytesRead))
					{
						FString serverMessage = "";
						FMemoryReader MemoryReader(Bytes);

						MemoryReader << serverMessage; // left shift for serialization

						GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Blue, FString::Printf(TEXT("Message from server: %s"), *serverMessage));
					}
				}
			}
		}
	// }
}


