#include<iostream>
#include<cstdlib>
#include <unistd.h>
#include<string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include<fstream>
#include <sstream>
#include <arpa/inet.h>
#include <sys/time.h> 
using namespace std;

/**
 * The WebClient class is used to send requests for files to the server. 
 * The client can request for either persistent
 * or non-persistent connections. 
 * @author Shruti Rachh
 *
 */
class WebClient
{
	private:
	int clientSockFileDesc;
	struct sockaddr_in serverAddress;
	char buff[1009630];
	struct hostent *server;

	public: 
	void displayError(const char *errorMsg);
	void createSocket();
	void getServerInfo(char* hostname);
	void setServerAddress(int portNo);
	void connectToServer();
	string* setRequestHeaders(string connectionType);
	string createRequest(string filename,string connectionType);
	int sendRequest(string request);
	int readResponse();
	void displayResponse(string response);
	void closeConnection();
};

/**
 * This method is used to display the error message.
 * @param
 * 	errorMsg: The error message that is to be displayed.
*/
void WebClient::displayError(const char *errorMsg)
{
	cerr<<"Error: "<<errorMsg<<endl;
	exit(1);
}

/**
 * This method is used to create the client socket and sets the 
 * clientSockFileDesc class variable.
 */
void WebClient::createSocket()
{
	clientSockFileDesc=socket(AF_INET,SOCK_STREAM,0);
	if (clientSockFileDesc < 0)
  	{
		displayError("The server socket could not be opened!");
	}
}

/**
 * This method is used to get the IP address of the server
 * based on the hostname provided by the client.
 * @param
 *	hostname: the server hostname
 */
void WebClient::getServerInfo(char* hostname)
{
	server=gethostbyname(hostname);
	if(server==NULL)
	{
		displayError("There is no such host!");
	}
}

/**
 * This method is used to set the server address.
 * @param
 *	portNo: the port number on which server is listening.
 */
