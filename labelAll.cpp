#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cmath>

using namespace std;

const int LINEMAX = 200000;
const int ARCMAX = 100000;
const int LABELINIT = 3344;
const double E = 0.1;
const double ZERO = 1e-10;
const double LINE_MERGE_ACCEPT = 30.0;
const double PI = 3.1415926535897932384626;
const int AVAILABLE = 1048576;
const double MAXN = 1e10;

const int DIR_X = 0;
const int DIR_Y = 1;
const int DIR_ELSE = -1;
const double PIPE_WIDTH = 100.0;
const double PIPE_WIDTH2 = 125.0;
const double PIPE_DIST = 70.0;

const string LINE = "LINE";
const string FLOW = "Flow";
const string ENTITY = "AcDbEntity";
const string ACDBLINE = "AcDbLine";
const string LAYER1 = "Control 1 - Push up";
const string LAYER2 = "Control 2 - Push down";

struct Point
{
	double x, y;
};

double posX[LINEMAX], posY[LINEMAX];
int xCnt, yCnt;

int pipeNums[LINEMAX], pipeCnt;

struct Line
{
	Point a, b;
	string name;
	int id;
	int dir;		//-1:else 0:x 1:y
	void swapAB(void)
	{
		Point p = a;
		a = b;
		b = p;
	}
}allLine[LINEMAX], lineX[LINEMAX], lineY[LINEMAX], lineExtraX[LINEMAX], lineExtraY[LINEMAX];
int lineCnt, lineXCnt, lineYCnt, lineExtCntX, lineExtCntY;

Line linePipeX[LINEMAX], linePipeY[LINEMAX], mergedPipeX[LINEMAX];
int linePipeXCnt, linePipeYCnt, mergedPipeXCnt;

struct Arc
{
	Point a, b, center;
	bool clockwise;
	string name;
	int id;
}allArc[ARCMAX];
int arcCnt;

int polyCnt, added, extraCnt, labelCnt;

string outputStr, bufferStr, extraStr, firstStr;

void clearBuffer(void)
{
	outputStr += bufferStr;
	bufferStr = "";
}

string to10(int now)
{
    string nowStr = "";
    int tmp;
    while(now)
    {
        tmp = now % 10;
		if (tmp < 10)
			nowStr = (char)('0' + tmp) + nowStr;
		else
			nowStr = (char)('A' + tmp - 10) + nowStr;
		now /= 10;
    }
    return nowStr;
}

string to16(int now)
{
    string nowStr = "";
    int tmp;
    while(now)
    {
        tmp = now % 16;
		if (tmp < 10)
			nowStr = (char)('0' + tmp) + nowStr;
		else
			nowStr = (char)('A' + tmp - 10) + nowStr;
		now /= 16;
    }
    return nowStr;
}

string getNum(void)
{
	string nowStr = to16(extraCnt + AVAILABLE);
	extraCnt ++;
	return nowStr;
}

string getLabel(void)
{
    labelCnt ++;
    string nowStr = "*D" + to10(labelCnt);
    return nowStr;
}

void labelPos(void)
{
	clearBuffer();
	ofstream fout("outputStr.txt");
	fout << outputStr << endl;
	firstStr = outputStr;
	outputStr = "";
	extraStr = "";
	added = 1;
}

void addLine(Point a, Point b, string name)   //添加边
{
	if (fabs(a.x - b.x) < E && fabs(a.y - b.y) < E)	return;
	allLine[++lineCnt].a = a;
	allLine[lineCnt].b = b;
	allLine[lineCnt].name = name;
	allLine[lineCnt].id = polyCnt;
	if (fabs(a.x - b.x) < E)	allLine[lineCnt].dir = DIR_X;
	else if (fabs(a.y - b.y) < E)	allLine[lineCnt].dir = DIR_Y;
	else
	{
		allLine[lineCnt].dir = DIR_ELSE;
		if (name != FLOW)	return;
		if (a.y > b.y)	{Point p = a; a = b; b = p;}
		Point p;
		p.x = a.x; p.y = b.y;
		//cout << "ADD ELSE LINE " << a.x << ' ' << a.y << ' ' << b.x << ' ' << b.y << ' ' << p.x << ' ' << p.y << endl;
		addLine(a, p, name);
		addLine(p, b, name);
	}
}

