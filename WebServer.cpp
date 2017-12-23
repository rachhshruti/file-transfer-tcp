#include<iostream>
#include<cstdlib>
#include <unistd.h>
#include<string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include<fstream>
#include<map>
#include <pthread.h>
#include<sstream>
#include <sys/stat.h>
using namespace std;

/**
 * The WebServer class is a multi-threaded server that is used to service 
 * multiple client requests. The client can request for either persistent
 * or non-persistent connections. 
 * @author Shruti Rachh
 *
 */
class WebServer
{
	private:
	int serverSockFileDesc,clientSockFileDesc;
	struct sockaddr_in serverAddress,clientAddress;	
	map<string,string> httpStatusCodes;
	char *msgContent;
	int fileSize;
	
	public:
	char buff[1024];
	void displayError(const char *errorMsg);
	void setHttpStatusCodes();
	void createSocket();
	void bindAddress(int portNumber);
	void listenConn();	
	int acceptConn();
	int readClientRequest(int clientSock);
	string* processRequest();
	int getFileSize(const char *filename);
	string getRequestedContent(string filename);	
	string setStatusLine(string statusCode,string httpVersion);	
	string* setResponseHeaders(string fileExt);	
	string createResponse(string statusCode,string httpVersion,string fileExt);
	int sendResponse(string response,int clientSock);
	void closeClientConnection();	
	void closeConnection();
};

/**
 * This method is used to display the error message.
 * @param
 * 	errorMsg: The error message that is to be displayed.
*/
void WebServer::displayError(const char *errorMsg)
{
	cerr<<"Error: "<<errorMsg<<endl;
	exit(1);
}

/**
 * This method is used to set the various HTTP status codes and 
 * their corresponding HTTP messages.
 */
void WebServer::setHttpStatusCodes()
{
	httpStatusCodes["200"]="OK";
	httpStatusCodes["404"]="Page Not Found";
	httpStatusCodes["400"]="Bad Request";	
}

/**
 * This method is used to create the server socket and sets the 
 * serverSockFileDesc class variable.
 */
void WebServer::createSocket()
{
	serverSockFileDesc=socket(AF_INET,SOCK_STREAM,0);
	if (serverSockFileDesc < 0)
  	{
		displayError("The server socket could not be opened!");
	}
}

/**
 * This method is used to bind the server socket to an address.
 * @param
 * 	portNumber: The port number on which the server will listen for incoming connections.
 */
