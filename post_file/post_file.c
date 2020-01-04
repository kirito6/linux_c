#include <stdlib.h>  
#include <sys/types.h>  
#include <stdio.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <string.h>  
#include <sys/stat.h>


#define SERVER_ADDR     "192.168.1.102"
#define SERVER_PORT     5000
#define SERVER_URL      "test.inteink.com"
#define SERVER_PATH     "/upload"
 
#define HTTP_HEAD       "POST %s HTTP/1.1\r\n"\
                                        "Host: %s\r\n"\
                                        "User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux i686; rv:59.0) Gecko/20100101 Firefox/59.0\r\n"\
                                        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"\
                                        "Accept-Language: en-US,en;q=0.5\r\n"\
                                        "Accept-Encoding: gzip, deflate\r\n"\
                                        "Content-Type: multipart/form-data; boundary=%s\r\n"\
                                        "Content-Length: %ld\r\n"\
                                        "Connection: close\r\n"\
                                        "Upgrade-Insecure-Requests: 1\r\n"\
                                        "DNT: 1\r\n\r\n"\
                                           


                                              
unsigned char header[1024]={0}; 
unsigned char send_request[1024]={0};
unsigned char send_end[1024]={0};
unsigned char send_key_value_request[512]={0};
unsigned char http_boundary[64]={0};
  
unsigned long get_file_size(const char *path)  //获取文件大小
{    
    unsigned long filesize = -1;        
    struct stat statbuff;    
    if(stat(path, &statbuff) < 0){    
        return filesize;    
    }else{    
        filesize = statbuff.st_size;    
    }    
    return filesize;    
}   


// 文件类型
typedef enum File_Type{
	jpeg = 1,
	png,
	bmp,
	bin
}File_Type;

typedef struct HTTP_POST_ITEM_TAG
{
	File_Type file_type;
	char is_file;
	char name[20];
	char name_len;
	char *content;
	long content_len;
	long request_len;
}HTTP_POST_ITEM;


#define MAX_POST_ITEM_LEN 5
typedef struct HTTP_POST_TAG
{
	HTTP_POST_ITEM http_post_item[MAX_POST_ITEM_LEN];
	char post_item_len;
}HTTP_POST;

HTTP_POST m_http_post;

// 添加到post中
int http_post_add_item(HTTP_POST *http_post,HTTP_POST_ITEM *http_post_item)
{
	if (http_post->post_item_len > MAX_POST_ITEM_LEN){
		return -1;
	}
	http_post->http_post_item[http_post->post_item_len++] = *http_post_item;
	return 0;
}



#define POST_FILE_REQUEST  "--%s\r\n"\
                                                "Content-Disposition: form-data; name=\"file\"; filename=\"%s\"\r\n"\
                                                "Content-Type: %s\r\n\r\n"
 
#define POST_KEY_VALUE_REQUEST "--%s\r\n"\
                                                "Content-Disposition: form-data; name=\"%s\"\r\n\r\n"\
                                                "%s\r\n"

char http_post_head_buffer[1024];
char http_post_item_buffer[80960];
char http_post_buffer[90960]={0};
char request[91060]={0};
char data_temp[20]="\r\n";
char end_temp[128];


