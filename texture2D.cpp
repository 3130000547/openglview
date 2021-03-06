
// need: NVIDIA SDK 9.5, vfw32.lib


#if defined(WIN32)
#  include <windows.h>
#endif
// for sprintf
#pragma warning(disable: 4996)
#define GLH_EXT_SINGLE_FILE
#include <glh/glh_extensions.h>

#include "logger.h"
Logger *logger;

#include <glh/glh_glut.h>

#include <shared/timer.h>
timer fps(10);

# include <shared/moviemaker.h>
static MovieMaker   *lpMovie = NULL;

#include "checker.h"

bool b[256];
int win_w = 480, win_h = 480;
bool bPersp = false;
float whRatio = 1.0f;
float fRotate = 0.0f; //rotation
bool bAnim = false; // control rotation

using namespace glh;
glut_simple_mouse_interactor object;

void updateView(int width, int height);


// check for OpenGL errors
void checkGLError()
{
    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR) {
		char msg[512];
		sprintf(msg, "error - %s\n", (char *) gluErrorString(error));
		logger->update(msg);
    }
}

void initSetting()
{
    b[' '] = false;
	b['l'] = true;  //控制光照Enable 开启时是红光
	b['t'] = true;  //控制纹理Enable
    b['w'] = true;	//线框模式
	b['1'] = true;  //纹理切换，texture[0]和texture[2]
	//b['2'] = false;  //控制混合纹理的开启

}

const int TEXDIM = 256;

#define BITMAP_ID 0x4D42

// 纹理标示符数组，保存两个纹理的标示符
unsigned int		texture[2];

// 描述: 通过指针，返回filename 指定的bitmap文件中数据。
//       同时也返回bitmap信息头.（不支持8-bit位图）
unsigned char *LoadBitmapFile(char *filename, BITMAPINFOHEADER *bitmapInfoHeader) 
{
	FILE *filePtr;	// 文件指针
	BITMAPFILEHEADER bitmapFileHeader;	// bitmap文件头
	unsigned char	*bitmapImage;		// bitmap图像数据
	int	imageIdx = 0;		// 图像位置索引
	unsigned char	tempRGB;	// 交换变量

	// 以“二进制+读”模式打开文件filename 
	filePtr = fopen(filename, "rb"); 
	if (filePtr == NULL) return NULL;
	// 读入bitmap文件图
	fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr); 
	// 验证是否为bitmap文件
	if (bitmapFileHeader.bfType != BITMAP_ID) {
		fprintf(stderr, "Error in LoadBitmapFile: the file is not a bitmap file\n");
		return NULL;
	}

	// 读入bitmap信息头
	fread(bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr); 
	// 将文件指针移至bitmap数据
	fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);
	// 为装载图像数据创建足够的内存
	bitmapImage = new unsigned char[bitmapInfoHeader->biSizeImage]; 
	// 验证内存是否创建成功
	if (!bitmapImage) {
		fprintf(stderr, "Error in LoadBitmapFile: memory error\n");
		return NULL;
	}

	// 读入bitmap图像数据
	fread(bitmapImage, 1, bitmapInfoHeader->biSizeImage, filePtr); 
	// 确认读入成功
	if (bitmapImage == NULL) {
		fprintf(stderr, "Error in LoadBitmapFile: memory error\n");
		return NULL;
	}

	//由于bitmap中保存的格式是BGR，下面交换R和B的值，得到RGB格式
	for (imageIdx = 0; 
	 imageIdx < bitmapInfoHeader->biSizeImage; imageIdx+=3) { 
		tempRGB = bitmapImage[imageIdx]; 
		bitmapImage[imageIdx] = bitmapImage[imageIdx + 2]; 
		bitmapImage[imageIdx + 2] = tempRGB; 
	}
	// 关闭bitmap图像文件
	fclose(filePtr); 
	return bitmapImage; 
}

