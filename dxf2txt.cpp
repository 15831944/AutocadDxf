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

struct Valve
{
	Point a, b, c, center;
	string info, name;
	int pipeNum, type, layer;
	int order, setId;
	double angle;
}allValve[LINEMAX];
int valveCnt, orderCnt;

int polyCnt, added, extraCnt;

string outputStr, bufferStr, extraStr, firstStr;

int cmpInt(const void *a, const void *b)
{
	return (*(int *)a) > (*(int *)b) ? 1 : -1;
}
int cmpLf(const void *a, const void *b)
{
	return (*(double *)a) > (*(double *)b) ? 1 : -1;
}
int cmpLineX(const void *a, const void *b)
{
	Line p = *(struct Line *)a, q = *(struct Line *)b;
	if (fabs(p.a.x - q.a.x) < E)
		return p.a.y > q.a.y ? 1 : -1;
	return p.a.x > q.a.x ? 1 : -1;
}
int cmpLineY(const void *a, const void *b)
{
	Line p = *(struct Line *)a, q = *(struct Line *)b;
	if (fabs(p.a.y - q.a.y) < E)
		return p.a.x > q.a.x ? 1 : -1;
	return p.a.y > q.a.y ? 1 : -1;
}

bool cmpLineXW(Line p, Line q)
{
	if (fabs(p.a.x - q.a.x) < LINE_MERGE_ACCEPT)
		return p.a.y > q.a.y ? 0 : 1;
	return p.a.x > q.a.x ? 0 : 1;
}
bool cmpLineYW(Line p, Line q)
{
	if (fabs(p.a.y - q.a.y) < LINE_MERGE_ACCEPT)
		return p.a.x > q.a.x ? 0 : 1;
	return p.a.y > q.a.y ? 0 : 1;
}
bool cmpValveXW(Valve p, Valve q)
{
	if (fabs(p.center.x - q.center.x) < PIPE_WIDTH + E)
		return p.center.y > q.center.y ? 0 : 1;
	return p.center.x > q.center.x ? 0 : 1;
}
bool cmpValveYW(Valve p, Valve q)
{
	if (fabs(p.center.y - q.center.y) < PIPE_WIDTH + E)
		return p.center.x > q.center.x ? 0 : 1;
	return p.center.y > q.center.y ? 0 : 1;
}

void clearBuffer(void)
{
	outputStr += bufferStr;
	bufferStr = "";
}

string getNum(void)
{
	string nowStr = "";
	int now = extraCnt + AVAILABLE, tmp;
	while(now)
	{
		tmp = now % 16;
		if (tmp < 10)
			nowStr = (char)('0' + tmp) + nowStr;
		else
			nowStr = (char)('A' + tmp - 10) + nowStr;
		now /= 16;
	}
	extraCnt ++;
	return nowStr;
}

void printExtraLine(Point a, Point b, string layer)
{
	clearBuffer();
	bufferStr += "0\n" + LINE + "\n";
	bufferStr += "5\n" + getNum() + "\n";
	bufferStr += "330\n2\n";
	bufferStr += "100\n" + ENTITY + "\n";
	bufferStr += "8\n" + layer + "\n";
	bufferStr += "100\n" + ACDBLINE + "\n";

	bufferStr += "10\n" + to_string(a.x) + "\n";
	bufferStr += "20\n" + to_string(a.y) + "\n";
	bufferStr += "30\n0.0\n";
	bufferStr += "11\n" + to_string(b.x) + "\n";
	bufferStr += "21\n" + to_string(b.y) + "\n";
	bufferStr += "31\n0.0\n";

	extraStr += bufferStr;
	bufferStr = "";
}

