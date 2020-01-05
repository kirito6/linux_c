void yuyv2yuv(unsigned char *yuv, unsigned char *yuyv, int width, int height)
{
		int i,j; //p
		printf("w %d h %d\n",width,height);
		for(i = 0; i < height; i += 2){
				for(j = 0; j < width; j += 2){
						yuv[i*width + j] 				= yuyv[i*width*2 + j*2 ];
						yuv[i*width + j + 1] 			= yuyv[i*width*2 + j*2 + 2];
						yuv[i*width + j + width] 		= yuyv[i*width*2 + j*2 + width*2];
						yuv[i*width + j + width + 1] 	= yuyv[i*width*2 + j*2 + width*2 + 2];

						yuv[width * height + i*width/4 + j/2 ] = yuyv[i*width*2 + j*2 +1];
						yuv[width * height*5/4 + i*width/4 + j/2 ] = yuyv[i*width*2 + j*2 +3];
						//			yuv[i / 4 + width * height * 5 / 4] = yuyv[ i * 2 + 7]; 
				}

		}
		printf("done\n");
}