void glutSolidCube2(float size)
{

	glPushMatrix();
	glScalef(size, size, size);
	glColor3f(1.0, 1.0, 1.0f);
	glBegin(GL_QUADS); //下表面
	glTexCoord2f(1.0f, 1.0f); 
	//glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1.0f, 1.0f);
	//glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 1.0f, 1.0f);
	glVertex3f(0.5, -0.5, -0.5);
	glTexCoord2f(1.0f, 0.0f);
	//glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1.0f, 0.0f);
	//glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 1.0f, 0.0f);
	glVertex3f(0.5, -0.5, 0.5);
	glTexCoord2f(0.0f, 0.0f); 
	//glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0f, 0.0f);
	//glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 0.0f, 0.0f);
	glVertex3f(-0.5, -0.5, 0.5);
	glTexCoord2f(0.0f, 1.0f); 
	//glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0f, 1.0f);
	//glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 0.0f, 1.0f);
	glVertex3f(-0.5, -0.5, -0.5);
	glEnd();

	glBegin(GL_QUADS); //上表面
	glTexCoord2f(1.0f, 0.0f); 
	//glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1.0f, 0.0f);
	//glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 1.0f, 0.0f);
	glVertex3f(0.5, 0.5, 0.5);
	glTexCoord2f(1.0f, 1.0f); 
	//glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1.0f, 1.0f);
	//glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 1.0f, 1.0f);
	glVertex3f(0.5, 0.5, -0.5);
	glTexCoord2f(0.0f, 1.0f); 
	//glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0f, 1.0f);
	//glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 0.0f, 1.0f);
	glVertex3f(-0.5, 0.5, -0.5);
	glTexCoord2f(0.0f, 0.0f);
	//glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0f, 0.0f);
	//glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 0.0f, 0.0f);
	glVertex3f(-0.5, 0.5, 0.5);
	glEnd();

	glBegin(GL_QUADS);  //前
	glTexCoord2f(0.0f, 0.0f); 
	//glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0f, 0.0f);
	//glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 0.0f, 0.0f);
	glVertex3f(-0.5, -0.5, 0.5);
	glTexCoord2f(1.0f, 0.0f); 
	//glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1.0f, 0.0f);
	//glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 1.0f, 0.0f);
	glVertex3f(0.5, -0.5, 0.5);
	glTexCoord2f(1.0f, 1.0f); 
	//glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1.0f, 1.0f);
	//glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 1.0f, 1.0f);
	glVertex3f(0.5, 0.5, 0.5);
	glTexCoord2f(0.0f, 1.0f); 
	//glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0f, 1.0f);
	//glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 0.0f, 1.0f);
	glVertex3f(-0.5, 0.5, 0.5);
	glEnd();

	glBegin(GL_QUADS);  //后
	glTexCoord2f(0.0f, 0.0f); 
	//glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0f, 0.0f);
	//glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 0.0f, 0.0f);
	glVertex3f(-0.5, -0.5, -0.5);
	glTexCoord2f(1.0f, 0.0f);
	//glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1.0f, 0.0f);
	//glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 1.0f, 0.0f);
	glVertex3f(0.5, -0.5, -0.5);
	glTexCoord2f(1.0f, 1.0f); 
	//glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1.0f, 1.0f);
	//glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 1.0f, 1.0f);
	glVertex3f(0.5, 0.5, -0.5);
	glTexCoord2f(0.0f, 1.0f); 
	//glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0f, 1.0f);
	//glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 0.0f, 1.0f);
	glVertex3f(-0.5, 0.5, -0.5);
	glEnd();

	glBegin(GL_QUADS);    //left
	glTexCoord2f(0.0f, 0.0f); 
	//glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0f, 0.0f);
	//glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 0.0f, 0.0f);
	glVertex3f(-0.5, -0.5, -0.5);
	glTexCoord2f(1.0f, 0.0f); 
	//glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1.0f, 0.0f);
	//glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 1.0f, 0.0f);
	glVertex3f(-0.5, -0.5, 0.5);
	glTexCoord2f(1.0f, 1.0f); 
	//glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1.0f, 1.0f);
	//glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 1.0f, 1.0f);
	glVertex3f(-0.5, 0.5, 0.5);
	glTexCoord2f(0.0f, 1.0f);
	//glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0f, 1.0f);
	//glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 0.0f, 1.0f);
	glVertex3f(-0.5, 0.5, -0.5);
	glEnd();

	glBegin(GL_QUADS);    //right
	glTexCoord2f(0.0f, 0.0f); 
	//glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0f, 0.0f);
	//glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 0.0f, 0.0f);
	glVertex3f(0.5, -0.5, -0.5);
	glTexCoord2f(1.0f, 0.0f); 
	//glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1.0f, 0.0f);
	//glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 1.0f, 0.0f);
	glVertex3f(0.5, -0.5, 0.5);
	glTexCoord2f(1.0f, 1.0f); 
	//glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1.0f, 1.0f);
	//glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 1.0f, 1.0f);
	glVertex3f(0.5, 0.5, 0.5);
	glTexCoord2f(0.0f, 1.0f); 
	//glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0f, 1.0f);
	//glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 0.0f, 1.0f);
	glVertex3f(0.5, 0.5, -0.5);
	glEnd();

	glPopMatrix();

}