double getDist(Line a, Line b)
{
	if (a.dir != b.dir)	return MAXN;
	if (a.dir == DIR_X)
	{
		if (max(a.a.y, a.b.y) < min(b.a.y, b.b.y) || min(a.a.y, a.b.y) > max(b.a.y, b.b.y))	return MAXN;
		if (fabs(max(a.a.y, a.b.y) - min(b.a.y, b.b.y)) < E || fabs(min(a.a.y, a.b.y) - max(b.a.y, b.b.y)) < E)	return MAXN; 
		return fabs(a.a.x-b.a.x);
	}
	else if (a.dir == DIR_Y)
	{
		if (max(a.a.x, a.b.x) < min(b.a.x, b.b.x) || min(a.a.x, a.b.x) > max(b.a.x, b.b.x))	return MAXN;
		if (fabs(max(a.a.x, a.b.x) - min(b.a.x, b.b.x)) < E || fabs(min(a.a.x, a.b.x) - max(b.a.x, b.b.x)) < E)	return MAXN; 
		return fabs(a.a.y-b.a.y);
	}
	else return MAXN;
}

double getDist(Point a, Point b)
{
	return sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}

void addExtraLine(Point a, Point b, string layer)
{
	Line now;
	now.a = a;
	now.b = b;
	now.name = layer;
	if (fabs(a.x - b.x) < E)	now.dir = DIR_X, lineExtraX[++lineExtCntX] = now;
	else if (fabs(a.y - b.y) < E)	now.dir = DIR_Y, lineExtraY[++lineExtCntY] = now;
	else	now.dir = DIR_ELSE;
}

void addMidLine(Line a, Line b, double dis, string layer)
{
	Point st, fi;
	if (a.dir == DIR_X)
	{
		st.x = fi.x = (a.a.x + b.a.x) / 2.0;
		st.y = max(min(a.a.y, a.b.y), min(b.a.y, b.b.y)) - dis / 2.0;
		fi.y = min(max(a.a.y, a.b.y), max(b.a.y, b.b.y)) + dis / 2.0;
		//cout << "ADD " << dis << ' ' << st.x << ' ' << st.y << ' ' << fi.x << ' ' << fi.y << endl;
		addExtraLine(st, fi, layer);
	}
	else
	{
		st.y = fi.y = (a.a.y + b.a.y) / 2.0;
		st.x = max(min(a.a.x, a.b.x), min(b.a.x, b.b.x)) - dis / 2.0;
		fi.x = min(max(a.a.x, a.b.x), max(b.a.x, b.b.x)) + dis / 2.0;
		//cout << "ADD " << dis << ' ' << st.x << ' ' << st.y << ' ' << fi.x << ' ' << fi.y << endl;
		addExtraLine(st, fi, layer);
	}
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

	if (name == "Flow")
		clearBuffer();
	else
		//clearBuffer();
		bufferStr = "";
	
	if (!added)
		labelPos();

	bufferStr += to_string(a) + '\n';

	if (close == 1)
		if (fabs(convexity) < ZERO)	addLine(now, start, name);
		else	addArc(now, start, convexity, name);
	return a;
}

void outputLine(int i, double dis)
{
	cout << "Line :" << i << ' ' << allLine[i].dir << ' ' << allLine[i].name << ' ' << allLine[i].a.x << ' ' << allLine[i].a.y << ' ' << allLine[i].b.x << ' ' << allLine[i].b.y << ' ' << dis << endl;
}

void scanX(void)
{
	int i, j, cnt;
	double stY, fiY, lastX;
	Line last, now;
	for (i = 1; i < yCnt; i++)
	{
		stY = posY[i], fiY = posY[i+1], cnt = 0;
		for (j = 1; j <= lineXCnt; j++)
		{
			now = lineX[j];
			if (now.a.y < stY + E && now.b.y > fiY - E)
			{
				//if (fabs(now.a.x - lastX) < E)	continue;
				cnt ++;
				now.a.y = stY, now.b.y = fiY;
				if (cnt % 2 == 0 && (fabs(now.a.x - last.a.x - PIPE_WIDTH < E) || fabs(now.a.x - last.a.x - PIPE_WIDTH2 < E)) )
					addMidLine(last, now, now.a.x - last.a.x, LAYER1);
				last = now, lastX = now.a.x;
			}
		}
	}
}

