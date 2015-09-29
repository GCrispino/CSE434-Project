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
#include <dirent.h>

#define TRUE 1
#define MAXCONNECTIONS 5
#define MAXSIZE 50

//functions
int search_no(int client_number,int *connections);//search a client number in the array to check if it already exists.
void add_no(int client_number,int *connections);//adds a number to the array of client numbers.
void remove_number(int client_number,int *connections);//remove a number of the array of the client number after client connection ends.
int getNumberOfConnections(int *connections);//gets the current number of established connections.
void getFilesDirectory(char * str);//shows all the files within the program directory
void initializeLockArray(char array[][MAXSIZE]);//function used to initialize write and read lock arrays with empty strings.
int search_file(char * filename, char array[][MAXSIZE]);//searches for filename in "filename" in the "array" array.
void add_file(char * filename, char array[][MAXSIZE]);//adds a file name to a lock array.
void file_operation(char mode,int sockfd);//this function does the type of operation described by "mode"(reading or writing).

//global variables
int *connections,nconnections = 0;//array that contains the client numbers(two client with the same number cannot be connected at the same time)
char readlock[MAXCONNECTIONS][MAXSIZE];//array that controls the readlocks for the files
char writelock[MAXCONNECTIONS][MAXSIZE];//array that controls the writelocks for the files

//!!!!!TRANSMISSION IS NOT WORKING!!!!!

int main(int argc, char *argv[]){   
  
    int serv_socket,cli_socket;//socket file descriptor
    int portno;//portnumber
    int client_len;//Contains the size of the "client_addr" structure. Used on the "accept()" function.
    int test;//variable used to check if read() and write() function were executed with no errors.
    int client_number;
    int search;//result of search_no() function
    pid_t child_proc;//variable that contains the pid number for the child processes created after the connection establishment
    
    FILE * file;
    
    char buffer[255],buffer2[512];
    
    char ans;//saves a value that answers if the client wants to make another request or not
    char mode;
    
    struct sockaddr_in serv_addr,client_addr;
    
    //check if the arguments are correct
    if (argc != 2){
	printf("Number of arguments has to be one!\n");
	exit(0);
    }
    else
      portno = atoi(argv[1]);
    
    bzero((char *) &serv_addr, sizeof(serv_addr));
    
    //ARRAY INITIALIZATIONS:
    
    //initializes the "connections" array
    connections = mmap(0,MAXCONNECTIONS * sizeof(int),PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1,0);
    
    //initialize the "connections" array with zeros
    memset(connections,0,50*sizeof(int));
    
    //initializes "readlock" and "writelock" arrays
    initializeLockArray(readlock);
    initializeLockArray(writelock);
    
    
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
	    do{
	      if (search >= 0)//if client number is invalid, the child process is terminated
		exit(0);
	      
	      printf("Connection with client socket %d established!\n",client_number);
	    
	      bzero(buffer,sizeof(buffer));
	      
	      //writes directory files' names to the client:
	      getFilesDirectory(buffer2);
	      test = write(cli_socket,buffer2,sizeof(buffer2));
	      
	      //reads mode of operation from the client
	      test = read(cli_socket,buffer,sizeof(buffer));
	      
	      if (test == -1){
		printf("Error on read() function!\n");
		exit(0);
	      }	      
	      
	      mode = buffer[0];
	      
	      printf("Mode of operation: %c\n",mode);
	      
	      
	      file_operation(mode,cli_socket);
	      
	      //------IMPORTANT----------
	      /*test = read(cli_socket,buffer,sizeof(buffer));
	      
	      if (test == -1){
		printf("Error on read() function!\n");
		exit(0);
	      }
	      
	      printf("Message from client %d: %s",client_number,buffer);
	      
	      //writes to client the same message that was received, so it can see it on the other side
	      write(cli_socket,buffer,sizeof(buffer));
	      */
	      
	      
	      printf("Waiting for client %d response...\n",client_number);
	      test = read(cli_socket,buffer,sizeof(buffer));
	      
	      if (test == -1){
		printf("Error on read() function!\n");
		exit(0);
	      }
	      
	      ans = buffer[0];
	      
	      if (ans == 'N'){
		printf("Connection with client socket %d terminated!\n",client_number);
		//write(cli_socket,"Goodbye client\n",15);
		
		remove_number(client_number,connections);
	      }
	    
	    }while(ans == 'Y');
	  }
	}
	
      }
      else{//if  number of connections is already 5
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

  
void getFilesDirectory(char * str){//reads the names of the files in the directory the program is running and saves it into "str"
  
  DIR * dir = opendir(".");
  struct dirent * entry;//represents the next directory entry
  char tmp[MAXSIZE];
  
  strcpy(str,"");
  
  
  while(entry = readdir(dir)){
    
    if (entry){
      sprintf(tmp,"%s\n",entry->d_name);
      strcat(str,tmp);
    }
    
  }
  
  closedir(dir);
} 

void initializeLockArray(char array[][MAXSIZE]){
  int i;
  
  for (i = 0;i < MAXSIZE;i++)
    strcpy(array[i],"");
 
}

int search_file(char * filename, char array[][MAXSIZE]){
  int i;
  
  for (i = 0;i < MAXCONNECTIONS;i++)
    if (!strcmp(array[i],filename))
      //found "filename" in the array
      return i;
  
  //didn't find the desired string
  return -1;
}

void add_file(char * filename, char array[][MAXSIZE]){
  int i;
  
  for (i = 0;i < MAXCONNECTIONS;i++)
    if (!strcmp(array[i],"")){
      strcpy(array[i],filename);
      break;
    }  
}

void remove_file(char * filename, char array[][MAXSIZE]){
  int pos = search_file(filename,array);
  strcpy(array[pos],"");
}

void file_operation(char mode,int sockfd){
  
  char filename[MAXSIZE],buff[BUFSIZ];
  FILE * file;
  int lock,count = 0;
  ssize_t len,test;
  
  bzero(filename,sizeof(filename));
  printf("Sizeof filename: %ld\n",sizeof(filename));
  
  if (mode == 'R'){
    //if the client chooses reading mode
    
    test = read(sockfd,filename,sizeof(filename));
    
    if (test == -1){
      printf("Error on read() function!\n");
      exit(0);      
    }
    
    printf("File name: %s\n",filename);
    
    if (search_file(filename,writelock) >= 0)
      //if there is a writelock, a client cannot read or write the file.
      printf("File is being used by other client!(write lock)\n");
    else{
      //otherwise, it can read the file.
      
      file = fopen(filename,"rb");
      
      if (file == NULL)
	printf("\n\nFile not found!\n\n");
      else{      
	
	if (search_file(filename,readlock) < 0){//if the file is not on the readlock array, it is put there now.
	  add_file(filename,readlock);
	  lock = 1;//variable used to close the lock afterwards.
	}
	
	while( (len = fread(buff,sizeof(char),sizeof(buff),file)) ){
	  //file transmission
	  printf("Count: %d\n",count);
	  
	  printf("len: %ld\n",len);
	  
	  test = write(sockfd,buff,len);
	  
	  printf("Number of bytes written: %ld\n",test);
	  
	  if (test == -1){
	      printf("Error on write() function!\n");
	      exit(0);
	  }
	  
	  count++;
	}
	
	
	if (lock)//if this client opened the lock, it will be the one who will closes it.
	  remove_file(filename,writelock);
      }
    }   
    
    
  
  }
  else{
    //if the client chooses writing mode
    
  }
  
  
}