void addArc(Point a, Point b, double convexity, string name)	//添加圆弧
{
	//cout << "########################ADD_ARC#######################" << endl;
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
		addLine(a, b, name);
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
			double q = 2 * (k * (bk - a.y) - a.x);
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

	if (fabs(nowArc.a.x - nowArc.center.x) < E && fabs(nowArc.b.y - nowArc.center.y) < E)
	{
		Point p;
		p.x = nowArc.b.x, p.y = nowArc.a.y;
		addLine(p, nowArc.a, nowArc.name);
		addLine(p, nowArc.b, nowArc.name);
	}
	else if (fabs(nowArc.a.y - nowArc.center.y) < E && fabs(nowArc.b.x - nowArc.center.x) < E)
	{
		Point p;
		p.x = nowArc.a.x, p.y = nowArc.b.y;
		addLine(p, nowArc.a, nowArc.name);
		addLine(p, nowArc.b, nowArc.name);
	}
	else
	{
		addLine(nowArc.a, nowArc.b, nowArc.name);
	}
}

void inputInt(ifstream& fin, int &x, int key, string errStr, int keep = 1)
{
	int a;
	fin >> a;
	if (keep)	bufferStr += to_string(a) + '\n';
	if (a == key)
	{
		fin >> x;
		if (keep)	bufferStr += to_string(x) + '\n';
		return;
	}
	cout << errStr << endl;
	exit(0);
}

void inputDouble(ifstream& fin, double &x, int key, string errStr, int keep = 1)
{
	int a;
	fin >> a;
	if (keep)	bufferStr += to_string(a) + '\n';
	if (a == key)
	{
		fin >> x;
		if (keep)	bufferStr += to_string(x) + '\n';
		return;
	}
	cout << errStr << endl;
	exit(0);
}

void inputStr(ifstream& fin, string &x, int key, string errStr, int keep = 1)
{
	int a;
	fin >> a;
	if (keep)	bufferStr += to_string(a) + '\n';
	//cout << a << ' ';
	if (a == key)
	{
		getline (fin, x);
		getline (fin, x);
		if (keep)	bufferStr += x + '\n';
		//cout << x << endl;
		return;
	}
	cout << errStr << ' ' << a << endl;
	exit(0);
}

void inputedStr(ifstream& fin, int a, string &x, int key, string errStr, int keep = 1)
{
	if (a == key)
	{
		getline (fin, x);
		getline (fin, x);
		if (keep == 1)	bufferStr += x + '\n';
		else if (keep == -1 && x != "DIMENSION")	bufferStr += x + '\n';
		//cout << x << endl;
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
		
		if (name == "Flow")
			clearBuffer();
		else
			//clearBuffer();
			bufferStr = "";

		fin >> a;
		bufferStr += to_string(a) + '\n';
		return a;
	}

	inputInt(fin, sum, 90, "error sum");
	inputInt(fin, close, 70, "error close");
	inputDouble(fin, lineWidth, 43, "error lineWidth");
	
	for (int i = 0; i < sum; i++)    //读入所有的节点
	{
		fin >> a;
		bufferStr += to_string(a) + '\n';
		//cout << a << ' ';
		if (a == 42)
		{
			fin >> convexity;
			bufferStr += to_string(convexity) + '\n';
			fin >> a;
			bufferStr += to_string(a) + '\n';
		}
		if (a != 10)
		{
			cout<<"error x"<<endl;
			return -1;
		}
		//cout << convexity << endl;
		fin >> now.x;
		bufferStr += to_string(now.x) + '\n';
		inputDouble(fin, now.y, 20, "error y");
		if (i == 0)	start = now;
		else if (fabs(convexity) < ZERO)	addLine(last, now, name);
		else	addArc(last, now, convexity, name);
		last = now;
		convexity = 0;
	}
	fin >> a;
	if (a == 42)
	{
		bufferStr += to_string(a) + '\n';
		fin >> convexity;
		bufferStr += to_string(convexity) + '\n';
		fin >> a;
	}

	clearBuffer();
	
	if (!added)
		labelPos();

	bufferStr += to_string(a) + '\n';

	if (close == 1)
		if (fabs(convexity) < ZERO)	addLine(now, start, name);
		else	addArc(now, start, convexity, name);
	return a;
}
 