int http_post_sent(const unsigned char *IP, const unsigned int port,char *URL,HTTP_POST *http_post)
{

	int cfd = -1;
    int recbytes = -1;
    int sin_size = -1;
    char rec_buffer[1024*10]={0};     
    struct sockaddr_in s_add,c_add;

    //创建socket套接字
    cfd = socket(AF_INET, SOCK_STREAM, 0);  
    if(-1 == cfd)  
    {  
        printf("socket fail ! \r\n");  
        return -1;  
    }
        
    bzero(&s_add,sizeof(struct sockaddr_in));  
    s_add.sin_family=AF_INET; //IPV4
    s_add.sin_addr.s_addr= inet_addr(IP);  
    s_add.sin_port=htons(port);
    printf("s_addr = %s ,port : %d\r\n",IP,port);


    //建立TCP连接
        if(-1 == connect(cfd,(struct sockaddr *)(&s_add), sizeof(struct sockaddr)))  
    {  
        printf("connect fail !\r\n");  
        return -1;  
    }


        //获取毫秒级的时间戳用于boundary的值


	unsigned long temp_len = 0;
	unsigned long request_len = 0;
	unsigned long request_upload_len = 0;
	unsigned long totalsize = 0;

	for(int i = 0; i < http_post->post_item_len; i++){
		temp_len += http_post->http_post_item[i].name_len + http_post->http_post_item[i].content_len;
	}

	for(int i = 0; i < http_post->post_item_len; i++)
	{	
	
		if(http_post->http_post_item[i].is_file){
			unsigned long request_item_len = snprintf(http_post_item_buffer,40960,POST_FILE_REQUEST,"---------------------------1234",http_post->http_post_item[i].name,"image/png"); //请求信息1
			printf("request_item_len:%d\n", request_item_len);
			// 复制到复制内容过去
			memcpy(http_post_item_buffer + request_item_len,http_post->http_post_item[i].content,http_post->http_post_item[i].content_len);
	
			// 末尾添加回车
			memcpy(http_post_item_buffer + request_item_len + http_post->http_post_item[i].content_len,data_temp,2);
			
			request_upload_len = request_item_len + http_post->http_post_item[i].content_len + 2;
			// 复制到总的缓冲区
			memcpy(http_post_buffer + request_len,http_post_item_buffer,request_upload_len);
			request_len += request_upload_len;
	

		}else{
			unsigned long request_item_len = snprintf(http_post_item_buffer,40960,POST_KEY_VALUE_REQUEST,"---------------------------1234",http_post->http_post_item[i].name,http_post->http_post_item[i].content); //请求信息1
			request_upload_len = request_item_len;
			memcpy(http_post_buffer + request_len,http_post_item_buffer,request_upload_len);
			request_len += request_upload_len;
		}
		printf("filesize:%d\n", request_upload_len);
	}

	// 尾巴数据
	unsigned long end_len = snprintf(end_temp,128,"--%s--\r\n","---------------------------1234"); //结束信息
	memcpy(http_post_buffer + request_len,end_temp,end_len);
	request_len += end_len;
	printf("end_len:%d\n", end_len);
	
 	// HTTP头
    unsigned long head_len = snprintf(request,1024,HTTP_HEAD,SERVER_PATH,URL,"---------------------------1234",request_len); //头信息
    totalsize = head_len + request_len;
    printf("head_len:%d\n", head_len);
    printf("totalsize:%d\n", totalsize);
    
 
    /******* 拼接http字节流信息 *********/ 
    strcat(request,header);                                                                     //http头信息
    memcpy(request + head_len,http_post_buffer,request_len);

    printf("hed:%d\n", head_len);
	for(int i =0 ;i< totalsize;i++){
		printf("%c", request[i]);
	}


     /*********  发送http 请求 ***********/
    if(-1 == write(cfd,request,totalsize))                                  
    {  
        printf("send http package fail!\r\n");  
        return -1;  
    }
 
    /*********  接受http post 回复的json信息 ***********/
     char ack_json[256]={0};

    if(-1 == (recbytes = read(cfd,rec_buffer,10240)))  
    {  
        printf("read http ACK fail !\r\n");  
        return -1;  
    }  
    rec_buffer[recbytes]='\0';
    int index = 0,start_flag = 0;
    int ack_json_len = 0;
    for(index = 0; index<recbytes; index++)
    {
            if(rec_buffer[index] == '{')
            {
                    start_flag = 1;
            }
            if(start_flag)
                    ack_json[ack_json_len++] = rec_buffer[index];  //遇到左大括号则开始拷贝

            if(rec_buffer[index] == '}')                //遇到右大括号则停止拷贝
            {
                    ack_json[ack_json_len] = '\0';
                    break;
            }
    }
    if(ack_json_len > 0 && ack_json[ack_json_len-1] == '}')  //遇到花括号且有json字符串
    {
            printf("Receive:%s\n",ack_json);
    }
    else
    {
            ack_json[0] = '\0';
            printf("Receive http ACK fail!!\n");
            printf("--- ack_json_len = %d\n",ack_json_len);
    }
       
    close(cfd);
	return 0;
}
int main(int argc, char *argv[])  
{       char path[]="cl.png";
        int ack_len = 256;
        char ack_json[256]={0};

        HTTP_POST http_post;
        HTTP_POST_ITEM http_post_item1;
        HTTP_POST_ITEM http_post_item2;

        char name[]="name34";
        char content[] = "kirito";
        char mss_data[]="1234567890";




        http_post_item1.file_type = jpeg;
        http_post_item1.is_file = 0;

        http_post_item1.name_len = strlen(name) +1;
        printf("len:%d\n",http_post_item1.name_len);
        memcpy(http_post_item1.name,name,http_post_item1.name_len);

        http_post_item1.content_len = strlen(content);
        printf("len:%d\n",http_post_item1.content_len);
        http_post_item1.content = content;

        unsigned long filesize = get_file_size(path); //文件大小
        printf("file len:%d\n",filesize);
        char* request_pic = (char*)malloc(filesize);
        if(request_pic == NULL){
        	printf("malloc fail");
        	return -1;
        }
        printf("malloc ok");

     	FILE* fp = fopen(path, "rb+");  
	    if (fp == NULL){  
	        printf("open file fail!\r\n");  
	        return -1;  
	    }
        int readbyte = fread(request_pic, 1, filesize, fp);//读取上传的图片信息
        fclose(fp);  
       
      	http_post_item2.is_file = 1;
        http_post_item2.name_len = strlen(path)+1;
        memcpy(http_post_item2.name,path,http_post_item2.name_len);

       
        http_post_item2.content_len = filesize;
        http_post_item2.content = request_pic;

        http_post_add_item(&http_post,&http_post_item2);
        http_post_add_item(&http_post,&http_post_item1);

        http_post_sent(SERVER_ADDR, SERVER_PORT,SERVER_URL,&http_post);
        return 0;  

}

