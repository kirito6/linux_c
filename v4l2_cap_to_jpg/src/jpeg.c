#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jpeglib.h"
void write_YUV_JPEG_file (char * filename, unsigned char* yuvData, int quality,
				int image_width,int image_height)
{

		struct jpeg_compress_struct cinfo;

		struct jpeg_error_mgr jerr;

		FILE * outfile; /* target file */
		//JSAMPROW row_pointer[1]; /* pointer to JSAMPLE row[s] */
		//int row_stride; /* physical row width in image buffer */
		JSAMPIMAGE buffer;

		int band,i,buf_width[3],buf_height[3];
		cinfo.err = jpeg_std_error(&jerr);

		jpeg_create_compress(&cinfo);


		if ((outfile = fopen(filename, "wb")) == NULL) {
				fprintf(stderr, "can't open %s\n", filename);
				exit(1);
		}
		jpeg_stdio_dest(&cinfo, outfile);


		cinfo.image_width = image_width; /* image width and height, in pixels */
		cinfo.image_height = image_height;
		cinfo.input_components = 3; /* # of color components per pixel */
		cinfo.in_color_space = JCS_YCbCr; /* colorspace of input image */

		jpeg_set_defaults(&cinfo);

		jpeg_set_quality(&cinfo, quality, TRUE );

		//////////////////////////////
		cinfo.raw_data_in = TRUE;
		cinfo.jpeg_color_space = JCS_YCbCr;
		cinfo.comp_info[0].h_samp_factor = 2;
		cinfo.comp_info[0].v_samp_factor = 2;
		/////////////////////////

		jpeg_start_compress(&cinfo, TRUE);

		buffer = (JSAMPIMAGE) (*cinfo.mem->alloc_small) ((j_common_ptr) &cinfo,  
						JPOOL_IMAGE, 3 * sizeof(JSAMPARRAY));   
		for(band=0; band<3; band++)
		{
				buf_width[band] = cinfo.comp_info[band].width_in_blocks * DCTSIZE;
				buf_height[band] = cinfo.comp_info[band].v_samp_factor * DCTSIZE;
				buffer[band] = (*cinfo.mem->alloc_sarray) ((j_common_ptr) &cinfo,
								JPOOL_IMAGE, buf_width[band], buf_height[band]);
		}   

		unsigned char *rawData[3];
		rawData[0]=yuvData;
		rawData[1]=yuvData+image_width*image_height;
		rawData[2]=yuvData+image_width*image_height*5/4;

		int max_line = cinfo.max_v_samp_factor*DCTSIZE;   
		int counter;
		for(counter=0; cinfo.next_scanline < cinfo.image_height; counter++)
		{   
				//buffer image copy.
				for(band=0; band<3; band++)
				{
						int mem_size = buf_width[band];
						unsigned char *pDst = (unsigned char *) buffer[band][0];
						unsigned char *pSrc = (unsigned char *) (rawData[band] +
										counter*buf_height[band] * buf_width[band]);
						memcpy(pDst,pSrc,buf_width[band]*buf_height[band]);
					//	for(i=0; i<buf_height[band]; i++)
					//	{
					//			memcpy(pDst, pSrc, mem_size);
					//			pSrc += buf_width[band];
					//			pDst += buf_width[band];
					//	}
				}
				jpeg_write_raw_data(&cinfo, buffer, max_line);
		}
		jpeg_finish_compress(&cinfo);
		fclose(outfile);
		jpeg_destroy_compress(&cinfo);
}


