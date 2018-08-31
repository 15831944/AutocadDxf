
Valve nowValves[LINEMAX];
int nowValveCnt;

struct HalfLine
{
	Point p;
	double angle;
	double dx, dy;
	int id;
}valveLines[LINEMAX];

struct NodePoint
{
	Point p;
	int id1, id2;
}lineNodes[LINEMAX];
int valveLineCnt, lineNodeCnt;

void sendHalfLine(Point p, double angle, int id)
{
	HalfLine now;
	now.p = p; now.angle = angle;
	if (fabs(angle - 0.0) < E)	now.dx = 1.0, now.dy = 0.0;
	else if (fabs(angle - 90.0) < E)	now.dx = 0.0, now.dy = 1.0;
	else if (fabs(angle - 180.0) < E)	now.dx = -1.0, now.dy = 0.0;
	else if (fabs(angle - 270.0) < E)	now.dx = 0.0, now.dy = -1.0;
	now.id = id;
	valveLines[++valveLineCnt] = now;
	//cout << "Halfline " << p.x << ' ' << p.y << ' ' << angle << ' ' << valveLineCnt << ' ' << now.dx << ' ' << now.dy << endl;
}

void addPipeLine(Point a, Point b, int &cnt, Line lines[], int layer, int dir)
{
	Line now;
	now.a = a; now.b = b; now.dir = dir;
	if (layer == 1)	now.name = LAYER1;
	else	now.name = LAYER2;
	lines[++cnt] = now;
	addExtraLine(a, b, now.name);
}

void getNodePoint(ofstream &fout, HalfLine a, HalfLine b, int &nodeCnt, NodePoint nodes[])
{
	double dx, dy, px, py;
	dx = a.p.x - b.p.x; dy = a.p.y - b.p.y;
	NodePoint np;
	fout  << a.dx << ' ' << b.dx << ' ' << dx << ' ' << a.dy << ' ' << b.dy << ' ' << dy << endl;
	if ((dx * a.dx < -E || dx * b.dx > E) && (dy * a.dy < -E || dy * b.dy > E) && allValve[a.id].name == allValve[b.id].name)
	{
		px = dx / (b.dx - a.dx) * b.dx + b.p.x;
		py = dy / (b.dy - a.dy) * b.dy + b.p.y;
		np.id1 = a.id; np.id2 = b.id;
		np.p.x = px; np.p.y = py;
		nodes[++nodeCnt] = np;
		fout << "OK " << a.p.x << ' ' << a.p.y << ' ' << a.angle << ' ' << b.p.x << ' ' << b.p.y << ' ' << b.angle << ' ' << px << ' ' << py << endl;
		if (fabs(px - a.p.x) < E)	addPipeLine(np.p, a.p, linePipeXCnt, linePipeX, allValve[a.id].layer, 0);
		if (fabs(py - a.p.y) < E)	addPipeLine(np.p, a.p, linePipeYCnt, linePipeY, allValve[a.id].layer, 1);
		if (fabs(px - b.p.x) < E)	addPipeLine(np.p, b.p, linePipeXCnt, linePipeX, allValve[b.id].layer, 0);
		if (fabs(py - b.p.y) < E)	addPipeLine(np.p, b.p, linePipeYCnt, linePipeY, allValve[b.id].layer, 1);
	}
}

void getAllNodes(int &cnt, HalfLine lines[], int &nodeCnt, NodePoint nodes[])
{
	ofstream fout ("allPipeLines.txt");
	fout << "PipeNodes: " << cnt << endl;
	int i, j;
	for (i = 1; i <= cnt; i++)
		for (j = i + 1; j <= cnt; j++)
		{
			getNodePoint(fout, lines[i], lines[j], nodeCnt, nodes);
			
		}
	outputLines(fout, linePipeXCnt, linePipeX, "LinePipeX: ");
	outputLines(fout, linePipeYCnt, linePipeY, "LinePipeY: ");
}

void mergeValveSameY(int &cnt, Valve valves[])
{
	int i;
	valveLineCnt = lineNodeCnt = 0;
	for (i = 1; i <= cnt; i++)
	{
		if (fabs(valves[i].angle - 0.0) < E || fabs(valves[i].angle - 180.0) < E)
			sendHalfLine(valves[i].center, 0.0, i), sendHalfLine(valves[i].center, 180.0, i);
		if (fabs(valves[i].angle - 90.0) < E || fabs(valves[i].angle - 270.0) < E)
			sendHalfLine(valves[i].center, 90.0, i), sendHalfLine(valves[i].center, 270.0, i);
	}
	getAllNodes(valveLineCnt, valveLines, lineNodeCnt, lineNodes);
}

void buildPipes(void)
{
	int i, j;
	for (i = 1; i <= orderCnt; i++)
	{
		for (nowValveCnt = 0, j = 1; j <= valveCnt; j++)	if (allValve[j].order == i)	nowValves[++nowValveCnt] = allValve[j];
		sort(nowValves + 1, nowValves + nowValveCnt + 1, cmpValveXW);
		mergeValveSameY(nowValveCnt, nowValves);
	}
}
