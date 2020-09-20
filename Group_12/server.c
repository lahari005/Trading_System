#include <stdio.h>  
#include <string.h>   //strlen  
#include <stdlib.h>  
#include <errno.h>  
#include <unistd.h>   //close  
#include <arpa/inet.h>    //close  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros 

#define INT_MAX 2147483647
// structure for request
typedef struct request
{
    int user_id;
    int item_code;
    int price;
    int quantity;
}request; 

//array of buy requests
request buy[1024];

//array of sell requests
request sell[1024];

//structure for matched requests.
typedef struct orders
{
    int sell_id;
    int buy_id;
    int price;
    int quantity;
    int item_code;
}orders;

//array of matched orders
orders trade[1024];
int index_tr=0;

//helper function for match_trade which return min selling price for 10 items.
request* trade_helper()
{
    request min_sell[10];
    request * temp;
    temp = (request *) malloc(10*sizeof(request)); 
    for(int i=0;i<10;i++)
    {
        
        min_sell[i].price=INT_MAX;
        min_sell[i].item_code=i+1;
        min_sell[i].quantity=0;
        min_sell[i].user_id=-1; 
    }

    for(int i=0;i<1024;i++)
    {
        if(sell[i].price < min_sell[sell[i].item_code-1].price)
        {
            min_sell[sell[i].item_code-1]=sell[i];
        }
    }
    
    for(int i=0;i<10;i++)
    {
  
        if(min_sell[i].price==INT_MAX)
        {
            temp[i].item_code=i+1;
            temp[i].quantity=0;
            temp[i].price=0;
        }
        else
        {
            temp[i]=min_sell[i];   
        }

    } 
    return temp;
}

// function which matches sell requests and buy requests.
// for each buy request function finds min sell price and add both to trade array and makes necessary changes to buy and sell array.
void match_trade()
{
    for(int i=0;i<1024;i++)
    {
        if(buy[i].item_code==-1)
            break;

        request* min_sell=trade_helper(); 
        //printf("hello\n"); 
        int itemcode = buy[i].item_code;
        
        int flag=1;
        if(min_sell[itemcode-1].quantity==0)
        {
        	//printf("hello2\n");
            flag=0;
        }
        int k=0;

        for (int j = 0; j < 1024 && flag==1; ++j)
        {
            if(min_sell[itemcode-1].user_id==sell[j].user_id && min_sell[itemcode-1].item_code == sell[j].item_code && min_sell[itemcode-1].quantity == sell[j].quantity && min_sell[itemcode-1].price == sell[j].price)
            {   
            	//printf("hello3\n");
            	k=j;
                break;
            }
        }

        if(min_sell[itemcode-1].quantity==buy[i].quantity && min_sell[itemcode-1].price<=buy[i].price && flag!=0)
        {
        	//printf("hello4\n");
            trade[index_tr].sell_id=min_sell[itemcode-1].user_id;
            trade[index_tr].buy_id=buy[i].user_id;
            trade[index_tr].price=min_sell[itemcode-1].price;
            trade[index_tr].quantity=buy[i].quantity;
            trade[index_tr].item_code=buy[i].item_code;
            index_tr++;
            buy[i].quantity=0;
            buy[i].item_code=0;
            buy[i].price=0;
            buy[i].user_id=0;
            sell[k].quantity=0;
            sell[k].item_code=0;
            sell[k].price=0;
            sell[k].user_id=0;
        }
        else
        if(min_sell[itemcode-1].quantity>buy[i].quantity && min_sell[itemcode-1].price<=buy[i].price && flag!=0)
        {
        	//printf("hello5\n");
            trade[index_tr].sell_id=min_sell[itemcode-1].user_id;
            trade[index_tr].buy_id=buy[i].user_id;
            trade[index_tr].price=min_sell[itemcode-1].price;
            trade[index_tr].quantity=buy[i].quantity;
            trade[index_tr].item_code=buy[i].item_code;
            index_tr++;
            sell[k].quantity=sell[k].quantity-buy[i].quantity;
            buy[i].quantity=0;
            buy[i].item_code=0;
            buy[i].price=0;
            buy[i].user_id=0;
            
        }
        else
        if(min_sell[itemcode-1].quantity<buy[i].quantity && min_sell[itemcode-1].price<=buy[i].price && flag!=0)
        {
        	//printf("hello6\n");
            trade[index_tr].sell_id=min_sell[itemcode-1].user_id;
            trade[index_tr].buy_id=buy[i].user_id;
            trade[index_tr].price=min_sell[itemcode-1].price;
            trade[index_tr].quantity=sell[k].quantity;
            trade[index_tr].item_code=sell[k].item_code;
            index_tr++;
            buy[i].quantity=buy[i].quantity-sell[k].quantity;
            sell[k].quantity=0;
            sell[k].item_code=0;
            sell[k].price=0;
            sell[k].user_id=0;
            //i=i-1;
        }

    }
}