void scanY(void)
{
	ofstream fout ("tmp.txt");
	int i, j, cnt;
	double stX, fiX, lastY;
	Line last, now;
	int tmp, tLast;
	for (i = 1; i < xCnt; i++)
	{
		stX = posX[i], fiX = posX[i+1], cnt = 0;
		for (j = 1; j <= lineYCnt; j++)
		{
			now = lineY[j];
			if (now.a.x < stX + E && now.b.x > fiX - E)
			{
				//if (fabs(now.a.y - lastY) < E)	continue;
				cnt ++;
				tLast = tmp; tmp = j;
				now.a.x = stX, now.b.x = fiX;

				if (cnt % 2 == 0 && (fabs(now.a.y - last.a.y - PIPE_WIDTH < E) || fabs(now.a.y - last.a.y - PIPE_WIDTH2 < E)))
				{
					addMidLine(last, now, now.a.y - last.a.y, LAYER1);
				}
				last = now, lastY = now.a.y;
			}
		}
	}
}

void outputLines(ofstream &fout, int cnt, Line allLine[], string title)
{
	fout << title << ' ' << cnt << endl;
	for (int i = 1; i <= cnt; i++)
		if (allLine[i].id != -1)
			fout << i << ' ' << allLine[i].name << ' ' << allLine[i].a.x << ' ' << allLine[i].a.y << ' ' << allLine[i].b.x << ' ' << allLine[i].b.y << " id:" << allLine[i].id << endl;
	fout << endl << endl;
}

void outputArray(ofstream &fout, int cnt, double num[], string title)
{
	fout << title << ' ' << cnt << endl;
	for (int i = 1; i <= cnt; i++)
		fout << num[i] << ' ';
	fout << endl << endl;
}

void sortIntArray(int &cnt, int num[])
{
	qsort(num + 1, cnt, sizeof(num[0]), cmpInt);
	num[0] = -1;
	int i, j;
	for (i = 1, j = 0; i <= cnt; i++)	if (num[i] != num[j])	num[++j] = num[i];
	cnt = j;
}

void sortArray(ofstream &fout, int &cnt, double num[], string title)
{
	qsort(num + 1, cnt, sizeof(num[0]), cmpLf);
	num[0] = MAXN;
	int i, j;
	
	for (i = 1, j = 0; i <= cnt; i++)
		if (fabs(num[i] - num[j]) > E)
			num[++j] = num[i];
	cnt = j;
	
	outputArray(fout, cnt, num, title);
}

void sortLine(ofstream &fout, int &cnt, struct Line lines[], string title, int dir)
{
	int i, j;
	double nowMax;

	if (dir == DIR_X)	sort(lines + 1, lines + 1 + cnt, cmpLineXW);
	else	sort(lines + 1, lines + 1 + cnt, cmpLineYW);

	outputLines(fout, cnt, lines, "before " + title);
	for (i = 1, nowMax = -MAXN, j = 0; i <= cnt; i++)
		if (dir == DIR_X)
		{
			if (lines[i].id == -1)	continue;
			if (fabs(lines[i].a.x - lines[i-1].a.x) > LINE_MERGE_ACCEPT)
			{
				nowMax = lines[i].b.y, j = i;
				continue;
			}
			if (lines[i].b.y + E < nowMax)	lines[i].id = -1;
			else if (lines[i].a.y - LINE_MERGE_ACCEPT > nowMax)	j = i, nowMax = lines[i].b.y;
			else
			{
				nowMax = lines[j].b.y = lines[i].b.y;
				lines[i].id = -1;
			}
			//fout << "IN_PROGRESS" << i << ' ' << lines[i].a.x << ' ' << lines[i].a.y << ' ' << lines[i].b.x << ' ' << lines[i].b.y << " id:" << lines[i].id << ' ' << nowMax << endl;
		}
		else
		{
			if (lines[i].id == -1)	continue;
			if (fabs(lines[i].a.y - lines[i-1].a.y) > LINE_MERGE_ACCEPT)
			{
				nowMax = lines[i].b.x, j = i;
				continue;
			}
			if (lines[i].b.x + E < nowMax)	lines[i].id = -1;
			else if (lines[i].a.x - LINE_MERGE_ACCEPT > nowMax)	j = i, nowMax = lines[i].b.x;
			else
			{
				nowMax = lines[j].b.x = lines[i].b.x;
				lines[i].id = -1;
			}
		}
	for (i = 1, nowMax = -MAXN, j = 0; i <= cnt; i++)
		if (lines[i].id != -1)
			lines[++j] = lines[i];
	cnt = j;

	outputLines(fout, cnt, lines, "after " + title);
}

