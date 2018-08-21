#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <time.h>  
#include <math.h>
using namespace std;
#define NAX 0.000000001
#define EDGESUM 118
#define NODESUM 67
#define SPACE 8
#define PLACE 8
#define VD 20
#define SPEED 10
#define ALLLENG 1600
#define HALFLENG 800

int length1,length2,length3;
struct node
{          
	vector<int> elist;  //点所连接的边，按照顺时针的顺序存储下来

};
struct edge
{
	bool exist; //表示这条边是否存在
	bool fc;  //表示该边的浓度时是否已经确定
	bool ft;  // 记录该边是否已经被遍历过
	bool fl;  //记录该边是否在遍历的队列中
	double v; //流速
	double leng; //管道长度
	double con; //管道液体浓度
	int n1,n2; //连接的节点
};
edge edges[EDGESUM]; //存储所有的边
node nodes[NODESUM]; //存储所有的节点
vector< vector<double> >rect(200);//存储行列式
bool *fl; //记录对应行的行列式是否存在
bool *fr; //记录对应节点是否在队列中
int *q; //节点存储队列
int tail = 0; //存储队列的尾
int head = 0; //存储队列的首
void insert(int x)   //插入队列
{
	if (x==65)
		return;
	if (fr[x])
		return ;
	fr[x] = true;
	q[tail] = x;
	tail++;
	if (tail == NODESUM)
		tail = 0;
}
 
int pop()      //弹出队列
{
	int ans = q[head];
	head++;
	fr[ans] = false;
	if (head == NODESUM)
		head = 0;
	return ans;
}

bool check(int x)   //判断一个节点的所有流入节点是否全部求出
{
	for (int i=0; i<nodes[x].elist.size(); i++)
	{
		edge tmp = edges[nodes[x].elist[i]];
		if (tmp.n2 == x && !tmp.fc){
			return false;
		}
	}
	return true;
}

