#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/ioctl.h>
#include <linux/videodev2.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>

#define NR_BUFFER 4
void * vm_addr[NR_BUFFER];
void * framebuffer;
int capflags;

int xioctl(int fd, int request, void * arg) {
    int r;
    do
        r = ioctl(fd, request, arg);
    while (-1 == r && EINTR == errno);
    return r;
}

#define M_H 480
#define M_W 640

//-p preview
//-c capture jpeg
int main(int argc,char **argv)
{
		int fd, vd, fb, ret, i, type;
		char buffer[32];
		char *yuv_buf = NULL;
		struct fb_var_screeninfo fb_var;
		struct fb_fix_screeninfo fb_fix;
		struct v4l2_capability cap;
		struct v4l2_fmtdesc fmt;
		struct v4l2_frmsizeenum fsize;
		struct v4l2_frmivalenum fival;
		struct v4l2_streamparm sparm;

		parse_args(argc,argv);

		if(!capflags){ //preview init fb
				fb = open("/dev/fb0",O_RDWR);
				if(fb < 0){
						perror("open fb");
						exit(1);
				}
				if(xioctl(fb, FBIOGET_VSCREENINFO, &fb_var) < 0){
						perror("get var");
						exit(1);
				}
				if(xioctl(fb, FBIOGET_FSCREENINFO, &fb_fix) < 0){
						perror("get fix");
						exit(1);
				}
				framebuffer = mmap(NULL,fb_var.xres*fb_var.yres*fb_var.bits_per_pixel/8 * 2,PROT_WRITE,MAP_SHARED,fb,0);
				if(framebuffer == MAP_FAILED){
						perror("fb map");
						exit(1);
				}
		}

		//video init
		vd = open( "/dev/video0", O_RDWR);
		if(vd < 0){
				perror("open cam ");
				exit(1);
		}

		if(xioctl(vd,VIDIOC_QUERYCAP,&cap) < 0){
				perror("cam capability");
				exit(1);
		}
		printf("capability driver %s card %s businfo %s\n",cap.driver,cap.card,cap.bus_info);

		struct v4l2_format vfmt;
		memset(&vfmt,0,sizeof(vfmt));
		vfmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		vfmt.fmt.pix.width = M_W;
		vfmt.fmt.pix.height = M_H;
		vfmt.fmt.pix.pixelformat =V4L2_PIX_FMT_YUYV;// V4L2_PIX_FMT_SBGGR8;//V4L2_PIX_FMT_YUV32;// V4L2_PIX_FMT_YUV420;//V4L2_PIX_FMT_YUYV;//V4L2_PIX_FMT_NV12;//V4L2_PIX_FMT_YUYV;
		vfmt.fmt.pix.field = V4L2_FIELD_ANY;
		if(xioctl(vd,VIDIOC_S_FMT,&vfmt) < 0){
				perror("cam set fmt");
				exit(1);
		}
		printf("cam set width %d height %d\n",vfmt.fmt.pix.width,vfmt.fmt.pix.height);
		memset(&sparm,0,sizeof(sparm));
		sparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		sparm.parm.capture.capturemode = 0;
		sparm.parm.capture.timeperframe.numerator = 1;
		sparm.parm.capture.timeperframe.denominator = 15;
		if(xioctl(vd,VIDIOC_S_PARM,&sparm) < 0){
				perror("cam s parm");
				exit(1);
		}


		struct v4l2_requestbuffers reqb;
		memset(&reqb,0,sizeof(reqb));
		reqb.count = NR_BUFFER;
		reqb.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		reqb.memory = V4L2_MEMORY_MMAP;
		if(xioctl(vd,VIDIOC_REQBUFS,&reqb) < 0){
				perror("cam req buf");
				exit(1);
		}
		struct v4l2_buffer vbuffer;
		for(i = 0; i < NR_BUFFER; i++){
				memset(&vbuffer,0,sizeof(vbuffer));
				vbuffer.index = i;
				vbuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				vbuffer.memory = V4L2_MEMORY_MMAP;
				if(xioctl(vd,VIDIOC_QUERYBUF,&vbuffer) < 0){
						perror("cam querybuf");
						exit(1);
				}
				vm_addr[i] = mmap(NULL,vbuffer.length,PROT_READ,MAP_SHARED,vd,vbuffer.m.offset);
				if(vm_addr[i] == MAP_FAILED){
						perror("cam map ");
						exit(1);
				}
		}
		for(i = 0;i < NR_BUFFER; i++){
				memset(&vbuffer,0,sizeof(vbuffer));
				vbuffer.index = i;
				vbuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				vbuffer.memory = V4L2_MEMORY_MMAP;
				if(xioctl(vd,VIDIOC_QBUF,&vbuffer) < 0){
						perror("cam qbuf");
						exit(1);

				}

		}

		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if(xioctl(vd,VIDIOC_STREAMON,&type) < 0){
				perror("cam streamon");
				exit(1);
		}

		if(capflags){
				memset(&vbuffer,0,sizeof(vbuffer));
				vbuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				vbuffer.memory = V4L2_MEMORY_MMAP;
				if(xioctl(vd,VIDIOC_DQBUF,&vbuffer) < 0){
						perror("cam dqbuf");
						exit(1);
				}
				/* ******** */
				yuv_buf = malloc(vfmt.fmt.pix.width * vfmt.fmt.pix.height * 3 / 2);
				yuyv2yuv(yuv_buf,vm_addr[vbuffer.index],vfmt.fmt.pix.width,vfmt.fmt.pix.height);
				write_YUV_JPEG_file("test.jpg",
								yuv_buf,
								60,
								vfmt.fmt.pix.width ,
								vfmt.fmt.pix.height 
								);
				/* *******  */

		} else {

				memset(&vbuffer,0,sizeof(vbuffer));
				vbuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				vbuffer.memory = V4L2_MEMORY_MMAP;
				while(1){
						if(xioctl(vd,VIDIOC_DQBUF,&vbuffer) < 0){
								perror("cam dqbuf");
								exit(1);
						}
						if(vbuffer.index  == 0 ){

								yuv2rgb(vm_addr[vbuffer.index],framebuffer,M_W,M_H);
								fb_var.yoffset = 0;
								if(xioctl(fb,FBIOPAN_DISPLAY,&fb_var) < 0){
										perror("fb pan 0");
								}

						} else if( vbuffer.index == 2 ){
								yuv2rgb(vm_addr[vbuffer.index],framebuffer + 800*480 * 2,M_W,M_H);
								fb_var.yoffset = M_H;
								if(xioctl(fb,FBIOPAN_DISPLAY,&fb_var) < 0){
										perror("fb pan 1");
								}
						}
						if(xioctl(vd,VIDIOC_QBUF,&vbuffer) < 0){
								perror("cam qbuf");
								exit(1);
						}
				}
		}

		if(xioctl(vd, VIDIOC_STREAMOFF, &type) < 0){
				perror("cam stream off");
				exit(1);
		}
		//FIXME  add unmap to video and fb,if process not exit 
		close(vd);
		return 0;
}

