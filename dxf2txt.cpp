#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cmath>

using namespace std;

const int LINEMAX = 200000;
const int ARCMAX = 100000;
const double E = 1e-10;
const double PI = 3.1415926535897932384626;

struct Point
{
	double x, y;
};

struct Line
{
	Point a, b;
	string name;
	int id;
}allLine[LINEMAX];
int lineCnt;

struct Arc
{
	Point a, b, center;
	bool clockwise;
	string name;
	int id;
}allArc[ARCMAX];
int arcCnt;

int polyCnt;

void addLine(Point a, Point b, string name)   //添加边
{
	allLine[++lineCnt].a = a;
	allLine[lineCnt].b = b;
	allLine[lineCnt].name = name;
	allLine[lineCnt].id = polyCnt;
}

void addArc(Point a, Point b, double convexity, string name)	//添加圆弧
{
	Arc nowArc;
	if (convexity > 0)	nowArc.clockwise = false;
	else	nowArc.clockwise = true;
	nowArc.a = a;
	nowArc.b = b;
	double theta_arc = atan(convexity) * 4;
    double theta_degree = theta_arc * 180 / PI;

	if (fabs(180 - fabs(theta_degree)) < 0.1)   //半圆，直接计算中心
    {
    	nowArc.center.x = (a.x + b.x)/2;
       	nowArc.center.y = (a.y + b.y)/2;
	}
	else
	{
		double length = sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));	//弦长
		double dfR = length / (2.0 * sin(fabs(theta_arc) / 2.0));	//半径
		Point cen1, cen2;	//暂存两个圆心
		if (b.y == a.y)	//中垂线平行于y轴
		{
			double dfDiff = dfR * dfR - pow((b.x - a.x) / 2, 2);
			if (dfDiff < 0)	return;
			cen2.x = cen1.x = (a.x + b.x) / 2;
			cen1.y = a.y + sqrt(dfDiff);
			cen2.y = a.y - sqrt(dfDiff);
		}
		else	//中垂线 y = kx + bk
		{
			double k = -(b.x - a.x) / (b.y - a.y);	//中垂线斜率
			double bk = 0.5 * (b.y * b.y - a.y * a.y + b.x * b.x - a.x * a.x) / (b.y - a.y);
			double p = k * k + 1;
			double q = 2 * (k * (bk - a.y));
			double r = a.x * a.x + (bk - a.y) * (bk - a.y) - dfR * dfR;
			double delta = q * q - 4 * p * r;
			if (delta < 0)	return;
			cen1.x = (-q + sqrt(delta)) / (2 * p);
			cen1.y = k * cen1.x + bk;
			cen2.x = (-q - sqrt(delta)) / (2 * p);
			cen2.y = k * cen2.x + bk;
			
		}
		//根据矢量判断 P0为起点，P2为终点，P1为圆心（即判断P1P0 差乘 P1P2）
		double dfDiff = (a.x - cen1.x) * (b.y - cen1.y) - (b.x - cen1.x) * (a.y - cen1.y);
		if (fabs(theta_arc) < PI)
			if (dfDiff * theta_arc > 0)	nowArc.center = cen1;
			else	nowArc.center = cen2;
		else if (dfDiff * theta_arc < 0)	nowArc.center = cen1;
		else	nowArc.center = cen2; 
	}
	nowArc.name = name;
	nowArc.id = polyCnt;

	allArc[++arcCnt] = nowArc;
}

void inputInt(ifstream& fin, int &x, int key, string errStr)
{
	int a;
	fin >> a;
	if (a == key)
	{
		fin >> x;
		return;
	}
	cout << errStr << endl;
	exit(0);
}

void inputDouble(ifstream& fin, double &x, int key, string errStr)
{
	int a;
	fin >> a;
	if (a == key)
	{
		fin >> x;
		return;
	}
	cout << errStr << endl;
	exit(0);
}

void inputStr(ifstream& fin, string &x, int key, string errStr)
{
	int a;
	fin >> a;
	cout << a << ' ';
	if (a == key)
	{
		getline (fin, x);
		getline (fin, x);
		cout << x << endl;
		return;
	}
	cout << errStr << ' ' << a << endl;
	exit(0);
}

void inputedStr(ifstream& fin, int a, string &x, int key, string errStr)
{
	if (a == key)
	{
		getline (fin, x);
		getline (fin, x);
		cout << x << endl;
		return;
	}
	cout << errStr << ' ' << a << endl;
	exit(0);
}

