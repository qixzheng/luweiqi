//此源代码不需要包含额外的库
#include <iostream>
#include <fstream>
#include <cmath>
#include <gl\glut.h>

#define NULL 0
#define OK 1
#define PI 3.1415926

using namespace std;

typedef unsigned long       DWORD;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef unsigned long       LONG;

//定义bmp读取
typedef struct tagBITMAPFILEHEADER {
	WORD    bfType;
	DWORD   bfSize;
	WORD    bfReserved1;
	WORD    bfReserved2;
	DWORD   bfOffBits;
} BITMAPFILEHEADER;
typedef struct tagBITMAPINFOHEADER {
	DWORD      biSize;
	LONG       biWidth;
	LONG       biHeight;
	WORD       biPlanes;
	WORD       biBitCount;
	DWORD      biCompression;
	DWORD      biSizeImage;
	LONG       biXPelsPerMeter;
	LONG       biYPelsPerMeter;
	DWORD      biClrUsed;
	DWORD      biClrImportant;
} BITMAPINFOHEADER;

typedef struct Vertex //点
{
	GLfloat v[3];
}Vertex, *Ver;
typedef struct Normal //法线
{
	GLfloat n[3];
}Normal, *Nor;
typedef struct Material //材质
{
	GLfloat ambient[4], diffuse[4], specular[4], emission[4], shininess;
	int index;
}Material, *Mat;
typedef struct Texture_loc //贴图坐标
{
	GLfloat t[2];
}Texture_loc, *Tex_loc;
typedef struct Model_info
{
	unsigned int info[9];
}Model_info, *Mod_info;
typedef struct Model //模型
{
	int submodel, index;
	Model_info info[2000];
}Model, *Mod;

typedef struct Mapping_file //贴图文件名
{
	char name[20];
}Mapping_file, *Map_file;

//全局变量（点、法向量、材质等的声明）
Ver vertex = NULL;
Nor normal = NULL;
Mat material = NULL;
Tex_loc texture_loc = NULL;
Mod model = NULL;
Map_file mapping_file = NULL;
GLuint *text;
int flag = 1; //1表示点，2表示线，3表示面

//平移
GLdouble tX = 0, tY = 0, tZ = 0;
//旋转
GLdouble rAngle = 0, rX = -1, rY = 1, rZ = 0;
//缩放
GLdouble sX, sY, sZ;
//投影
GLdouble theta = 0, aspect = 0, dnear = 0, dfar = 0;
//三维观察
GLdouble x_0 = 0.1, y_0 = 0.2, z_0 = 0, xref = -0.1, yref = -0.4, zref = 0, V_X = 0, V_Y = 0, V_Z = 1.0;
//参考指针
GLdouble *indexA = NULL, *indexB = NULL, *indexC = NULL, *indexD = NULL;

int LoadGLTextures(const char *FileName, GLuint *ttexture)
{
	//加载贴图
	glGenTextures(1, ttexture);
	char *Filename = new char[100];
	strcpy_s(Filename, 100, FileName);
	BITMAPINFOHEADER header;
	BYTE *texture;
	int width, height, m;
	unsigned char *image;
	ifstream File(Filename, ios::binary);
	if (File) {
		File.seekg(14, SEEK_SET);
		File.read((char *)&header, sizeof(BITMAPINFOHEADER));
	}
	else { cout << "Error opening " << Filename << " for input" << endl; system("pause"); exit(-1); }
	//读取长宽
	width = abs((long)header.biWidth);
	height = abs((long)header.biHeight);
	//判断位图类型并标记
	int Type;
	if (header.biBitCount == 32) Type = 4;
	else Type = 3;
	//为image分配像素空间
	image = new unsigned char[width*height*Type];

	File.read((char *)image, width*height*Type * sizeof(unsigned char));

	texture = new BYTE[width*height*Type];
	//对texture赋值
	if (Type == 4)
		for (m = 0; m < width*height; m++)
		{
			texture[4 * m] = image[4 * m + 2];
			texture[4 * m + 1] = image[4 * m + 1];
			texture[4 * m + 2] = image[4 * m];
			texture[4 * m + 3] = image[4 * m + 3];
		}
	else
		for (m = 0; m < width*height * 3; m++)
			texture[m] = image[m];
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glBindTexture(GL_TEXTURE_2D, *ttexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	if (Type == 4) glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture);
	else glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, texture);
	glBindTexture(GL_TEXTURE_2D, *ttexture);
	delete texture;
	delete image;
	delete Filename;
	File.close();
	return OK;
}
void light() {
	//光源
	glEnable(GL_LIGHT0);
	GLfloat Ambient[] = { 0.2f, 0.0f, 0.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, Ambient);
	GLfloat Diffuse[] = { 0.8f, 0.7f, 0.65f, 1.0f };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, Diffuse);
	GLfloat Specular[] = { 0.0f, 0.0f, 0.2f, 1.0f };
	glLightfv(GL_LIGHT0, GL_SPECULAR, Specular);
	GLfloat position[] = { 1.0f, 1.0f, 0.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, position);
	GLfloat spotDirection[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, spotDirection);
	GLfloat light[] = { 0.6f, 0.6f, 0.6f, 1.0f };
	glLightfv(GL_LIGHT0, GL_CONSTANT_ATTENUATION, light);
	GLfloat ambient_lightModel[] = { 0.4f, 0.4f, 0.4f, 1.0f };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient_lightModel);
	glEnable(GL_LIGHTING);
}

