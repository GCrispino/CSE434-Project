// Name of Author(s): Gabriel Nunes Crispino
// Course Number and Name: CSE 434, Computer Networks
// Semester: Fall 2015
// Project Part: 2
// Time Spent: 


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <netdb.h>
#include <dirent.h>

#define MAXSIZE 50 //maximum size of some strings used on the program
#define TRUE 1

//function that verifies errors after some functions
void test_err(int test,int type);

/*function that gets user input("str") and puts the filename string into "filename" variable, 
 *and the mode('R' or 'W') int "mode" variable 
*/
void getNameMode(char * str, char filename[][MAXSIZE], char * mode);

//this function does the type of operation described by "mode"(reading or writing) on the file which name is "filename"
void file_operation(char *filename,char mode,int sockfd);

//prints message that says what mode(read or write) the program is
void message(char mode);

int main(int argc, char *argv[]){
    //argument checking
    if ((argc - 1) != 3){
	printf("Number of arguments must be 3!\n");
	exit(0);
    }  
  
    int client_number,port_number,cli_socket,test;
  
    char buffer[255],buffer2[512];
    
    char filename[MAXSIZE],mode;
    
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
	bzero(buffer2,sizeof(buffer2));
	
	//printf("Please enter the message to send to the server: ");
	//reads the directory files' names to the client:
	test = read(cli_socket,buffer2,sizeof(buffer2));
	
	printf("Files located at server directory:\n %s\n",buffer2);
	
	do{
	  mode = ' ';
	  printf("Please enter you request in the following format: <filename>,<mode>: ");
	  fgets(buffer,sizeof(buffer),stdin);
	  
	  getNameMode(buffer,&filename,&mode);
	  
	  printf("File name: %s\n",filename);
	  
	  if (mode != 'R' && mode != 'W'){
	    printf("Invalid mode!");
	    getchar();
	  }
	  
	}while(mode != 'R' && mode != 'W');
	
	
	//puts "mode" content in "buffer" variable
	sprintf(buffer,"%c",mode);
	
	//write mode of operation to the server
	test = write(cli_socket,buffer,strlen(buffer));
	
	test_err(test,1);
	
	message(mode);
	
	file_operation(filename,mode,cli_socket);
	
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

void message(char mode){
  char md[10];
  
  if (mode == 'R')
    strcpy(md,"reading");
  else
    strcpy(md,"writing");
  
  printf("You are on the %s mode!\n",md);
  getchar();
}

void getNameMode(char * str, char filename[][MAXSIZE], char * mode){
  int i,flag_name = 1,flag_mode = 0;  
  
  for (i = 0; i < strlen(str);i++){
    
    if (str[i] == ','){
      flag_name = 0;
      flag_mode = 1;
      
      (*filename)[i] = '\0';
    }
    
    if (flag_name)
      //get the characters for the file name
      (*filename)[i] = str[i];
    else if (flag_mode){
      //get the character for the mode
      *mode = toupper(str[i + 1]);
      break;
    } 
  }
  
}

void file_operation(char *filename,char mode,int sockfd){
  
  FILE *file;  
  ssize_t len,test;
  char buff[BUFSIZ],buffer[255];
  int count = 0;
  
  bzero(buff,sizeof(buff));
  
  //writes file name to server
  len = write(sockfd,filename,MAXSIZE);
  test_err(len,1);  
  
  if (mode == 'R'){//reading mode
    file = fopen(filename,"wb");        
    
    //reads flag that indicates if file was found at the server
    test = read(sockfd,buffer,sizeof(buffer));
    test_err(test,0);
    
    if (buffer[0] == '0')
      printf("File not found!\n");
    else if (buffer[0] == '1')
      while( (len = read(sockfd,buff,sizeof(buff)) )){
	//client receives the file here
	
	test = fwrite(buff,sizeof(char),len,file);    
	test_err(test,1);
	
	if (len == 0 || len != BUFSIZ) break;
      
	count++;
      }
      
  }
  else{//writing mode
    file = fopen(filename,"rb");   
   
    if (file == NULL){
      printf("\n\nFile not found!\n\n");
      
      buffer[0] = '0';
      test = write(sockfd,buffer,sizeof(buffer));
    }
    else{
      buffer[0] = '1';
      test = write(sockfd,buffer,sizeof(buffer));
      
      while( (len = fread(buff,sizeof(char),sizeof(buff),file)) ){
	//file transmission
	    
	test = write(sockfd,buff,len);
	    
	if (test == -1){
	  printf("Error on write() function!\n");
	  exit(0);
	}
	    
	count++;
      }
      
    }
  }
  
  if (file) fclose(file);
}