int getEntitiesLwPolyline(ifstream& fin)   //读连线信息，按照折线的方式读入
{
	int a, sum, close, polyType;      //a为里面的预设参数，sum为统计点的数量，close为统计是否闭环
	string polyNum, subClass, name, subClass2;    //都是dxf文件的定义参数
	double lineWidth, convexity;
	Point start, now, last;   //开始节点的x,y;正在读入的节点的x,y;上一个节点的x.y;

	polyCnt++;
	inputStr(fin, polyNum, 5, "error polyNum");
	inputInt(fin, polyType, 330, "error polyType");
	inputStr(fin, subClass, 100, "error subClass");
	inputStr(fin, name, 8, "error name");
	inputStr(fin, subClass2, 100, "error subClass2");

	if (subClass2 == "AcDbLine")
	{
		double tmp;
		inputDouble(fin, start.x, 10, "error x");
		inputDouble(fin, start.y, 20, "error y");
		inputDouble(fin, tmp, 30, "error z");
		inputDouble(fin, now.x, 11, "error x");
		inputDouble(fin, now.y, 21, "error y");
		inputDouble(fin, tmp, 31, "error z");
		addLine(start, now, name);
		
		fin >> a;
		return a;
	}

	inputInt(fin, sum, 90, "error sum");
	inputInt(fin, close, 70, "error close");
	inputDouble(fin, lineWidth, 43, "error lineWidth");
	
	for (int i = 0; i < sum; i++)    //读入所有的节点
	{
		fin >> a;
		if (a == 42)
		{
			fin >> convexity;
			fin >> a;
		}
		if (a != 10)
		{
			cout<<"error x"<<endl;
			return -1;
		}
		fin >> now.x;
		inputDouble(fin, now.y, 20, "error y");
		if (i == 0)	start = now;
		if (fabs(convexity) < E)	addLine(last, now, name);
		else	addArc(last, now, convexity, name);
		last = now;
		convexity = 0;
	}
	fin >> a;
	if (a == 42)
	{
		fin >> convexity;
		fin >> a;
	}

	if (close == 1)
		if (fabs(convexity) < E)	addLine(now, start, name);
		else	addArc(now, start, convexity, name);
	return a;
}

void buildGraph(void)
{
	ofstream fout("alledges.txt");
	int i;
	fout << "Lines: " << lineCnt << endl;
	for (i = 1; i <= lineCnt; i++)
		fout << allLine[i].a.x << ' ' << allLine[i].a.y << ' ' << allLine[i].b.x << ' ' << allLine[i].b.y << endl;
	fout << "Arcs: " << arcCnt << endl;
	for (i = 1; i <= arcCnt; i++)
		fout << allArc[i].a.x << ' ' << allArc[i].a.y << ' ' << allArc[i].b.x << ' ' << allArc[i].b.y 
			<< ' ' << allArc[i].center.x << ' ' << allArc[i].center.y << ' ' << allArc[i].clockwise << endl;
}

void getEntities(ifstream& fin)    //对于定义连线的section，针对节点内容读信息
{
	int a;
	fin >> a;
	string st;
	while(!fin.eof())
	{
		inputedStr(fin, a, st, 0, "error entities");
		if (st == "LWPOLYLINE" || st == "LINE")
			a = getEntitiesLwPolyline(fin);
		else if (st=="ENDSEC")
			break;
		else
		{
			cout<<"error 0 entities"<<endl;
			return ;
		}
	}
	buildGraph();
}

void eraseSpace(string &s)
{
    int index = 0;
    if(!s.empty())
        while((index = s.find(' ', index)) != string::npos)
            s.erase(index, 1);
}

void getElse(ifstream& fin)	//不是ENTITIES的section忽略
{
	string a;
	int end = 0;
	fin >> a;
	while (!fin.eof())
	{
		fin >> a;
		eraseSpace(a);
		if (end)
			if (a == "ENDSEC")
				return;
			else 
				end = 0;
		if (a == "0")
			end = 1;
	}
}

void getSection(ifstream& fin)	//按照section，即按照不同的板块读文件
{
	int a;
	string st;
	fin >> a >> st;
	cout << a << ' ' << st << endl;
	if (a == 2)
		if (st=="ENTITIES")	getEntities(fin);
		else	getElse(fin);
	else	printf("error 2 section\n");
}


void readFile(char* filename)	//开始读文件
{
	ifstream fin(filename);
	while(!fin.eof())
	{
		string st;
		inputStr(fin, st, 0, "error SECTION");
		if (st == "SECTION")	getSection(fin);
		else if (st == "EOF")	break;
		else
		{
			printf("error 0 SECTION\n");
			break;
		}
	}
	fin.close();
}

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		cout << "Use: dxf2txt.exe filename savename" << endl;
		return 0;
	}
	readFile(argv[1]);
	//printFile(argv[2]);
	return 0;
}