void getconcentration()   //计算浓度
{
	time_t   first,   second;   
	first = time(   NULL   );
	fr = new bool[EDGESUM];
	for (int i=0; i<EDGESUM; i++)
		fr[i] = false;
	q = new int[NODESUM];
	insert(59);				//两个入口节点
	insert(62);
	while (tail!=head)
	{
		second = time(NULL);
		if (difftime(second,first)>1){
			cout<<"time error"<<endl;
			return;
		}
		int x = pop();
		//cout<<"head "<<head<<" tail "<<tail<<endl;
		//cout<<"pop "<<x<<endl;
		if (check(x))
		{
			node temp = nodes[x];
			int esum = temp.elist.size();
			//cout<<x<<" "<<esum<<endl;
			if (esum == 2)
			{
				if (edges[temp.elist[0]].n2==x)
				{
					edges[temp.elist[1]].con = edges[temp.elist[0]].con;
					edges[temp.elist[1]].fc=true;
					insert(edges[temp.elist[1]].n2);
				}else{
					edges[temp.elist[0]].con = edges[temp.elist[1]].con;
					edges[temp.elist[0]].fc=true;
					insert(edges[temp.elist[0]].n2);
				}
			}else if (esum == 3)
			{
				int sum = 0;
				int in,out;
				for (int i=0; i<3; i++)
				{
					edge tmp = edges[temp.elist[i]];
					if (tmp.n2 == x)
					{
						in = i;
						sum++;
					}else
						out = i;
				}
				if (sum == 1)
				{
					for (int i=0; i<3; i++)
					if (i != in)
					{
						edges[temp.elist[i]].con = edges[temp.elist[in]].con;
						edges[temp.elist[i]].fc=true;
						insert(edges[temp.elist[i]].n2);
					}
				}else{
					double c = 0;
					for (int i=0; i<3; i++)
					if (i != out)
					{
						c = c+edges[temp.elist[i]].con*edges[temp.elist[i]].v;
					}
					edges[temp.elist[out]].con = c/edges[temp.elist[out]].v;
					edges[temp.elist[out]].fc=true;
					insert(edges[temp.elist[out]].n2);
				}
			} else if (esum == 4)
			{
				int sum = 0;
				int in,out;
				for (int i=0; i<4; i++)
				{
					//cout<<x<<" "<<temp.elist[i]<<" "<<edges[temp.elist[i]].n2<<endl;
					edge tmp = edges[temp.elist[i]];
					if (tmp.n2 == x)
					{
						in = i;
						sum++;
					}else
						out = i;
				}
				if (sum == 1)
				{
					for (int i=0; i<4; i++)
					if (i != in)
					{
						edges[temp.elist[i]].con = edges[temp.elist[in]].con;
						edges[temp.elist[i]].fc=true;
						insert(edges[temp.elist[i]].n2);
					}
				}else if (sum==3)
				{
					double c = 0;
					for (int i=0; i<4; i++)
					if (i != out)
					{
						c = c+edges[temp.elist[i]].con*edges[temp.elist[i]].v;
					}
					edges[temp.elist[out]].con = c/edges[temp.elist[out]].v;
					edges[temp.elist[out]].fc=true;
					insert(edges[temp.elist[out]].n2);
				}else if (sum==2)
				{
					double c = 0;
					for (int i=0; i<4; i++)
					if (edges[temp.elist[i]].n2 == x)
					{
						c = c+edges[temp.elist[i]].con*edges[temp.elist[i]].v;
					}else{
						insert(edges[temp.elist[i]].n2);
					}
					if (in==3 && edges[temp.elist[0]].n2 == x)
						in = 0;
					double v = edges[temp.elist[in]].v;
					int mt = in;
					//cout<<"in "<<in<<endl;
					mt++;
					if (mt == 4)
						mt = 0;
					if (edges[temp.elist[mt]].v<v)
					{
						edges[temp.elist[mt]].con = edges[temp.elist[in]].con;
						edges[temp.elist[mt]].fc=true;
						//cout<<"first "<<mt<<endl;
						c = c-edges[temp.elist[mt]].con*edges[temp.elist[mt]].v;
						mt++;
						if (mt == 4)
							mt = 0;
						//cout<<"second "<<mt<<endl;
						edges[temp.elist[mt]].con = c/edges[temp.elist[mt]].v;
						edges[temp.elist[mt]].fc=true;
					}else{
						int lt = mt;
						lt++;
						if (lt == 4)
							lt = 0;
						int kt = lt;
						kt++;
						if (kt==4)
							kt = 0;
						edges[temp.elist[lt]].con = edges[temp.elist[kt]].con;
						edges[temp.elist[lt]].fc=true;
						//cout<<"first "<<mt<<endl;
						//cout<<"second "<<lt<<endl;
						c = c-edges[temp.elist[lt]].con*edges[temp.elist[lt]].v;
						edges[temp.elist[mt]].con = c/edges[temp.elist[mt]].v;
						edges[temp.elist[mt]].fc=true;
					} 
				}
			} 
		}else
		{
			insert(x);
		}
	}

	// for (int i=0; i<EDGESUM; i++)
	// 	cout<<i<<":   "<<edges[i].con<<endl;
}





int min (int a, int b)  //求最小值
{
	if (a>b)
		return b;
	else
		return a;
}

// double abs(double a)  //求绝对值
// {
// 	if (a>0)
// 		return a;
// 	return -a;
// }

void addline(int a,double leng)   //添加边
{
	if (edges[a].exist)
		return;
	else{
		if (a==81 && leng>1000){
			edges[0].exist = true;
			edges[0].leng = HALFLENG;
			edges[a].exist = true;
			edges[a].leng = HALFLENG;
		}else{
			edges[a].exist = true;
			edges[a].leng = leng;
		}
	}
}