void WebServer::bindAddress(int portNumber)
{
	bzero((char *) &serverAddress, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(portNumber);     	
	serverAddress.sin_addr.s_addr = INADDR_ANY;
     	
	if (bind(serverSockFileDesc, (struct sockaddr *) &serverAddress,sizeof(serverAddress)) < 0) 
  	{      
		displayError("There is some problem while binding the server socket to an address!");
	}
}

/**
 * In this method, server listens for incoming client connections and defines the queue size 
 * for the number of client connections that can wait while server is handling 
 * other connections. The maximum size is 5.
 */
void WebServer::listenConn()
{
	listen(serverSockFileDesc,5);
}

/**
 * This method is used to accept the client connection.
 * @return
 *	client socket file descriptor
 */
int WebServer::acceptConn()
{
	socklen_t clientAddrLength=sizeof(clientAddress);
  	clientSockFileDesc=accept(serverSockFileDesc,(struct sockaddr *) &clientAddress,&clientAddrLength);
	return clientSockFileDesc;
}

/**
 * This method reads the client request into a buffer using the client socket.
 * @param
 *	clientSock: This is the client socket file descriptor.
 * @return
 *	the number of characters read
 */
int WebServer::readClientRequest(int clientSock)
{
	bzero(buff,1024);
  	int noOfCharacters= read(clientSock,buff,1023);
  	if (noOfCharacters < 0) 
	{
		displayError("There is problem while reading from socket!");
	}
	return noOfCharacters;	
}

/**
 * This method is used to process the client request.
 * @return
 *	a string array consisting of the file name, HTTP version and connection type. 
 */
string* WebServer::processRequest()
{
	char *filename;
	string httpVersion;
	string request=string(buff);
	string httpVersionStr;
	string statusCode;
	size_t connectionTypeStartPos=request.find("Connection: ");
	size_t connectionTypeEndPos=request.find("\r\n");
	string connectionType=request.substr(connectionTypeStartPos,connectionTypeEndPos);
	char *req=strtok(buff,"/");
	if(req)
	{
		req=strtok(NULL," ");
		if(req)
		{
			filename=req;
			req=strtok(NULL,"\n");
			if(req)
			{
				httpVersionStr=string(req);
				httpVersion=httpVersionStr.substr(4,(httpVersionStr.size()-5));	
			}
		}	
	}
	if(strcmp(httpVersionStr.c_str(),"HTTP/1.1")==0 || strcmp(request.substr(0,3).c_str(),"GET")==0)
	{
		statusCode="200";
	}else
	{
		statusCode="400";
	}
	string* values=new string[4];
	values[0]="files/"+string(filename);
	values[1]=httpVersion;
	values[2]=connectionType;
	values[3]=statusCode;
	return values;
}

/**
 * Gets the size of the requested file.
 * @param filename
 * 	the filename whose size is to be calculated.
 * @return the file size
 */
int WebServer::getFileSize(const char *filename)
{
    ifstream file;
    file.open(filename, ios_base::binary);
    file.seekg(0,ios_base::end);
    int size = file.tellg();
    file.close();
    return size;
}

/**
 * This method gets the requested file contents.
 * It sets the status code to either 200 or 404 depending 
 * on whether the file is present or not on the server's 
 * directory.
 * @param
 * 	filename: The name of the file requested.
 * @return 
 * 	the HTTP status code
 */
string WebServer::getRequestedContent(string filename)
{
	int iteration=0;
	string statusCode;
	ifstream readFile;
	readFile.open(filename.c_str());
	if(readFile.is_open())
	{
		fileSize=getFileSize(filename.c_str());
		msgContent=new char[fileSize];
		bzero(msgContent,fileSize);
		int iteration=0;
		while(!readFile.eof())
		{
			readFile.get(msgContent[iteration++]);
		}
		readFile.close();
		statusCode="200";
	}
	else
	{
		statusCode="404";
	}
	
	return statusCode;
}

/**
 * This method is used by the createResponse method in order to set the
 * status line of the response.
 * @param
 *	statusCode: The HTTP status code of the response.
 * @param
 * 	httpVersion: The HTTP version of the response.
 * @return
 *	the status line string
 */
string WebServer::setStatusLine(string statusCode,string httpVersion)
{
	string statusLine="HTTP"+httpVersion+" "+statusCode+" "+httpStatusCodes[statusCode];
	return statusLine;
}

/**
 * This method is used by the createResponse method in order to set the
 * headers of the response.
 * @param
 *	fileExt: The file extension used to determine whether the content is of text/html or not.
 * @return
 * 	the response headers
 */
string* WebServer::setResponseHeaders(string fileExt)
{
	string* headers=new string[2];
	int len=0;
	if(msgContent!=NULL)
	{
		len=strlen(msgContent);
	}
	stringstream contentLen;
	contentLen<<len;
	headers[0]="Content-Length: "+contentLen.str();
	if(fileExt==".html")
	{
		headers[1]="Content-Type: text/html";
	}
	else
	{
		headers[1]="Content-Type: text/plain";
	}	
	return headers;
}

/**
 * This method is used to create the complete HTTP response.
 * @param
 *	statusCode: The HTTP status code of the response.
 * @param
 * 	httpVersion: The HTTP version of the response.
 * @param
 *	fileExt: The file extension used to determine whether the content is of text/html or not.
 * @return
 *	the HTTP response
 */
string WebServer::createResponse(string statusCode,string httpVersion,string fileExt)
{		
	string response;	
	string statusLine=setStatusLine(statusCode,httpVersion);
	string* headers=setResponseHeaders(fileExt);
	if(statusCode=="200")
	{
		response=statusLine+"\r\n"+headers[0]+"\r\n"+headers[1]+"\r\n\r\n";
		for(int i=0;i<fileSize;i++)
		{
			response+=msgContent[i];
		}
	}else if(statusCode=="404")
	{
		if(fileExt==".html")
		{		
			response=statusLine+"\r\n"+headers[1]+"\r\n\r\n<html>404 Page Not Found</html>";
		}
		else
		{
			response=statusLine+"\r\n"+headers[1]+"\r\n\r\n404 File Not Found";
			cout<<"Response: "<<response<<endl;
		}
	}else
	{
		response=statusLine+"\r\n"+headers[1]+"\r\n\r\n400 Bad Request";
	}				
	return response;		
}

/**
 * This method is used to send the response to the client.
 * @param
 *	response: the response to be sent
 * @param
 * 	clientSock: the client socket
 * @return
 *	the number of characters written
 */
int WebServer::sendResponse(string response,int clientSock)
{
	int noOfCharacters = write(clientSock,(const void *)response.c_str(),response.length());     	
	if (noOfCharacters < 0)
	{ 
		displayError("There is problem while writing to socket!");
	}
	cout<<"Number of bytes sent: "<<noOfCharacters<<endl;
	return noOfCharacters;
}

/**
 * This method closes the socket that accepts client connections after the client is been served.
 */
void WebServer::closeClientConnection()
{
	close(clientSockFileDesc);

}

/**
 * This method closes the server socket.
 */
void WebServer::closeConnection()
{
	close(serverSockFileDesc);

}

/**
 * This is a thread routine which will be called each time a new thread is created.
 * It is used to read each client requests and respond back to each client.
 * @param
 *	clientSocketDesc: the unique client socket which separates each client is used for read and write. 
 */
void* handleClientConn(void *clientSocketDesc)
{
	WebServer server;
	int sock = *(int*)clientSocketDesc;
  	bool flag=true;  	
	int no=server.readClientRequest(sock);
    string* values=server.processRequest();
	server.setHttpStatusCodes();	
	while(values[2].find("keep-alive")!=string::npos)
  	{
    	size_t dotPos=values[0].find(".");
    	string fileExt=values[0].substr(dotPos);
		
		string statusCode;	
		if(strcmp(values[3].c_str(),"400")!=0)
		{    		
			statusCode=server.getRequestedContent(values[0]);
		}else
		{	
			statusCode=values[3];
		}		
    	string response=server.createResponse(statusCode,values[1],fileExt);
    	int noOfChar=server.sendResponse(response,sock);
		no=server.readClientRequest(sock);
    	values=server.processRequest();	 
  	}
	if(values[2].find("close")!=string::npos)
	{
		size_t dotPos=values[0].find(".");
    	string fileExt=values[0].substr(dotPos);
		string statusCode;
			
		if(strcmp(values[3].c_str(),"400")!=0)
		{ 		
			statusCode=server.getRequestedContent(values[0]);	
		}else
		{	
			statusCode=values[3];
		}		
    	string response=server.createResponse(statusCode,values[1],fileExt);
    	int noOfChar=server.sendResponse(response,sock);
	}
  	server.closeClientConnection();
}

int main(int noOfArguments,char *argumentList[])
{
	WebServer server;
  	pthread_t clientThread;
	/*
     * It checks if all the command-line arguments are provided.
     */	
	if(noOfArguments<2)
	{
		server.displayError("The client must provide a port number!");
	}
	server.createSocket();
	int portNo=atoi(argumentList[1]);	
	server.bindAddress(portNo);
	server.listenConn();
	int clientSocket;
	/*
   	 * It accepts client connections if any and creates a separate thread for each client connection.
   	 */
  	while(1)
  	{
		clientSocket=server.acceptConn();
	  	int *tmpClientSock=&clientSocket;
    	if(pthread_create( &clientThread , NULL , handleClientConn , (void*) tmpClientSock )< 0)
    	{
      		server.displayError("The thread could not be created!");
    	}
    	pthread_detach(clientThread);
    	server.listenConn();
  	}
	if(clientSocket<0)
	{
		server.displayError("There is some problem in accepting connection!");
	}
	server.closeConnection();
	return 0;
}
