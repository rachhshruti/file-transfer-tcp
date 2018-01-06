# Web server and client implementation using TCP

Developed a multi-threaded web server which accepts persistent and non-persistent file request connections from web client as well as from the browser.

# Running the code

## Creating and configuring virtual machines
	
	vagrant up
   
   This will boot both the server and client machines

## SSH into virtual machines
	
	vagrant ssh tcpserver
	vagrant ssh tcpclient
   
## Compile the code
	
	make

## Run the WebServer:

	./Server port_number(any number between 1025 and 65535)

## Run the WebClient:

### For persistent connections:
	
	./Client server_IP/hostname server_portNo p ListOfFiles.txt
	
ListOfFiles.txt: contains the list of files to be requested, one on each line

### For non-persistent connections:
	
	./Client server_IP/hostname server_portNo np filename

## Screenshot:

<img src="https://github.com/rachhshruti/file-transfer-tcp/blob/master/images/file-transfer-tcp-output.png" width="1000" height="600" align="center"/>