void getentitieslwpolyline(ifstream& fin)   //读连线信息，按照折线的方式读入
{
	int a, sum, close;      //a为里面的预设参数，sum为统计点的数量，close为统计是否闭环
	int prihandle;            //polyline的内置参数
	string subclass, name, subclass2;    //都是dxf文件的定义参数
	double startx, starty, x, y, lastx, lasty;   //开始节点的x,y;正在读入的节点的x,y;上一个节点的x.y;
	fin>>a;
	if (a==5)
		fin>>prihandle;
	else 
	{
		cout<<"error handle"<<endl;
		return;
	}
	fin>>a;
	if (a==100)
		fin>>subclass;
	else{
		cout<<"error subclass"<<endl;
		return ;
	}
	fin>>a;
	if (a==8)
		fin>>name;
	else{
		cout<<"error name"<<endl;
		return ;
	}
	fin>>a;
	if (a==100)
		fin>>subclass2;
	else{
		cout<<"error subclass2"<<endl;
		return ;
	}
	fin>>a;
	if (a==90)
		fin>>sum;
	else{
		cout<<"error sum"<<endl;
		return ;
	}
	fin>>a;
	if (a==70)
		fin>>close;
	else{
		cout<<"error close"<<endl;
		return ;
	}

	for (int i=0; i<sum; i++)    //读入所有的节点
	{
		fin>>a;
		if (a==10)
		{
			fin>>x;
		}else
		{
			cout<<"error x"<<endl;
			return ;
		}fin>>a;
		if (a==20)				//横坐标
		{
			fin>>y;
		}else
		{
			cout<<"error y"<<endl;
			return ;
		}
		if (i==0)				//纵坐标
		{
			startx = x;			//存储初始节点
			starty = y;
		}else if (y>=0 && lasty>=0) {					
			if (abs(x-lastx)>1000)
			{
				int fir = x/1600;
				int sec = lastx/1600;
				int ty = y/1600;
				// cout<<"point1 "<<fir+ty*8+1<<endl;
				// cout<<"point2 "<<sec+ty*8+1<<endl;
				addline(56+ty+SPACE*min(sec,fir)+1,ALLLENG);   //addline表示对应的边是存在的
			}else if (abs(x-lastx)>500){
				if (x>5700)
					addline(0,HALFLENG);
				else
					addline(81,HALFLENG);
			}
			else if(abs(y-lasty)>1000)
			{
				int fir = y/1600;
				int sec = lasty/1600;
				int tx = x/1600;
				if (fir<8 && sec<8){
					if (fir>=0 && fir<8&& sec>=0&& sec<8)
						addline(SPACE*min(fir,sec)+tx+1,ALLLENG);
				}
			}
		}
		lastx = x;
		lasty = y;
	}

	if (close == 1){
		if (starty>=0 && lasty>=0) {					
			if (abs(startx-lastx)>1000)
			{
				int fir = startx/1600;
				int sec = lastx/1600;
				int ty = starty/1600;
				addline(56+ty+SPACE*min(sec,fir)+1,ALLLENG);   //addline表示对应的边是存在的
			}else if (abs(startx-lastx)>500){
				if (startx>5700)
					addline(0,HALFLENG);
				else
					addline(81,HALFLENG);
			}
			else if(abs(starty-lasty)>1000)
			{
				int fir = starty/1600;
				int sec = lasty/1600;
				int tx = startx/1600;
				if (fir<8 && sec<8){
					if (fir>=0 && fir<8&& sec>=0&& sec<8)
						addline(SPACE*min(fir,sec)+tx+1,ALLLENG);
				}
			}
		}
	}
}


int getdirline(int x, int dir){
	if (x==0){
		if (dir == 1)
			return 0;
		if (dir == 2)
			return 114;
		if (dir == 3)
			return 81;
		return false;
	}
	int i = (x-1)/PLACE;
	int j = x-i*PLACE-1;
	if (dir == 0){
		if (i==7 ){
			if (j==2)
				return 116;
			if (j==5)
				return 117;
		}
		return x;
	}
	if (dir == 1){
		if (x==4)
			return 81;
		return j*PLACE+i+57;
	}
	if (dir == 2){
		if (i==0){
			if (j==0 )
				return 113;
			if (j==7)
				return 115;
		}
		return x-PLACE;
	}
	if (dir == 3){
		if (x==5)
			return 0;
		return j*PLACE+i+57-PLACE;
	}
}