void printExtraLabel(Point a, Point b, Point c, Point d, string info)
{
	clearBuffer();

	bufferStr += "0\nDIMENSION\n";
	bufferStr += "5\n" + getNum() + "\n";
	bufferStr += "330\n2\n";
	bufferStr += "100\n" + ENTITY + "\n";
	bufferStr += "8\n" + FLOW + "\n";
	bufferStr += "100\nAcDbDimension\n";
	bufferStr += "280\n0\n";
	bufferStr += "2\n" + getLabel() + "\n";

	bufferStr += "10\n" + to_string(a.x) + "\n";
	bufferStr += "20\n" + to_string(a.y) + "\n";
	bufferStr += "30\n0.0\n";
	bufferStr += "11\n" + to_string(b.x) + "\n";
	bufferStr += "21\n" + to_string(b.y) + "\n";
	bufferStr += "31\n0.0\n";

	bufferStr += "70\n32\n";
	bufferStr += "1\n" + info + "\n";
	bufferStr += "71\n5\n";
	bufferStr += "42\n" + to_string(fabs(c.x - d.x) + fabs(c.y - d.y)) + "\n";
	bufferStr += "73\n0\n";
	bufferStr += "74\n0\n";
	bufferStr += "75\n0\n";
	bufferStr += "3\nTrap\n";
	bufferStr += "100\nAcDbAlignedDimension\n";
	bufferStr += "13\n" + to_string(c.x) + "\n";
	bufferStr += "23\n" + to_string(c.y) + "\n";
	bufferStr += "33\n0.0\n";
	bufferStr += "14\n" + to_string(d.x) + "\n";
	bufferStr += "24\n" + to_string(d.y) + "\n";
	bufferStr += "34\n0.0\n";
    bufferStr += "50\n";
    if (fabs(c.x - d.x) < E)    bufferStr += "90.0\n";
    else    bufferStr += "0.0\n";
    bufferStr += "100\nAcDbRotatedDimension\n";

	extraStr += bufferStr;
	bufferStr = "";
}

void addm_n(int m, int n, Point init, string info, int dir, double width)
{
    //dir == 0: right 1:upper 2:left 3:down
    Point left, now;
    Point a, b, c, d;
    int i, j;

    left = init;
    for (i = 1; i <= n; i++)
    {
        now = left;
        for (j = 1; j <= m; j++)
        {
            if (dir == 0)
            {
                a = now; a.x += 10.0;
                b = now; b.y += width / 2.0; b.x += 10.0;
                d = now; d.y += width;
                c = now;
            }
            else if (dir == 1)
            {
                a = now; a.x += width; a.y += 15.0;
                b = now; b.x += width / 2.0; b.y += 15.0;
                d = now; 
                c = now; d.x += width;
            }
            else if (dir == 2)
            {
                a = now; a.x -= 10.0;
                b = now; b.y += width / 2.0; b.x -= 10.0;
                d = now; d.y += width;
                c = now;
            }
            else
            {
                a = now; a.x += width; a.y -= 15.0;
                b = now; b.x += width / 2.0; b.y -= 15.0;
                d = now; 
                c = now; d.x += width;
            }
            printExtraLabel(a, b, c, d, info);
            now.x += 1400;
        }
        left.y += 1700;
    }
}

void addAllLabels(void)
{
    Point p;
    p.x = 18912.4989; p.y = 10723.1072; addm_n(16, 8, p, "v0l1p1", 1, 100.0);
    p.x = 19812.4989; p.y = 10723.1072; addm_n(16, 8, p, "v0l1p1", 1, 100.0);
    p.x = 19362.4989; p.y = 11073.1072; addm_n(16, 7, p, "v0l1p1", 2, 100.0);

    p.x = 19062.4989; p.y = 10423.1072; addm_n(16, 8, p, "v0l1p2", 2, 100.0);
    p.x = 19662.4989; p.y = 10423.1072; addm_n(16, 8, p, "v0l1p2", 2, 100.0);
    p.x = 19362.4989; p.y = 11223.1072; addm_n(16, 7, p, "v0l1p2", 2, 100.0);
    
    p.x = 18912.4989; p.y = 10223.1072; addm_n(16, 8, p, "v0l1p3", 1, 100.0);
    p.x = 19812.4989; p.y = 10223.1072; addm_n(16, 8, p, "v0l1p3", 1, 100.0);
    p.x = 19362.4989; p.y = 11373.1072; addm_n(16, 7, p, "v0l1p3", 2, 100.0);

}

void getEntities(ifstream& fin)    //对于定义连线的section，针对节点内容读信息
{
	int a;
	clearBuffer();
	fin >> a;
	bufferStr += to_string(a) + '\n';
	string st;
	while(!fin.eof())
	{
		inputedStr(fin, a, st, 0, "error entities", -1);
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
	getline (fin, a);
	getline (fin, a);
	bufferStr += a + '\n';
	while (!fin.eof())
	{
		getline (fin, a);
		bufferStr += a + '\n';
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
	bufferStr += to_string(a) + '\n' + st + '\n';
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
	clearBuffer();
	fin.close();
}

void printFile(char* filename)
{
	ofstream fout(filename);
	//cout << firstStr << endl;
	outputStr = firstStr + extraStr + outputStr;
	fout << outputStr;
	fout.close();
}

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		cout << "Use: labelAll.exe filename savename" << endl;
		return 0;
	}
	readFile(argv[1]);
    addAllLabels();
	printFile(argv[2]);
	return 0;
}