void Draw_Leg()
{
	glScalef(1, 1, 3);
	glutSolidCube2(1.0);
}

void draw()
{
	// 从已转载纹理中选择当前纹理
	if (b['1'])
		glBindTexture(GL_TEXTURE_2D, texture[0]);
	else
		glBindTexture(GL_TEXTURE_2D, texture[2]);
	// 将当前颜色设置为白色（重要），可以试试设为红色效果如何
	if (b['r'])
		glColor3f(1.0f,0.0f,0.0f); 
	else
		glColor3f(1.0f,1.0f,1.0f);   //设置为红色时是光照和纹理混合的效果吗？？？？？？？？不是	
	
	glPushMatrix();
	glTranslatef(0, 0, 4 + 1);
	glRotatef(90, 1, 0, 0);
	glutSolidTeapot(1);
	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, texture[1]);

	//else    //混合纹理
	//{
	//	glActiveTextureARB(GL_TEXTURE0_ARB);
	//	glEnable(GL_TEXTURE_2D);
	//	glBindTexture(GL_TEXTURE_2D, texture[3]);

	//	glActiveTextureARB(GL_TEXTURE1_ARB);
	//	glEnable(GL_TEXTURE_2D);
	//	glBindTexture(GL_TEXTURE_2D, texture[1]);
	//}
	glPushMatrix();
	glTranslatef(0, 0, 3.5);
	glScalef(5, 4, 1);
	glutSolidCube2(1.0);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(1.5, 1, 1.5);
	Draw_Leg();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-1.5, 1, 1.5);
	Draw_Leg();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(1.5, -1, 1.5);
	Draw_Leg();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-1.5, -1, 1.5);
	Draw_Leg();
	glPopMatrix();

	//glActiveTextureARB(GL_TEXTURE1_ARB);
	glDisable(GL_TEXTURE_2D);
	//glActiveTextureARB(GL_TEXTURE0_ARB);
	//glDisable(GL_TEXTURE_2D);
	
}

void initOpengl()
{
	glClearColor(0.0, 0.0, 0.0, 1.0);

	// initialize OpenGL lighting  光源信息
	GLfloat matAmb[4] =    {0.2, 0.2, 0.2, 1.0};
	GLfloat matDiff[4] =   {1.0, 0.1, 0.2, 1.0};
	GLfloat matSpec[4] =   {1.0, 1.0, 1.0, 1.0};

	GLfloat lightPos[] =   {10.0, 10.0, 10.0, 0.0};
	GLfloat lightAmb[4] =  {0.0, 0.0, 0.0, 1.0};
	GLfloat lightDiff[4] = {1.0, 1.0, 1.0, 1.0};
	GLfloat lightSpec[4] = {1.0, 1.0, 1.0, 1.0};

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, matAmb);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, matDiff);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, matSpec);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 60.0);

	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

	glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmb);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiff);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpec);

	glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL_EXT, GL_SEPARATE_SPECULAR_COLOR_EXT);
	GLfloat black[] =  {0.0, 0.0, 0.0, 1.0};
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, black);

	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);

	BITMAPINFOHEADER bitmapInfoHeader; // bitmap信息头
	BITMAPINFOHEADER bitmapInfoHeader2; // bitmap信息头
	unsigned char*   bitmapData;         // 纹理数据
	unsigned char*   bitmapData2;         // 纹理数据
	BITMAPINFOHEADER bitmapInfoHeader3; // bitmap信息头
	unsigned char*   bitmapData3;         // 纹理数据
	bitmapData=NULL; 
	bitmapData2 = NULL;
	bitmapData3 = NULL;
	bitmapData = LoadBitmapFile("Monet.bmp", &bitmapInfoHeader);
	bitmapData2 = LoadBitmapFile("Crack.bmp", &bitmapInfoHeader2);
	bitmapData3 = LoadBitmapFile("Spot.bmp", &bitmapInfoHeader3);

	// 第一参数是需要生成标示符的个数
	// 第二参数是返回标示符的数组
	glGenTextures(4, texture);  
	glBindTexture(GL_TEXTURE_2D, texture[0]);  
	// 指定当前纹理的放大/缩小过滤方式
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 
		0, 		//mipmap层次 (通常为0，表示最上层) 
		GL_RGB,		//我们希望该纹理有红、绿、蓝数据
		bitmapInfoHeader.biWidth,	//纹理宽带，必须是2n，若有边框+2 
		bitmapInfoHeader.biHeight,	//纹理高度，必须是2n，若有边框+2 
		0,				//边框 (0=无边框, 1=有边框) 
		GL_RGB,				//bitmap数据的格式
		GL_UNSIGNED_BYTE,		//每个颜色数据的类型
		bitmapData);			//bitmap数据指针

	/*绑定第二个纹理 进行过滤*/
	glBindTexture(GL_TEXTURE_2D, texture[1]);
	// 指定当前纹理的放大/缩小过滤方式
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D,
		0, 		//mipmap层次 (通常为0，表示最上层) 
		GL_RGB,		//我们希望该纹理有红、绿、蓝数据
		bitmapInfoHeader2.biWidth,	//纹理宽带，必须是2n，若有边框+2 
		bitmapInfoHeader2.biHeight,	//纹理高度，必须是2n，若有边框+2 
		0,				//边框 (0=无边框, 1=有边框) 
		GL_RGB,				//bitmap数据的格式
		GL_UNSIGNED_BYTE,		//每个颜色数据的类型
		bitmapData2);			//bitmap数据指针

	/*绑定第三个纹理进行设置*/
	makeCheckImage();
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
#ifdef GL_VERSION_1_1
	
	glBindTexture(GL_TEXTURE_2D, texture[2]);