bool existdir(int x, int dir, int t){
	if (x==0){
		if (dir == 1)
			return edges[0].exist;
		if (dir == 2 && t>1)
			return true;
		if (dir == 3)
			return edges[81].exist;
		return false;
	}
	int i = (x-1)/PLACE;
	int j = x-i*PLACE-1;
	if (dir == 0){
		if (i==7 &&(j==2 || j==5 ))
			return true;
		if (i<SPACE-1)
			return edges[x].exist;
		else
			return false;
	}
	if (dir == 1){
		if (j<SPACE-1)
			return edges[j*PLACE+i+57].exist;
		else
			return false;
	}
	if (dir == 2){
		if (i==0 &&(j==0 || j==7 ))
			return true;
		if (i>0)
			return edges[x-PLACE].exist;
		else
			return false;
	}
	if (dir == 3){
		if (x==5)
			return edges[0].exist;
		if (j>0)
			return edges[j*PLACE+i+57-PLACE].exist;
		else
			return false;
	}
}


void printrect()
{
	int count = rect.size();
	cout<<"printrect "<<count<<endl;
    for (int i = 0; i < count;i++)
    {
        for (int j = 0; j < EDGESUM+1; j++)
        	cout<<rect[i][j]<<" ";
        cout<<endl;
    }
}

void addrect(vector<double> &tmp){
	rect.push_back(tmp);
}


void recursionline(int x, int dir, vector<double> &tmp,int start){
	int e = getdirline(x,dir);
	if (e>112)
		return;
	int x1;
	// cout<<"recursionline  e: "<<e<<"x: "<<x<<"dir: "<<dir<<endl;
	if (edges[e].n1 == x){
		tmp[e] = edges[e].leng;
		x1 = edges[e].n2;
	}
	else{
		tmp[e] = -edges[e].leng;
		x1 = edges[e].n1;
	}
	if (x1 == start){
		// cout<<"Adddrecr"<<endl;
		addrect(tmp);
		return;
	}
	for(int i=0; i<3; i++)
	{
		int newdir = dir-1+i;
		newdir = (newdir+4)%4;
		if (existdir(x1,newdir,1))
		{
			recursionline(x1,newdir,tmp,start);
			break;
		}
	}
}

void findline(int x){
	vector<double> tmp(EDGESUM+1,0);
	//cout<<"findline  "<<x<<endl;
	recursionline(x,1,tmp,x);
}


	int  recursionrect(int x, int ed,vector<double> &tmp, int end, int m)
	{
		// cout<<"recursionrect "<<x<<" "<<ed<<endl;
		if (ed==end)
		{
			tmp[ed] = edges[ed].leng;
			tmp[EDGESUM] = 0;
			return 1;
		}
		if (x==65)
			return 0;
		if (edges[ed].n1 == x)
			tmp[ed] = -edges[ed].leng;
		else
			tmp[ed] = edges[ed].leng;
		edges[ed].ft = true;
		for(int i=3; i>=0; i--)
		if (existdir(x,i,2))
		{
			// cout<<x<<" "<<i<<" 123123 "<<m<<endl;
			int e = getdirline(x,i);
			// cout<<e<<endl;
			if (!edges[e].ft){
				if (edges[e].n1 == x)
				{
					if (recursionrect(edges[e].n2,e,tmp,end,m+1)>0){
						// tmp[ed] = 0;
						edges[ed].ft = false;
						return 1;
					}
				}else{
					if (recursionrect(edges[e].n1,e,tmp,end,m+1)>0){
						// tmp[ed] = 0;
						edges[ed].ft = false;
						return 1;
					}
				}
			}	
		}	
		tmp[ed] = 0;
		edges[ed].ft = false;
		return 0;
	}
	void findrect(int x1){
		vector<double> tmp(EDGESUM+1,0);
		recursionrect(edges[x1].n2,x1,tmp,113,0);
		vector<double> temp(EDGESUM+1,0);
		recursionrect(edges[x1].n2,x1,temp,114,0);
		vector<double> tep(EDGESUM+1,0);
		recursionrect(edges[x1].n2,x1,tep,115,0);
		vector<double> emp(EDGESUM+1,0);
		for (int i=0; i<EDGESUM+1; i++)
			emp[i] = tmp[i]-temp[i];
		rect.push_back(emp);
		vector<double> tem(EDGESUM+1,0);
		for (int i=0; i<EDGESUM+1; i++)
			tem[i] = tep[i]-temp[i];
		rect.push_back(tem);

	}