double getDistLineX(Point p, Line l)
{
	if (l.a.x > l.b.x)	l.swapAB();
	if (p.x + E > l.a.x && p.x - E < l.b.x)	return fabs(p.y - l.a.y);
	return min(getDist(p, l.a), getDist(p, l.b));
}

double getDistLineY(Point p, Line l)
{
	if (l.a.y > l.b.y)	l.swapAB();
	if (p.y + E > l.a.y && p.y - E < l.b.y)	return fabs(p.x - l.a.x);
	return min(getDist(p, l.a), getDist(p, l.b));
}

int mergePoint(Point &p, int &cnt, Line lines[])
{
	int i;
	for (i = 1; i <= cnt; i++)	if (getDistLineY(p, lines[i]) < E)	return 0;
	for (i = 1; i <= cnt; i++)
		if (getDistLineY(p, lines[i]) < PIPE_WIDTH)
		{
			p.x = lines[i].a.x;
			if (getDist(p, lines[i].a) + E < PIPE_WIDTH)	lines[i].a = p;
			if (getDist(p, lines[i].b) + E < PIPE_WIDTH)	lines[i].b = p;
			return 1;
		}
	return -1;
}

int lineMerged(Point &p, int &cnt, Line lines[])
{
	int i;
	for (i = 1; i <= cnt; i++)	if (getDistLineX(p, lines[i]) < E)	return 0;
	for (i = 1; i <= cnt; i++)
		if (getDistLineX(p, lines[i]) < PIPE_WIDTH)
		{
			p.y = lines[i].a.y;
			if (getDist(p, lines[i].a) + E < PIPE_WIDTH)	lines[i].a = p;
			if (getDist(p, lines[i].b) + E < PIPE_WIDTH)	lines[i].b = p;
			return 1;
		}
	return -1;
}

void mergePrint(int &cntX, Line lineX[], int &cntY, Line lineY[])
{
	int i, j, flag;
	for (i = 1; i <= cntY; i++)
	{
		flag = mergePoint(lineY[i].a, cntX, lineX);
		flag = max(flag, mergePoint(lineY[i].b, cntX, lineX));
		if (flag == -1 || getDist(lineY[i].a, lineY[i].b) + E < PIPE_WIDTH)
			lineY[i].id = -1;
	}
	for (i = 1; i <= cntX; i++)
	{
		flag = lineMerged(lineX[i].a, cntY, lineY);
		flag = max(flag, lineMerged(lineX[i].b, cntY, lineY));
		if (flag == -1 || getDist(lineX[i].a, lineX[i].b) + E < PIPE_WIDTH)
			lineX[i].id = -1;
	}

	for (i = 1; i <= cntX; i++)
		if (lineX[i].id != -1)
			printExtraLine(lineX[i].a, lineX[i].b, lineX[i].name);
	for (i = 1; i <= cntY; i++)
		if (lineY[i].id != -1)
			printExtraLine(lineY[i].a, lineY[i].b, lineY[i].name);
}

void fixScan(void)
{
	int i;
	for (i = 1; i <= lineExtCntX; i++)
		if (fabs(lineExtraX[i].a.x - 35431.2) < E && fabs(lineExtraX[i].a.y - 29375.2) < E)
		{
			cout << "fix " << lineExtraX[i].a.x << ' ' << lineExtraX[i].a.y << ' ' << lineExtraX[i].b.x << ' ' << lineExtraX[i].b.y << endl;
			lineExtraX[i].a.y = 29113.1;
		}
		else if (fabs(lineExtraX[i].a.x - 35377.3) < E)
			lineExtraX[i].id = -1;
}