//order status function finds maximum selling price and minimum buying price for each order.
void orders_st(int st)
{
    request max_buy[10];
    request min_sell[10];
    char buffer[1024],temp1[512],temp2[512];
    memset(buffer,0,1024);
    memset(temp1,0,512);
    memset(temp2,0,512);

    for(int i=0;i<10;i++)
    {
        max_buy[i].price=-1;
        min_sell[i].price=INT_MAX;
        max_buy[i].item_code=i+1;
        max_buy[i].quantity=0;
        max_buy[i].user_id=-1;  
        min_sell[i].item_code=i+1;
        min_sell[i].quantity=0;
        min_sell[i].user_id=-1; 
    }

    for(int i=0;i<1024;i++)
    {
      //  printf("%d %d\n",buy[i].price,max_buy[buy[i].item_code].price);
        if(buy[i].price > max_buy[buy[i].item_code-1].price)
        {
            max_buy[buy[i].item_code-1]=buy[i];
          
        }
    }

    strcat(buffer,"buy\n");
    for(int i=0;i<10;i++)
    {
        memset(temp1,0,512);
        if(max_buy[i].price==-1)
        {
            sprintf(temp1,"item %d:price %d:quantity %d\n",i+1,0,0);
        }
        else
        {
         sprintf(temp1,"item %d:price %d:quantity %d\n",i+1,max_buy[i].price,max_buy[i].quantity);   
        }
        strcat(buffer,temp1);
    }

    for(int i=0;i<1024;i++)
    {
        if(sell[i].price < min_sell[sell[i].item_code-1].price)
        {
            min_sell[sell[i].item_code-1]=sell[i];
        }
    } 
    
    strcat(buffer,"sell\n");
    for(int i=0;i<10;i++)
    {
        memset(temp2,0,512);
        if(min_sell[i].price==INT_MAX)
        {
            sprintf(temp2,"item %d:price %d:quantity %d\n",i+1,0,0);
        }
        else
        {
         sprintf(temp2,"item %d:price %d:quantity %d\n",i+1,min_sell[i].price,min_sell[i].quantity);   
        }
        strcat(buffer,temp2);
    }
    int n = write(st,buffer,sizeof(buffer));
}

// function send the matched trade status information to client.
// Reads data from trade array and sends to client
void trade_st(int st, int user_id)
{
    int i=0;
    char buffer[1024],temp[512];
    memset(buffer,0,1024);

    while(i<index_tr)
    {
        memset(temp,0,512);
        sprintf(temp,"buy id :%d  sell id :%d item :%d quantity :%d price :%d\n",trade[i].buy_id,trade[i].sell_id,trade[i].item_code,trade[i].quantity,trade[i].price);
        strcat(buffer,temp);
        i++;
    }
    int n=write(st,buffer,sizeof(buffer));

}

//reads the data from client and adds the details to sell array.
void sell_order(char * temp, int user_id)
{
    char *t=strtok(temp,"-");
    int itemcode=atoi(t);
    t=strtok(NULL,"-");
    int quantity=atoi(t);
    t=strtok(NULL,"-");
    int price=atoi(t);
    int i=0;
    while(1)
    {
        if(sell[i].item_code==-1)
            break;
        i++;
    }
    
    sell[i].item_code=itemcode;
    sell[i].quantity=quantity;
    sell[i].price=price;
    sell[i].user_id=user_id;
}

//reads the data from client and adds the details to buy array. 
void buy_order(char * temp, int user_id)
{
    char *t=strtok(temp,"-");
    int itemcode=atoi(t);
    t=strtok(NULL,"-");
    int quantity=atoi(t);
    t=strtok(NULL,"-");
    int price=atoi(t);
    int i=0;
    
    while(1)
    {
        if(buy[i].item_code==-1)
            break;
        i++;
    }

    buy[i].item_code=itemcode;
    buy[i].quantity=quantity;
    buy[i].price=price;
    buy[i].user_id=user_id;

    // for(int i=0;i<3;i++)
    // {
    //     printf("%d %d %d %d\n",buy[i].user_id,buy[i].item_code,buy[i].quantity,buy[i].price);
    // }
}

