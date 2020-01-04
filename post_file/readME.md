# Easy post file

>modify:kirito 2019-11-10
>
>create:kirito 2019-11-10

## 1 easy to post msg or file to server

you can post msg  , or txt img file 

## 2 start

1 comile

``` 
arm-linux-gnueabihf-gcc   post_file.c  -o post_file
```

2 run

```
./post_file
```

## 3 how to post file

```c
HTTP_POST http_post;
HTTP_POST_ITEM http_post_item1;
HTTP_POST_ITEM http_post_item2;


// key & value
char name[]="key";
char content[] = "val";

http_post_item1.file_type = jpeg;
http_post_item1.is_file = 0;

http_post_item1.name_len = strlen(name) +1;
memcpy(http_post_item1.name,name,http_post_item1.name_len);
http_post_item1.content_len = strlen(content);
http_post_item1.content = content;

// file
char *path="img.jpg";
unsigned long filesize = get_file_size(path); //文件大小
char* request_pic = (char*)malloc(filesize);
if(request_pic == NULL){
    printf("malloc fail");
    return -1;
}
FILE* fp = fopen(path, "rb+");  
if (fp == NULL){  
    return -1;  
}
int readbyte = fread(request_pic, 1, filesize, fp);//读取上传的图片信息
fclose(fp);  

http_post_item2.is_file = 1;
http_post_item2.name_len = strlen(path)+1;
memcpy(http_post_item2.name,path,http_post_item2.name_len);
http_post_item2.content_len = filesize;
http_post_item2.content = request_pic;


// add item
http_post_add_item(&http_post,&http_post_item2);
http_post_add_item(&http_post,&http_post_item1);
// post to server . you can change ip and port in define
http_post_sent(SERVER_ADDR, SERVER_PORT,SERVER_URL,&http_post);
```