void WebClient::setServerAddress(int portNo)
{
	bzero((char *) &serverAddress, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	bcopy((char *)server->h_addr,(char *)&serverAddress.sin_addr.s_addr,server->h_length);
	serverAddress.sin_port = htons(portNo);
}

/**
 * This method is used to connect to the server.
 */
void WebClient::connectToServer()
{
	if (connect(clientSockFileDesc,(sockaddr*)&serverAddress,sizeof(serverAddress)) < 0)
  	{
		displayError("Cannot connect to server!");
	}
}

/**
 * This method is used by the createRequest method in order to set the
 * headers of the request. 
 * @param
 *	connectionType: The connection type will be either close for non-persistent connection
 *			or keep-alive for persistent connection.
 * @return
 *	string array consisting of all the request headers
 */
string* WebClient::setRequestHeaders(string connectionType)
{
	string* headers=new string[5];
	char tmpServerAddr[32];
	const char* dottedDecimalStr=inet_ntop(AF_INET,(void *)&serverAddress.sin_addr.s_addr,tmpServerAddr,sizeof(tmpServerAddr));
	stringstream serverPort;
	serverPort<<ntohs(serverAddress.sin_port);	
	headers[0]="Host: "+string(dottedDecimalStr)+":"+serverPort.str()+"\r\n";
	headers[1]="Connection: "+connectionType+"\r\n";
	headers[2]="Accept: text/plain; charset='UTF-8'\r\n";
	headers[3]="Accept-Encoding: gzip, deflate, sdch\r\n";
	headers[4]="Accept-Language: en-US,en;q=0.8\r\n";
	return headers;
}

/**
 * This method is used to create the request.
 * @param
 *	filename: the name of the file that is to be requested.
 * @param
 *	connectionType: The connection type will be either close for non-persistent connection
 *			or keep-alive for persistent connection.
 * @return
 *	the request string to be sent
 */
string WebClient::createRequest(string filename,string connectionType)
{
	string* headers=setRequestHeaders(connectionType);
	string requestLine="GET /"+filename+" HTTP/1.1";
	string request=requestLine+"\r\n";
	
	for(int it=0;it<5;it++)
	{
		request=request+headers[it];
	}
	return request;
}

/**
 * This method is used to send the request to the server.
 * @param
 *	request: the request to be sent.
 * @return
 * 	the number of characters written.
 */
int WebClient::sendRequest(string request)
{
	bzero(buff,1009629);
  
	strncpy(buff,request.c_str(),1009628);
	int noOfCharacters=write(clientSockFileDesc,buff,strlen(buff));
  	if (noOfCharacters<0)
   	{
		displayError("There is problem while writing to socket!");
	}
	return noOfCharacters;
}

/**
 * This method reads the server response into a buffer.
 * @return 
 * 	the number of characters read.
 */
int WebClient::readResponse()
{
	bzero(buff,1009630);
	int no=read(clientSockFileDesc,buff,32000);	
	string resp=string(buff);
	displayResponse(resp);	
	string contentLengthHeader="Content-Length: ";	
	size_t contentLengthStartPos=resp.find(contentLengthHeader.c_str());
	size_t contentLengthEndPos=resp.find("\r\n");
	string contentLenStr=resp.substr(contentLengthStartPos+contentLengthHeader.length(),contentLengthEndPos);
	int contentLen=atoi(contentLenStr.c_str());	
	while(no<=contentLen)
	{
		bzero(buff,32000);
		no+=read(clientSockFileDesc,buff,32000);
		cout<<buff;
	}	
	cout<<"No of bytes read: "<<no<<endl;
	return no; 
}

/**
 * This method is used by readResponse to display the requested file content.
 * @param
 *	response: the response containing the header and the file content.
 */
void WebClient::displayResponse(string response)
{
	size_t fileContentStartPos=response.find("\r\n\r\n");
	string fileContent=response.substr(fileContentStartPos+4);
	cout<<fileContent<<endl;
}

/*
 * Structure for saving the number of lines in a file and the actual lines of the file
 */
struct fileContent
{
  int number;
  string lines[20];
};

/**
 * This method is used only in case client requests for persistent connection.
 * This method is used to read the file in which the list of the files to be requested
 * is present.
 * @param
 *	filename: the name of the file which contains the list of files.
 * @return
 *	fileContent structure which contains the number of lines and the actual lines of the file.
 */
fileContent getListOfFiles(string filename)
{
  
  fileContent fc;
  ifstream fileList(filename.c_str());
  string fileLine;
  int noOfLines=0;
  while(!fileList.eof() && getline(fileList,fileLine))
  { 
    if(fileLine=="")
    {
      break;
    }
    fc.lines[noOfLines++]=fileLine;
  }
  fc.number=noOfLines;
  fileList.close();
  return fc;
}

/**
 * This method closes the connection sockets.
 */
void WebClient::closeConnection()
{
	close(clientSockFileDesc);	
}

int main(int noOfArguments,char *argumentList[])
{
	WebClient client;
	string connectionType;
	/*
     * It checks if all the command-line arguments are provided.
     */	
	if(noOfArguments<5)
	{
		client.displayError("Invalid arguments!");
	}
	client.createSocket();
	int portNo=atoi(argumentList[2]);
	client.getServerInfo(argumentList[1]);
	client.setServerAddress(portNo);
	client.connectToServer();
	string filename="files/"+string(argumentList[4]);
	
	/*
   	 * Checks if the client has requested for persistent or non-persistent connection.
     */	
	if(strcmp(argumentList[3],"p")==0)
	{
		connectionType="keep-alive";
	  	fileContent fileList=getListOfFiles(filename);
	  	string fileLine;
	  	struct timeval t1, t2;
	  	double elapsedTime;
	  	gettimeofday(&t1, NULL);
	  	for(int i=0;i<fileList.number;i++)
	  	{	
	    	if(i==(fileList.number-1))
	    	{
	      		connectionType="close";
	    	}
	    	string request=client.createRequest(fileList.lines[i],connectionType);
	    	int noOfBytesSent=client.sendRequest(request);
	    	int noOfBytesRecvd=client.readResponse();
			cout<<"No of bytes received for file "<<fileList.number<< " : "<<noOfBytesRecvd<<endl;	
	  	}
	  	gettimeofday(&t2, NULL);	  	
	  	elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      
	  	elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   
	  	cout << "RTT: "<<elapsedTime << " ms\n";
	}
	else
	{
	  struct timeval t1, t2;
	  double elapsedTime;
	  gettimeofday(&t1, NULL);
	  connectionType="close";
	  string request=client.createRequest(argumentList[4],connectionType);
	  int noOfCharacters=client.sendRequest(request);
	  noOfCharacters=client.readResponse();
	  gettimeofday(&t2, NULL);
	  elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      
	  elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   
	  cout << "RTT: "<<elapsedTime << " ms\n";
	}
	client.closeConnection();
	return 0;
}