void buildGraph(void)
{
	ofstream fout("alledges.txt");
	int i, j, cnt = 0;

	outputLines(fout, lineCnt, allLine, "Lines: ");
	fout << "Arcs: " << arcCnt << endl;
	for (i = 1; i <= arcCnt; i++)
		fout << i << ' ' << allArc[i].name << ' ' << allArc[i].a.x << ' ' << allArc[i].a.y << ' ' << allArc[i].b.x << ' ' << allArc[i].b.y 
			<< ' ' << allArc[i].center.x << ' ' << allArc[i].center.y << ' ' << allArc[i].clockwise << endl;
	
	for (i = 1; i <= lineCnt; i++)
	{
		if (allLine[i].name != FLOW)
			continue;
		posX[++xCnt] = allLine[i].a.x;
		posY[++yCnt] = allLine[i].a.y;
		posX[++xCnt] = allLine[i].b.x;
		posY[++yCnt] = allLine[i].b.y;
		if (allLine[i].dir == DIR_X)
		{
			lineX[++lineXCnt] = allLine[i];
			if (lineX[lineXCnt].a.y > lineX[lineXCnt].b.y)	lineX[lineXCnt].swapAB();
		}
		else if (allLine[i].dir == DIR_Y)
		{
			lineY[++lineYCnt] = allLine[i];
			if (lineY[lineYCnt].a.x > lineY[lineYCnt].b.x)	lineY[lineYCnt].swapAB();
		}
	}	

	sortArray(fout, xCnt, posX, "PosX: ");
	sortArray(fout, yCnt, posY, "PosY: ");

	qsort(lineX + 1, lineXCnt, sizeof(lineX[0]), cmpLineX);
	qsort(lineY + 1, lineYCnt, sizeof(lineY[0]), cmpLineY);

	outputLines(fout, lineXCnt, lineX, "LineX: ");
	outputLines(fout, lineYCnt, lineY, "LineY: ");

	fout.close();
	ofstream fo("allExtraLines.txt");
	scanX();
	scanY();
	fixScan();

	sortLine(fo, lineExtCntX, lineExtraX, "LineExtraX: ", DIR_X);
	sortLine(fo, lineExtCntY, lineExtraY, "LineExtraY: ", DIR_Y);

	mergePrint(lineExtCntX, lineExtraX, lineExtCntY, lineExtraY);
}

int getEntitiesDimension(ifstream& fin)   //读标记信息
{
	int i, a = 0, polyType, num280, signalType, unknown;      //a为里面的预设参数
	string polyNum, subClass, name, subClass2, name2, signalType2;    //都是dxf文件的定义参数
	double tmp, width;
	Point start, end;   //开始节点的x,y;结束节点的x.y;
	Valve now;

	polyCnt++;
	inputStr(fin, polyNum, 5, "error polyNum", 0);
	inputInt(fin, polyType, 330, "error polyType", 0);
	inputStr(fin, subClass, 100, "error subClass", 0);
	inputStr(fin, name, 8, "error name", 0);
	inputStr(fin, subClass2, 100, "error subClass2", 0);
	inputInt(fin, num280, 280, "error Dimension 280", 0);
	inputStr(fin, name2, 2, "error name2", 0);
	inputDouble(fin, tmp, 10, "error x0", 0);
	inputDouble(fin, tmp, 20, "error y0", 0);
	inputDouble(fin, tmp, 30, "error z0", 0);
	inputDouble(fin, tmp, 11, "error x1", 0);
	inputDouble(fin, tmp, 21, "error y1", 0);
	inputDouble(fin, tmp, 31, "error z1", 0);
	inputInt(fin, signalType, 70, "error signalType", 0);
	inputStr(fin, now.info, 1, "error info", 0);
	inputInt(fin, unknown, 71, "error unknown 71", 0);
	inputDouble(fin, width, 42, "error width", 0);
	inputInt(fin, unknown, 73, "error unknown 73", 0);
	inputInt(fin, unknown, 74, "error unknown 74", 0);
	inputInt(fin, unknown, 75, "error unknown 75", 0);
	inputStr(fin, signalType2, 3, "error signalType2", 0);
	inputStr(fin, subClass2, 100, "error subClass2", 0);

	inputDouble(fin, now.a.x, 13, "error x3", 0);
	inputDouble(fin, now.a.y, 23, "error y3", 0);
	inputDouble(fin, tmp, 33, "error z3", 0);
	inputDouble(fin, now.b.x, 14, "error x4", 0);
	inputDouble(fin, now.b.y, 24, "error y4", 0);
	inputDouble(fin, tmp, 34, "error z4", 0);

	fin >> a;
	if (a == 50)
	{
		fin >> now.angle;
		fin >> a;
	}
	else	now.angle = 0;
	
	if (a != 100)
	{
		cout << "error angle";
		return -1;
	}
	fin >> subClass2;

	now.name = name;
	now.c.x = (now.a.x + now.b.x) / 2.0;
	now.c.y = (now.a.y + now.b.y) / 2.0;

	for (i = 0, a = 0; i < now.info.length(); i++)
		if (now.info[i] == 'v')	continue;
		else if (now.info[i] == 'l')
		{
			now.type = a;
			a = 0;
		}
		else if (now.info[i] == 'p')
		{
			now.layer = a;
			a = 0;
		}
		else if (now.info[i] >= '0' && now.info[i] <= '9')	a = a * 10 + now.info[i] - '0';
		else
		{
			cout << "NAME ERROR: vXXlYYpZZ" << endl;
			return -1;
		}
	now.pipeNum = a;
	pipeNums[++pipeCnt] = now.pipeNum;

	allValve[++valveCnt] = now;

	fin >> a;
	return a;
}

