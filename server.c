// Name of Author: Gabriel Nunes Crispino
// Course Number and Name: CSE 434, Computer Networks
// Semester: Fall 2015
// Project Part: 1
// Time Spent: 



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>

#define TRUE 1
#define MAXCONNECTIONS 5

//functions
int search_no(int client_number,int *connections);//search a client number in the array to check if it already exists.
void add_no(int client_number,int *connections);//adds a number to the array of client numbers.
void remove_number(int client_number,int *connections);//remove a number of the array of the client number after client connection ends.
int getNumberOfConnections(int *connections);//gets the current number of established connections.

//global variables
int *connections,nconnections = 0;//array that contains the client numbers(two client with the same number cannot be connected at the same time)


//MISSING VERIFICATION ON THE NUMBER OF CLIENTS!!!!!!!!!!!!!

int main(int argc, char *argv[]){
    int serv_socket,cli_socket;//socket file descriptor
    int portno;//portnumber
    int client_len;//Contains the size of the "client_addr" structure. Used on the "accept()" function.
    int test;//variable used to check if read() and write() function were executed with no errors.
    int client_number;
    int search;//result of search_no() function
    pid_t child_proc;//variable that contains the pid number for the child processes created after the connection establishment
    
    char buffer[255];
    
    struct sockaddr_in serv_addr,client_addr;
    
    //check if the arguments are correct
    if (argc != 2){
	printf("Number of arguments has to be one!\n");
	exit(0);
    }
    else
      portno = atoi(argv[1]);
    
    bzero((char *) &serv_addr, sizeof(serv_addr));
    
    connections = mmap(0,MAXCONNECTIONS * sizeof(int),PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1,0);
    
    memset(connections,0,50*sizeof(int));//initialize the "connections" array with zeros
    
    serv_socket = socket(AF_INET,SOCK_STREAM,0);
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(serv_socket,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0){
	printf("Error on bind() function!\n");
	exit(0);
    }
    
    if (listen(serv_socket,5) < 0){
	printf("Error on listen() function!\n");
	exit(0);
    }
    
        
    while(TRUE){
      bzero(buffer,sizeof(buffer));
      
      client_len = sizeof(client_addr);
      
      printf("Waiting for client socket...\n");
      
      cli_socket = accept(serv_socket,(struct sockaddr *)&client_addr,&client_len);
      
      if (getNumberOfConnections(connections) < MAXCONNECTIONS){//checks if maximum number of connections was reached
	
	if (cli_socket == -1){
	    printf("Error on accept() function!\n");
	    exit(0);
	}
	else{
	  //reads client number from client
	  test = read(cli_socket,buffer,sizeof(buffer));
	    
	  if (test == -1){
	      printf("Error on read() function!\n");
	      exit(0);
	  }
	  
	  
	  if (atoi(buffer)){
	      //if the passed string is a number
	      search = search_no(atoi(buffer),connections);
	      if (search < 0){
		client_number = atoi(buffer);
		add_no(client_number,connections);
		printf("nconnections: %d\n",getNumberOfConnections(connections));
		printf("Client number: %d\n",client_number);
	      }
	      else
		printf("Invalid client number!\n");
	      
	      //write search variable content to buffer
	      sprintf(buffer,"%d",search);
	      test = write(cli_socket,buffer,strlen(buffer));
	  }
	  else{
	    printf("Invalid client number!\n");
	    exit(0);
	  }
	  
	  child_proc = fork();//creates child process to handle client requests
	  
	  if (!child_proc){
	    if (search >= 0)//if client number is invalid, the child process is terminated
	      exit(0);
	    
	    printf("Connection with client socket %d established!\n",client_number);
	  
	    bzero(buffer,sizeof(buffer));
	    
	    test = read(cli_socket,buffer,sizeof(buffer));
	    
	    if (test == -1){
	      printf("Error on read() function!\n");
	      exit(0);
	    }
	    
	    printf("Message from client %d: %s",client_number,buffer);
	    
	    printf("Connection with client socket %d terminated!\n",client_number);
	    write(cli_socket,"Goodbye client\n",15);
	    
	    remove_number(client_number,connections);
	    
	  }
	}
	
      }
      else{
	printf("Maximum of 5 connections reached! Try again later.\n");
	
	//server writes "-2" value to buffer, which indicates that the maximum number of connections was reached
	sprintf(buffer,"%d",-2);
	test = write(cli_socket,buffer,strlen(buffer));
      }
      
    }
    
    close(cli_socket);
    close(serv_socket);
      
    return 0;  
}

//------------------------------------------------------------------------------

int search_no(int client_number,int *connections){
  int i;
  for (i = 0;i < MAXCONNECTIONS;i++)
    if (connections[i] == client_number)
      //found "client_number"
      return i;
    
    //if it doesn't find "client_number", it returns -1
    return -1;
}

void add_no(int client_number,int *connections){
  int i;
  
  for (i = 0;i < MAXCONNECTIONS;i++)
    if (!connections[i]){
      connections[i] = client_number;
      break;
    }
}

void remove_number(int client_number,int *connections){
  int pos = search_no(client_number,connections);
  connections[pos] = 0;
}

int getNumberOfConnections(int *connections){
  int nconnections = 0,i; 
  
  for (i = 0;i < MAXCONNECTIONS;i++)
    if (connections[i] != 0)
      nconnections++;
    
    return nconnections;
}