void initrect(){    //统计行列式的值

	// cout<<rect.size()<<"  1321321231321321"<<endl;
	for (int i=0;i<EDGESUM-5; i++)
		if (!edges[i].exist)
		{
			vector<double> tmp(EDGESUM+1,0);
			tmp[i]=1;
			addrect(tmp);
		}else{
			// cout<<"exist "<<i<<endl;
		}
	// cout<<rect.size()<<"  21321321231321321"<<endl;


	for (int i=0; i<NODESUM-2; i++)  //首先根据基尔霍夫定律，统计所有的电流的相等关系
	if (nodes[i].elist.size()>0)
	{

		// cout<<"initrect  "<<i<<endl;
		vector<double> tmp(EDGESUM+1,0);
		for (int j=0; j<nodes[i].elist.size(); j++)
		{

			// cout<<"j  "<<j<<"  "<<nodes[i].elist[j]<<endl;
			if (edges[nodes[i].elist[j]].n1 == i)
				tmp[nodes[i].elist[j]] = 1;
			else
				tmp[nodes[i].elist[j]] = -1;
		}
		tmp[EDGESUM] = 0;
		addrect(tmp);
	}
	// cout<<rect.size()<<"  1321321231321321"<<endl;
	

	for (int i=0; i<PLACE-1; i++)
		for (int j=0; j<PLACE; j++)
		{
			int t = i*PLACE+j+1; 
			int m = j*PLACE+i+57; 
			if (edges[t].n1==t || edges[t].n2==t)
				if (edges[m].n1==t || edges[m].n2==t){
					// cout<<t<<endl;
					findline(t);
				}
		}
	// cout<<rect.size()<<"  1321321231321321"<<endl;
	// cout<<edges[114].leng<<endl;
	findrect(116);

	vector<double> tmp(EDGESUM+1,0);
	tmp[116] = 1;
	tmp[EDGESUM] = 200;
	addrect(tmp);
	vector<double> temp(EDGESUM+1,0);
	temp[117] = 1;
	temp[EDGESUM] = 200;
	addrect(temp);
	// cout<<rect.size()<<"  1321321231321321"<<endl;
}


double GreatestCommonDivisor(double a, double b)
{
	// cout<<"GreatestCommonDivisor "<<a<<" "<<b<<endl;
    double t;
    if (a < b)
    {
        t = a;
        a = b;
        b = t;
    }    
    if (b==1)
    	return 1;
    while (abs(b) >= NAX)
    {
        t = fmod(a , b);
        a = b;
        b = t;
    }
    // cout<<"a  "<<a<<endl;
    return a;
}

double LeastCommonMultiple (double a, double b)

{
	double c = GreatestCommonDivisor(a,b);
    double t = a  /c * b;
    return t;

}

