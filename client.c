// Name of Author(s): Gabriel Nunes Crispino
// Course Number and Name: CSE 434, Computer Networks
// Semester: Fall 2015
// Project Part: 1
// Time Spent: 


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <netdb.h>

//function that verifies errors after some functions
void test_err(int test,int type){
  //if type is 0, its a read() function. Otherwise, its a write() function
  
  char s[20];
  
  if (test)
    return;
  else{
    if (type)
      strcpy(s,"write");
    else
      strcpy(s,"read");
    
    printf("Error on %s() function!\n",s);
    exit(0);
  } 
  
}

//WORK ON THE SERIES OF REQUEST ON PROGRESS(LINE 104)!!!

int main(int argc, char *argv[]){
    //argument checking
    if ((argc - 1) != 3){
	printf("Number of arguments must be 3!\n");
	exit(0);
    }  
  
    int client_number,port_number,cli_socket,test;
  
    char buffer[255];
    char ans;//saves a value that answers if the client wants to make another request or not
    
    //structures used in the program
    struct hostent *server;//structure used to handle the server hostname
    struct sockaddr_in serv_addr;//structure used to handle the server's internet address
    
    
    //arguments handling
    client_number = atoi(argv[2]);
    port_number = atoi(argv[3]);
    
    server = gethostbyname(argv[1]);
    
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port_number);
    bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);//copies address from "server" structure to "serv_addr" structure
    
    
    cli_socket = socket(AF_INET,SOCK_STREAM,0);
    
    if (connect(cli_socket,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0){
      printf("Error on connect() function!\n");
      exit(0);
    }
    
    sprintf(buffer,"%d",client_number); //sprintf converts integer variable "client_number" to a string and puts it into "buffer"
    
    test = write(cli_socket,buffer,strlen(buffer));//write client number to buffer
    test_err(test,1);
    
    bzero(buffer,sizeof(buffer));
    
    /*reads client number verification result from the server(if content stored in "client_number" alredy exists, 
    the server is going to store 0 or a positive number in the buffer. Otherwise, -1 is going to be stored) */
    test = read(cli_socket,buffer,sizeof(buffer));
    test_err(test,0);
    
    test = atoi(buffer);
    if (test == -1){
      do{
	//then client number sent to the server is valid
      
	bzero(buffer,sizeof(buffer));
	
	printf("Please enter the message to send to the server: ");
	fgets(buffer,sizeof(buffer),stdin);
	
	test = write(cli_socket,buffer,strlen(buffer));
	
	test_err(test,1);
	
	//reads message that was sent
	test = read(cli_socket,buffer,sizeof(buffer));
	
	test_err(test,0);
	
	//prints the message the server got
	printf("Message from the server: %s",buffer);
	
	do{
	  printf("Do you want to send another request(Y or N)?\n");
	  ans = getchar();
	  ans = toupper(ans);
	  
	  getchar();
	  
	  if (ans != 'Y' && ans != 'N'){
	    printf("Invalid input!\n");
	    getchar();
	  }
	}while(ans != 'Y' && ans != 'N');
	
	buffer[0] = ans;
	
	test = write(cli_socket,buffer,strlen(buffer));
	
	test_err(test,1);
	
	if (ans == 'N')
	  break;
      }while(ans == 'Y');
    }
    else if(test == -2)
      printf("Maximum of 5 connections reached! Try again later.\n");
    else
      printf("Invalid client number!\n");
    
    
    close(cli_socket);
    
    return 0;
}
