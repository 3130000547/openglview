#include <GL\glut.h>
#include <stdlib.h>
#include <stdio.h>

/*  Create checkerboard texture */
#define checkImageWidth 64
#define checkImageHeight 64
static GLubyte checkImage[checkImageHeight][checkImageWidth][4];

#ifdef GL_VERSION_1_1
static GLuint texName;
#endif
void makeCheckImage(void)
{
	int i, j, c;
	for (i = 0; i < checkImageHeight; i++){
		for (j = 0; j < checkImageWidth; j++){
			c = ((((i & 0x8) == 0) ^ ((j & 0x8)) == 0)) * 255;  //生成8*8的棋盘纹理
			checkImage[i][j][0] = (GLubyte)c;
			checkImage[i][j][1] = (GLubyte)c;
			checkImage[i][j][2] = (GLubyte)c;
			checkImage[i][j][3] = (GLubyte)255;
		}
	}
}