void getans()    //计算行列式的值
{
	int n = rect.size();
	int num = 0;
	for (int i=0; i<n; i++){
		// cout<<"num "<<num<<" "<<i<<endl;
		if (abs(rect[i][num]) <NAX){
			int mjj = 0;
			for (int j=i+1; j<n; j++)
				if ( !(abs(rect[j][num])<NAX))
				{
					mjj++;
					for (int k=0; k<EDGESUM+1; k++)
					{
						double t = rect[j][k];
						rect[j][k] = rect[i][k];
						rect[i][k] = t;
					}
					break;
				}
			if (mjj == 0)
				cout<<"454354354354354"<<endl;
		}
		for (int j=0; j<n; j++)
			if (i!=j && (abs(rect[j][num])>NAX))
				{
					double ml = LeastCommonMultiple(abs(rect[j][num]),abs(rect[i][num]));
					double t = ml/rect[j][num];
					double kt = ml/rect[i][num];
					// cout<<"change "<<num<<" "<<j<<endl;
					// cout<<"t "<<t<<" kt "<<kt<<" ml "<<ml<<endl;
					// cout<<"rect[j][num] "<<rect[j][num]<<" rect[i][num] "<<rect[i][num]<<endl;
					for (int k=EDGESUM; k>=0; k--){
						rect[j][k] = rect[j][k]*t;
						rect[j][k] = (abs(rect[j][k])<NAX)?0:rect[j][k];
					}
					for (int k=EDGESUM; k>=num; k--)
					{
						rect[j][k] -= kt*rect[i][k];
						rect[j][k] = (abs(rect[j][k])<NAX)?0:rect[j][k];
					}


				}

			// string s = std::to_string(num);

			// string name = "test\\test"+s+".txt";
			// ofstream fmt(name);
			// for (int gg=0; gg<n; gg++){
			// 	for (int j=0; j<EDGESUM+1; j++)
			// 		fmt<<rect[gg][j]<<" ";
			// 	fmt<<endl;
			// }
			// fmt.close();

			num ++;
			for (int j=0; j<n; j++){
			// cout<<"mjj 132321  "<<j<<endl;
				double common = 0;
				for (int k=0; k<EDGESUM+1; k++)
				if ((abs(rect[j][k]) > NAX)){
					// cout<<common<<" "<<rect[j][k]<<" "<<k<<endl;
					if (abs(common) < NAX)
						common = abs(rect[j][k]);
					else
						common = GreatestCommonDivisor(common,abs(rect[j][k]));
					// cout<<"commonend "<<common<<endl;
					// cout<<"j "<<j<<" k "<<k<<" common "<<common<<endl;
				}
				if (common!=0)
					for (int k=0; k<EDGESUM+1; k++){
						rect[j][k] = rect[j][k]/common;
						rect[j][k] = (abs(rect[j][k])<NAX)?0:rect[j][k];
					}
			// printout(i*100);
			}
			// string s = std::to_string(num);

			// string name = "test\\test"+s+".txt";
			// ofstream fmt(name);
			// for (int gg=0; gg<n; gg++){
			// 	for (int j=0; j<EDGESUM+1; j++)
			// 		fmt<<rect[gg][j]<<" ";
			// 	fmt<<endl;
			// }
			// fmt.close();
		if (num==EDGESUM)
			break;
	}
	// cout<<"getans half"<<endl;
	
	num = 0;
	for (int i=0; i<EDGESUM; i++)
	{
		edges[num].v = double(rect[i][EDGESUM])/double(rect[i][num]);
		if (edges[num].v<0)
		{
			edges[num].v = -edges[num].v;
			int tm = edges[num].n1;
			edges[num].n1 = edges[num].n2;
			edges[num].n2 = tm;
		}
		num++;
		if (num==EDGESUM)
			break;
	}

	// 			cout<<nodes[18].elist.size()<<" 12312830198209381092380912830919823"<<endl;
	// for (int i=0; i<EDGESUM; i++){
	// 	cout<<i<<":   "<<edges[i].n1<<"  "<<edges[i].n2<<endl;
	// 	cout<<i<<":   "<<edges[i].v<<endl;
	// }

}