void buildLabels(void)
{
	ofstream fout ("allValve.txt");
	int i, j;
	Valve now;
	cout << "Build labels" << endl;
	for (j = 1; j <= valveCnt; j++)
	{
		now = allValve[j];
		if (fabs(now.angle - 0.0) < E)		
		{
			for (i = 1; i <= lineExtCntY; i++)
				if (getDistLineX(now.c, lineExtraY[i]) < PIPE_WIDTH / 2.0 + E)
				{
					now.center.x = now.c.x, now.center.y = lineExtraY[i].a.y;
					break;
				}
			if (now.center.y > now.c.y)	now.angle = 270.0;	//angle: 0=East 90=North 180=West 270=South
			else	now.angle = 90.0;
		}
		else if (fabs(now.angle - 90.0) < E)
		{
			for (i = 1; i <= lineExtCntX; i++)
				if (getDistLineY(now.c, lineExtraX[i]) < PIPE_WIDTH / 2.0 + E)
				{
					now.center.x = lineExtraX[i].a.x, now.center.y = now.c.y;
					break;
				}
			if (now.center.x > now.c.x)	now.angle = 180.0;
			else	now.angle = 0.0;
		}
		else
		{
			cout << "SIGNAL ANGLE WRONG: " << now.angle << endl;
			return;
		}
		allValve[j] = now;
	}

	for (i = 1; i <= valveCnt; i++)
		if (allValve[i].layer == 1)
			printExtraLine(allValve[i].c, allValve[i].center, LAYER1);
		else if (allValve[i].layer == 2)
			printExtraLine(allValve[i].c, allValve[i].center, LAYER2);

	sortIntArray(pipeCnt, pipeNums);
	
	sort(allValve + 1, allValve + 1 + valveCnt, cmpValveYW);
	for (i = 1; i <= valveCnt; i++)
		if (fabs(allValve[i].center.y - allValve[i-1].center.y) > PIPE_WIDTH + E)	allValve[i].order = ++orderCnt;
		else	allValve[i].order = orderCnt;

	fout << "Valves: " << valveCnt << endl;
	for (i = 1; i <= valveCnt; i++)
		fout << i << ' ' << allValve[i].info << ' ' << allValve[i].center.x << ' ' << allValve[i].center.y << ' ' << allValve[i].c.x << ' ' << allValve[i].c.y << ' ' << allValve[i].type << ' ' << allValve[i].layer << ' ' << allValve[i].pipeNum << ' ' << allValve[i].angle << ' ' << allValve[i].order << ' ' << allValve[i].name<< endl;
	cout << "Label build over" << endl;
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
		else if (st == "DIMENSION")
			a = getEntitiesDimension(fin);
		else if (st=="ENDSEC")
			break;
		else
		{
			cout<<"error 0 entities"<<endl;
			return ;
		}
	}
	buildGraph();
	buildLabels();
	//buildPipes();
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
		cout << "Use: dxf2txt.exe filename savename" << endl;
		return 0;
	}
	readFile(argv[1]);
	printFile(argv[2]);
	return 0;
}