void init(void)
{
	//初始化
	glClearColor(0.20, 0.20, 0.20, 0.0);
	glMatrixMode(GL_MODELVIEW);
	glOrtho(-10, 10, -10, 10, -10, 10);
	light();
}

void lineSegment(void)
{
	//显示窗口
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluPerspective(theta * 5 * PI, aspect, dnear, dfar);
	gluLookAt(x_0, y_0, z_0, xref, yref, zref, V_X, V_Y, V_Z);
	glTranslatef(tX, tY, tZ);
	glRotatef(rAngle * 5 * PI, rX, rY, rZ);
	glScalef(sX, sY, sZ);
	//绘制坐标系
	glBegin(GL_LINES);
	glVertex3f(1.0, 0.0, 0.0);
	glVertex3f(-1.0, 0.0, 0.0);
	glVertex3f(0.0, 1.0, 0.0);
	glVertex3f(0.0, -1.0, 0.0);
	glVertex3f(0.0, 0.0, 1.0);
	glVertex3f(0.0, 0.0, -1.0);
	glEnd();
	for (int j = 0; j < 4; j++)
	{
		//加载贴图
		glMaterialfv(GL_FRONT, GL_AMBIENT, material[j].ambient);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, material[j].diffuse);
		glMaterialfv(GL_FRONT, GL_SPECULAR, material[j].specular);
		glMaterialfv(GL_FRONT, GL_EMISSION, material[j].emission);
		glMaterialf(GL_FRONT, GL_SHININESS, material[j].shininess);
		LoadGLTextures(mapping_file[j].name, &text[j]);
		glEnable(GL_TEXTURE_2D);
		for (int i = 0; i < model[j].submodel; i++)
		{
			//根据flag值选择绘制模式
			if (flag == 1) glBegin(GL_POINTS);
			else if (flag == 2) glBegin(GL_LINE_LOOP);
			else glBegin(GL_TRIANGLES);
			glNormal3fv(normal[model[j].info[i].info[2] - 1].n);
			glTexCoord2fv(texture_loc[model[j].info[i].info[1] - 1].t);
			glVertex3fv(vertex[model[j].info[i].info[0] - 1].v);
			glNormal3fv(normal[model[j].info[i].info[5] - 1].n);
			glTexCoord2fv(texture_loc[model[j].info[i].info[4] - 1].t);
			glVertex3fv(vertex[model[j].info[i].info[3] - 1].v);
			glNormal3fv(normal[model[j].info[i].info[8] - 1].n);
			glTexCoord2fv(texture_loc[model[j].info[i].info[7] - 1].t);
			glVertex3fv(vertex[model[j].info[i].info[6] - 1].v);
			glEnd();
		}
	}
	glFlush();
}