void initnode()    //初始化点和边的关系
{
	
	addline(113,length1);
	edges[113].n1 = 1;
	edges[113].n2 = 65;
	nodes[65].elist.push_back(113);
	addline(114,length2);
	edges[114].n1 = 0;
	edges[114].n2 = 65;
	nodes[65].elist.push_back(114);
	addline(115,length3);
	edges[115].n1 = 8;
	edges[115].n2 = 65;
	nodes[65].elist.push_back(115);
	addline(116,ALLLENG);
	edges[116].n2 = 59;
	edges[116].n1 = 66;
	edges[116].con = 100;
	edges[116].fc = true;
	nodes[66].elist.push_back(116);
	nodes[59].elist.push_back(116);
	addline(117,ALLLENG);
	edges[117].n2 = 62;
	edges[117].n1 = 66;
	edges[117].con = 0;
	edges[117].fc = true;
	nodes[66].elist.push_back(117);
	nodes[62].elist.push_back(117);
	for (int i=1; i<=PLACE*(PLACE-1); i++)
	{
		if (edges[i].exist){
			{
				edges[i].n2 = i;
				edges[i].n1 = i+PLACE;
				nodes[i].elist.push_back(i);
				//nodes[i+PLACE].elist.push_back(i);
			}
		}
	}

	if (edges[0].exist){
		edges[0].n2 = 0;
		edges[0].n1 = 5;
		nodes[0].elist.push_back(0);
	}
	for (int i=1; i<=PLACE*(PLACE-1); i++)
	{
		int x1 = (i-1)/PLACE;
		int y1 = i-x1*PLACE-1;
		int t = i+PLACE*(PLACE-1);
		if (edges[t].exist){
			if (t==81){
				edges[81].n2 = 0;
				edges[81].n1 = 4;
				//nodes[0].elist.push_back(81);
				nodes[4].elist.push_back(81);
			}else {
				edges[t].n2 = x1+y1*PLACE+2;
				edges[t].n1 = x1+y1*PLACE+1;
				//nodes[x1+y1*PLACE+2].elist.push_back(t);
				nodes[x1+y1*PLACE+1].elist.push_back(t);
			}
		}
	}

	nodes[1].elist.push_back(113);
	nodes[0].elist.push_back(114);
	nodes[8].elist.push_back(115);
	for (int i=1; i<=PLACE*(PLACE-1); i++)
	{
		if (edges[i].exist){
			nodes[i+PLACE].elist.push_back(i);
		}
	}



	if (edges[0].exist){
		edges[0].n2 = 0;
		edges[0].n1 = 5;
		nodes[5].elist.push_back(0);
	}

	for (int i=1; i<=PLACE*(PLACE-1); i++)
	{
		int x1 = (i-1)/PLACE;
		int y1 = i-x1*PLACE-1;
		int t = i+PLACE*(PLACE-1);
		if (edges[t].exist){
			if (t==81){
				nodes[0].elist.push_back(81);
			}else {
				edges[t].n2 = x1+y1*PLACE+2;
				edges[t].n1 = x1+y1*PLACE+1;
				nodes[x1+y1*PLACE+2].elist.push_back(t);
			}
		}
	}


	// for (int i=0; i<115; i++)
	// 	if (edges[i].exist)
	// 		cout<<"exist "<<i<<" "<<edges[i].n1<<" "<<edges[i].n2<<endl;
}


void getentities(ifstream& fin)//, ofstream& fout)    //对于定义连线的section，针对节点内容读信息
{

	while(!fin.eof())
	{
		int a;
		fin>>a;
		if (a == 0)
		{
			string st;
			fin>>st;
			if (st=="LWPOLYLINE")
				getentitieslwpolyline(fin);
			else if (st=="ENDSEC")
			{
				rect.clear();
				initnode();
				initrect();
				// cout<<"initrect"<<endl;
				getans();
				// cout<<"getans"<<endl;
				getconcentration();
				// cout<<"getconcentration"<<endl;
				// fout<<a<<endl;
				// fout<<st<<endl;
				return ;
			}
			else{
				cout<<"error 0 entities"<<endl;
				return ;
			}
		}else
		{
			cout<<"error entities"<<endl;
			return;
		}
	}
}