int search_index(char buffer[])
{
    for(int i=0;i<1024;i++)
    {
        if(buffer[i]==':')
            return i;
    } 

    return 1024;
}

  

  
int auth(char* logindetails)
{
    //The login details are stored in a file login.txt line by line.So we are given logindetails in format username:password we open file and
    //each line by comparing with logindetails and simultaneoulsy increment the index i for each mismatch.When match found retrn the index i

    FILE *fp=fopen("login.txt","r");
    char *dataToBeRead; dataToBeRead = (char*) malloc(64*sizeof(char));
    int i=0;int f=0;
    if ( fp != NULL ) 
    {  
    
    while(fscanf(fp,"%s\n",dataToBeRead) != EOF) 
    { i++;
       if(strcmp(logindetails,dataToBeRead)==0)
            {f=1;break;}
    } 
      
    fclose(fp);
    }
    else
    {
        printf("Login File not exitsing\n");
    }
    if(f) return i;

    return 0;
}   
int main(int argc , char *argv[])   
{   
     for(int i=0;i<1024;i++)
        {
            buy[i].item_code=-1;
            buy[i].user_id=-1;
            buy[i].price=-1;
            buy[i].quantity=-1;

            sell[i].item_code=-1;
            sell[i].user_id=-1;
            sell[i].price=-1;
            sell[i].quantity=-1;

        }

         for(int i=0;i<1024;i++)
        {
            trade[i].item_code=-1;
            trade[i].buy_id=-1;
            trade[i].sell_id=-1;
            trade[i].price=-1;
            trade[i].quantity=-1;
        }

    if(argc!=2){
        printf("Number of arguments should be 2\n");
        return 0;
    }
    int PORT=atoi(argv[1]); //PORT here is the port number taken from the arguments 
    
    int opt = 1;   
    int head_socket,new_socket,addr_length;
    int *client_socket;
    client_socket=(int *)malloc(5*sizeof(int));
    int *user_id;
    user_id=(int *)malloc(5*sizeof(int));
    int max_clients=5;
    int count_clients=0;
    int activity,value_read,sd,max_sd,k;
    struct sockaddr_in server_address,client_address;   
         
    char buffer[1025];  //data buffer of 1K  
         
    //set of socket descriptors  
    fd_set readfds;   
         
    //a message  
    char *message = "Connection is Established\n";   
    //initialising user id's to 0
    int m=0;
    while(m<max_clients){
        user_id[m]=0;
        m++;
    }
     
    //initialise all client_socket[] to 0 so not checked  
    for (int i = 0; i < max_clients; i++)   
    {   
        client_socket[i] = 0;   
    }   
         
    //create a head socket 
    head_socket=socket(AF_INET,SOCK_STREAM,0); 

    if(head_socket == 0){ 
        perror("socket failed");   
        exit(EXIT_FAILURE);   
    }      
     
    //set head socket to allow multiple connections ,  
    //this is just a good habit, it will work without this  
    if( setsockopt(head_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,sizeof(opt)) < 0 )   
    {   
        perror("setsockopt");   
        exit(EXIT_FAILURE);   
    }   
     
    //type of socket created  
    server_address.sin_family = AF_INET;   
    server_address.sin_addr.s_addr = INADDR_ANY;   
    server_address.sin_port = htons( PORT );   
         
    //bind the socket with server server_address  
    if (bind(head_socket, (struct sockaddr *)&server_address, sizeof(server_address))<0)   
    {   
        perror("bind failed");   
        exit(EXIT_FAILURE);   
    }     
         
    //if listen is successful then it will return 0 otherwise some other socket might be using the port 
    if (listen(head_socket, 3) < 0)   
    {   
        perror("listen");   
        exit(EXIT_FAILURE);   
    }   
         
    //accept the incoming connection  
    addr_length = sizeof(server_address);           
    while(1)   
    {   
        //clear the socket set  
        FD_ZERO(&readfds);   
     
        //add head socket to set  
        FD_SET(head_socket, &readfds); 
        //max_sd denotes the highest file descriptor and it must be updated accordingly                          
        max_sd = head_socket;   
             
        //add child sockets to set  
        for ( int i = 0 ; i < max_clients ; i++)   
        {   
            //socket descriptor  
            sd = client_socket[i];   
                 
            //if valid socket descriptor then add to read list  
            if(sd > 0)   
                FD_SET( sd , &readfds);   
                 
            //highest file descriptor number, need it for the select function  
            if(client_socket[i] > max_sd)   
                max_sd = sd;   
        }   
     
        //wait for an activity on one of the sockets , timeout is NULL ,  
        //so wait indefinitely  
        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);   
       
        if ((activity < 0) && (errno!=EINTR))   
        {   
            printf("select error");   
        }   
             
        //If something happened on the head socket ,  
        //then its an incoming connection  
        if (FD_ISSET(head_socket, &readfds))   
        {   
            if ((new_socket = accept(head_socket,  
                    (struct sockaddr *)&server_address, (socklen_t*)&addr_length))<0)   
            {   
                perror("accept did not succeed");   
                exit(EXIT_FAILURE);   
            }  

            if (count_clients < max_clients)
             {
                 count_clients++;
                 printf("New connection has been Established\n");
                 if( send(new_socket, message, strlen(message), 0) != strlen(message) )   
                    {   
                         perror("sending failed");   
                    }
             }
             else
             {
                char *message2="Number of connections exceeded";
                k=write(new_socket,message2,strlen(message2));
                close(new_socket);
             } 

            //add new socket to array of sockets  
            for (int i = 0; i < max_clients; i++)   
            {   
                //if position is empty  
                if( client_socket[i] == 0 )   
                {   
                    client_socket[i] = new_socket;   
                    printf("Adding to list of sockets as %d\n" , i);   
                         
                    break;   
                }   
            }   
        }   
        
       
        //else its some IO operation on some other socket 
        for (int i = 0; i < max_clients; i++)   
        {   
            sd = client_socket[i];   
                 
            if (FD_ISSET( sd , &readfds))   
            {   
                //Check if it was for closing , and also read the  
                //incoming message  
                value_read=read(sd, buffer, 1024);
                //Echo back the message that came in 
                if(value_read != 0)   
                {   
                    char *index=(char *)malloc(1025*sizeof(char));
                    strcpy(index,buffer);
                    memset(buffer,0,1025);
                    int size=search_index(index);
                    if(size<1024) index[size]='\0';
                    printf("USER ID %d Client number %d Message Received %s\n",user_id[i],i,index );
                    //String before # indicates  the function that the client wants to use
                    char* temp=strtok(index,"#"); 
                    char t= (*temp);
                    if (t=='L')
                    {
                    	temp=strtok(NULL,"#");
                    	int ind=auth(temp);
                    	if(ind!=0){
                    		int b=0;
                    		for (int j = 0; j < 5; ++j)
                    		{
                    			if (user_id[j]==ind)
                    			{
                    				b=1;
                    				break;
                    			}
                    		}
                    		if (b==1)
                    		{
                    			char *message2="Already Logged In";
                    			write(sd,message2,strlen(message2));
                    			getpeername(sd,(struct sockaddr*)&client_address,(socklen_t*)&addr_length);
                    			printf("Host has been disconnected, IP address : %s , Port number : %d , Client number : %d \n",inet_ntoa(client_address.sin_addr),ntohs(client_address.sin_port),i);
                    			close(sd);
                    			client_socket[i]=0;
                    			count_clients--;
                    		}
                    		else 
                    		{
                    			user_id[i]=ind;
                    			char message2[1024];
                    			memset(message2,0,1024);
                    			sprintf(message2,"User_ID : %d Logged In",ind);
                    			write(sd,message2,strlen(message2));
                    		}
                    	}
                    	else 
                    	{
                    		char *message2="Invalid username or password";
                    		write(sd,message2,strlen(message2));
                    	}
                    	break;
                    }

		    //Buy Request
                    //We check for trade after buy or sell request.
                    if (t=='B')
                    {
                        
                    	temp=strtok(NULL,"#");  
                        buy_order(temp,user_id[i]); 
                        match_trade();
                        char *message2="Message Received\n";
                        write(sd , message2 , strlen(message2) );     
                        break;
                    }
	            //Sell Request
                    if (t=='S')
                    {
                    	temp=strtok(NULL,"#");  
                        sell_order(temp,user_id[i]); 
                        match_trade();
                        char *message3="Message Received\n";
                        write(sd , message3 , strlen(message3) );     
                        break;
                    }
		    //Order Status
                    if(t=='O')
                    {
                    	orders_st(sd);
                    	break;
                    }
		    //Trade_status
                    if(t=='T') 
                    {
                    	trade_st(sd,user_id[i]);
                    	break;
                    }

                }   
                else //if value_read==0 then it implies that the client has sent 0 bytes of dates which means that the client wants to terminate the connection
                {   
                    //Somebody disconnected , get his details and print  
                    getpeername(sd , (struct sockaddr*)&client_address ,(socklen_t*)&addr_length);  
                    printf("Host disconnected , ip %s , port %d,client no:%d ,user  id :%d\n" ,inet_ntoa(client_address.sin_addr) , ntohs(client_address.sin_port),i,user_id[i]);  
                         
                    //Close the socket and mark as 0 in list for reuse  
                    close( sd );   
                    client_socket[i] = 0;
                    user_id[i] = 0; 
                    count_clients--;
                }   
            }   
        }   
    }   
         
    return 0;   
}