#endif

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#ifdef GL_VERSION_1_1
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, checkImageWidth, checkImageHeight,
		0, GL_RGBA, GL_UNSIGNED_BYTE, checkImage);
#else
	glTexImage2D(GL_TEXTURE_2D, 0, 4, checkImageWidth, checkImageHeight,
		0, GL_RGBA, GL_UNSIGNED_BYTE, checkImage);
#endif

	/*绑定第四个纹理进行设置*/
	glBindTexture(GL_TEXTURE_2D, texture[3]);
	// 指定当前纹理的放大/缩小过滤方式
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D,
		0, 		//mipmap层次 (通常为0，表示最上层) 
		GL_RGB,		//我们希望该纹理有红、绿、蓝数据
		bitmapInfoHeader3.biWidth,	//纹理宽带，必须是2n，若有边框+2 
		bitmapInfoHeader3.biHeight,	//纹理高度，必须是2n，若有边框+2 
		0,				//边框 (0=无边框, 1=有边框) 
		GL_RGB,				//bitmap数据的格式
		GL_UNSIGNED_BYTE,		//每个颜色数据的类型
		bitmapData3);			//bitmap数据指针
	
}

void updateFPS()
{
    char buf[512];
    fps.frame();
    if (fps.timing_updated()) {
        sprintf(buf, "Texture2D - FPS: %.1f", fps.get_fps());

		glutSetWindowTitle(buf);
	}
}

void begin_window_coords()
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    //glOrtho(0.0, win_w, 0.0, win_h, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void end_window_coords()
{
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

float eye[] = { 0, 0, 8 };
float center[] = { 0, 0, 0 };

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //glMatrixMode(GL_MODELVIEW);  //??
    glLoadIdentity();
    //??object.apply_transform();
	//** add camera setting here.

		gluLookAt(eye[0], eye[1], eye[2],
			center[0], center[1], center[2],
			0, 1, 0);				// 场景(0，0，0)的视点中心 (0,5,50)，Y轴向上
    // draw scene
	if (!b['w'])
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // draw scene
	if (b['h']) {   //开关灯，实现光照、纹理混合 初始时灯是关的？
		glDisable(GL_LIGHTING);
	}
	else
		glEnable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);

	if (b['t'])
		glEnable(GL_TEXTURE_2D);

	glRotatef(fRotate, 0, 1.0f, 0);			// Rotate around Y axis
	glRotatef(-90, 1, 0, 0);
	glScalef(0.2, 0.2, 0.2);

	draw();

	if (bAnim) fRotate += 0.5f;

	glDisable(GL_TEXTURE_2D);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glutSwapBuffers();
	updateFPS();
	checkGLError();
    if(b['x'])  
        lpMovie->Snap();
}

void dynamic()
{
}