void trim(string &s)
{
    int index = 0;
    if( !s.empty())
    {
        while( (index = s.find(' ',index)) != string::npos)
        {
            s.erase(index,1);
        }
    }

}
void getelse(ifstream& fin)//, ofstream& fout)
{
	string a;
	string st;
	bool  end = false;
		// getline(fin,a);
	fin>>a;
	while(!fin.eof())
	{
		// getline(fin,a);

	fin>>a;
		//fout<<a<<endl;
		trim(a);
		if (end)
		{
			if (a=="ENDSEC")
				return;
			else 
				end =false;
		}
		if (a=="0")
		{
			end = true;
		}

	}
}

void getsection(ifstream& fin)//, ofstream& fout)    //按照section，即按照不同的板块读文件
{  
	string a;
	string st;
	fin>>a;
	fin>>st;
	// cout<<a<<endl;
	// cout<<st<<endl;
	if (a=="2")
		if (st=="ENTITIES")
			getentities(fin);//,fout);
		else
			getelse(fin);//,fout);
	else{
		cout<<"error 2 section"<<endl;
		return;
	}
	// cout<<"section"<<endl;
}

void init(char* filename)  //开始读文件
{
	rect.clear();
	for (int i=0; i<EDGESUM; i++)
	{
		edges[i].exist = false;
		edges[i].v = 0;
		edges[i].n1 = -1;
		edges[i].n2 = -1;
		edges[i].fl = false;
		edges[i].ft = false;
		edges[i].fc = false;
	}
	for (int i=0; i<NODESUM; i++)
	{
		nodes[i].elist.clear();
	}

	string sm = "new";
	sm = sm+filename;
	ifstream fin(filename);
	// ofstream fout(sm);
	while(!fin.eof())
	{
		string a;
		string st;
		fin>>a;
		// cout<<"a: "<<a<<endl;
		//fout<<a<<endl;
		fin>>st;
		// cout<<"st: "<<st<<endl;
		//fout<<st<<endl;
		if (a=="0")
			if (st=="SECTION")
			{
				getsection(fin);//, fout);
			}else if (st=="EOF")
			{
				// cout<<"end"<<endl;
				// cout<<sm<<endl;
				fin.close();
				// fout.close();
				return;
			}else
			{
				printf("error 0 SECTION \n");
				return;
			}
		else
		{
			printf("error SECTION \n");
			return;
		}


	}
}


int main(int argc, char ** argv)
{
	if (argc !=6 )
	{
		cout<<"use: calucon.exe filename length1 length2 length3 savename"<<endl;
		return 0;
	}
	std::stringstream ss2;
	ss2 << argv[2];
	ss2 >> length1;
	std::stringstream ss3;
	ss3 << argv[3];
	ss3 >> length2;
	std::stringstream ss4;
	ss4 << argv[4];
	ss4 >> length3;
	// length1 = strtod(argv[2],NULL);
	// length2 = strtod(argv[3],NULL);
	// length3 = strtod(argv[4],NULL);
	// cout<<length1<<" "<<length2<<" "<<length3<<endl;
	init(argv[1]);
	
	ofstream ft(argv[5]);
	ft<<edges[113].v/200<<" "<<edges[114].v/200<<" "<<edges[115].v/200<<endl;
	ft<<edges[113].con<<" "<<edges[114].con<<" "<<edges[115].con<<endl;
	ft<<edges[113].con*edges[113].v/200+edges[114].con*edges[114].v/200+edges[115].con*edges[115].v/200<<endl;
	cout<<edges[113].con<<" "<<edges[114].con<<" "<<edges[115].con<<endl;
	return 0;
}