void keyBoardFunc(unsigned char key, int x, int y)
{
	//键盘处理函数
	if (key == '=') flag++;
	if (key == '-') flag--;
	if (flag == 4)flag = 1;
	if (flag == 0)flag = 3;
	//改变参数
	if (key == 'a')
		if (indexA) *indexA += 0.05;
	if (key == 'd')
		if (indexA) *indexA -= 0.05;
	if (key == 'b')
		if (indexB) *indexB += 0.05;
	if (key == 'f')
		if (indexB) *indexB -= 0.05;
	if (key == 'w')
		if (indexC) *indexC += 0.05;
	if (key == 's')
		if (indexC) *indexC -= 0.05;
	if (key == 'z')
		if (indexD) *indexD += 0.05;
	if (key == 'c')
		if (indexD) *indexD -= 0.05;
	system("cls");
	lineSegment();
}

void chooseMode(GLint menuIteemNum)
{
	//根据鼠标点击处理
	switch (menuIteemNum)
	{
	case 0:
		indexA = &tX; indexB = &tY; indexC = &tZ; indexD = NULL; break;
	case 1:
		indexA = &sX; indexB = &sY; indexC = &sZ; indexD = NULL; break;
	case 2:
		indexA = &rAngle; indexB = &rX; indexC = &rY; indexD = &rZ; break;
	case 3:
		indexA = &theta; indexB = &aspect; indexC = &dnear; indexD = &dfar; break;
	case 4:
		indexA = &x_0; indexB = &y_0; indexC = &z_0; indexD = NULL; break;
	case 5:
		indexA = &xref; indexB = &yref; indexC = &zref; indexD = NULL; break;
	case 6:
		indexA = &V_X; indexB = &V_Y; indexC = &V_Z; indexD = NULL; break;
	default:
		indexA = NULL; indexB = NULL; indexC = NULL; indexD = NULL; break;
	}
}



void mouseFunc(GLint button, GLint action, GLint xMouse, GLint yMouse)
{
	//右击鼠标显示一系列窗口
	glutCreateMenu(chooseMode);
	glutAddMenuEntry("平移", 0);
	glutAddMenuEntry("缩放", 1);
	glutAddMenuEntry("旋转", 2);
	glutAddMenuEntry("投影", 3);
	glutAddMenuEntry("视线", 4);
	glutAddMenuEntry("中心位置", 5);
	glutAddMenuEntry("z负方向", 6);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

int main(int argc, char** argv)
{
	//读取文件
	ifstream fp;
	fp.open("luweiqi.txt");
	int number;
	
	fp >> number;
	mapping_file = new Mapping_file[number];
	text = new GLuint[number];
	for (int i = 0; i < number; i++)
		fp>>mapping_file[i].name;
	fp >> number;
	material = new Material[number];
	for (int i = 0; i < number; i++)
	{
		for (int j = 0; j < 4; j++)
			fp >> material[i].ambient[j];
		for (int j = 0; j < 4; j++)
			fp >> material[i].diffuse[j];
		for (int j = 0; j < 4; j++)
			fp >> material[i].specular[j];
		for (int j = 0; j < 4; j++)
			fp >> material[i].emission[j];
		fp >> material[i].shininess >> material[i].index;
		material[i].index--;//index从1开始
	}
	fp >> number;
	vertex = new Vertex[number];
	for (int i = 0; i < number; i++)
	{
		fp >> vertex[i].v[0] >> vertex[i].v[1] >> vertex[i].v[2];
	}
	fp >> number;
	texture_loc = new Texture_loc[number];
	for (int i = 0; i < number; i++)
		fp >> texture_loc[i].t[0] >> texture_loc[i].t[1];
	fp >> number;
	normal = new Normal[number];
	for (int i = 0; i < number; i++)
		fp >> normal[i].n[0] >> normal[i].n[1] >> normal[i].n[2];
	fp >> number;
	fp >> sX >> sY >> sZ;
	model = new Model[number];
	for (int i = 0; i < number; i++)
	{
		fp >> model[i].submodel >> model[i].index;
		model[i].index--;
		for (int j = 0; j < model[i].submodel; j++)
			for (int k = 0; k < 9; k++)
				fp >> model[i].info[j].info[k];		
	}
	fp.close();
	//工作部分
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowPosition(50, 100);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Luweiqi");
	init();
	glutDisplayFunc(lineSegment);
	glutKeyboardFunc(keyBoardFunc);
	glutMouseFunc(mouseFunc);
	glutMainLoop();
	return 0;
}