void idle()
{
    if (b[' '])
        object.trackball.increment_rotation();

	if (b['d'])
		dynamic();

    glutPostRedisplay();
}

void key(unsigned char k, int x, int y)
{
	b[k] = ! b[k];

	switch (k) {
	case 27:
	case 'q':
		if (lpMovie) {
			lpMovie->EndCapture();
			delete lpMovie;
		}
		exit(0);
		break;

	case 'x':
	{
				static int mcnt = 0;
				if (b['x'])
				{
					char strtmp[100];
					sprintf(strtmp, "movie_%d.avi", mcnt++);
					lpMovie = new MovieMaker;
					lpMovie->StartCapture(strtmp);

					char msg[512];
					sprintf(msg, "starting recording %s", strtmp);
					logger->update(msg);
				}
				else
				{
					logger->update("stopped recording.");
					lpMovie->EndCapture();
					delete lpMovie;
					lpMovie = NULL;
				}
				break;
	}
	case 'p':{bPersp = !bPersp;  break; }
	case 'j':{eye[0] -= 0.2f; center[0] -= 0.2f; break; }
	case 'l':{eye[0] += 0.2f; center[0] += 0.2f; break; }
	case 'i':{eye[1] -= 0.2f; center[1] -= 0.2f; break; }
	case 'k':{eye[1] += 0.2f; center[1] += 0.2f; break; }
	case 'z':{eye[2] -= 0.2f; center[2] -= 0.2f; break; }
	case 'c':{eye[2] += 0.2f; center[2] += 0.2f; break; }
	case ' ':{bAnim = !bAnim; break; }
	}
	
	updateView(win_h, win_w);
    object.keyboard(k, x, y);    
	glutPostRedisplay();
}

void updateView(int width, int height)
{
	glViewport(0, 0, width, height);						// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix
	//printf("!!!begin\n");
	whRatio = (GLfloat)width / (GLfloat)height;
	if (bPersp) {
		//printf("perspective ");
		//printf("%f", whRatio);
		gluPerspective(45.0f, whRatio, 0.1f, 100.0f);
		//glFrustum(-3, 3, -3, 3, 3,100);
	}
	else {
		glOrtho(-3, 3, -3, 3, -100, 100);
	}

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
}

void reshape(int width, int height)
{
	if (height == 0)										// Prevent A Divide By Zero By
	{
		height = 1;										// Making Height Equal One
	}

	win_h = height;
	win_w = width;

	updateView(win_h, win_w);
}


void mouse(int button, int state, int x, int y)
{
    object.mouse(button, state, x, y);
}

void motion(int x, int y)
{
    object.motion(x, y);
}

void main_menu(int i)
{
    key((unsigned char) i, 0, 0);
}

void initMenu()
{    
    glutCreateMenu(main_menu);
    glutAddMenuEntry("Toggle wireframe [w]", 'w');
	glutAddMenuEntry("Toggle lighting [l]", 'l');
    glutAddMenuEntry("Quit [esc]", '\033');
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

bool initWin(int argc, char **argv)
{
	if (argc > 1) {
		int rx = 0, ry = 0;

		for (int i=1; i<argc; i++)
			if (!strcmp(argv[i], "rx") && (i+1) < argc)
				rx = atol(argv[i+1]);
			else if (!strcmp(argv[i], "ry") && (i+1) < argc)
				ry = atol(argv[i+1]);

		char mode[128];
		sprintf(mode, "%dx%d:32", rx, ry);
		glutGameModeString(mode);
		if (glutGameModeGet(GLUT_GAME_MODE_POSSIBLE))
			glutEnterGameMode(); 
		else {
			char msg[512];
			sprintf(msg, "unsupported resolution: %s", mode);
			logger->update(msg);
			logger->update("return to window mode (512x512)");
			return false;
		}

		return true;
	}

	return false;
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA | GLUT_STENCIL);

	logger = new Logger("logger.txt");
	if (!initWin(argc, argv)) {
		glutInitWindowSize(480, 480);
		glutCreateWindow("Model Viewer");
	}

	initOpengl();


    object.configure_buttons(1);
    object.dolly.dolly[2] = -3;
    object.trackball.incr = rotationf(vec3f(1, 1, 0), 0.05);

	glutDisplayFunc(display);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutIdleFunc(idle);
    glutKeyboardFunc(key);
    glutReshapeFunc(reshape);

    initMenu();

	initSetting();

	glutMainLoop();

	return 0;
}
