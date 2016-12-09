//////////////////////////////////////////////////////////////////////////
//	Author: Day															//
//																		//
//////////////////////////////////////////////////////////////////////////
#ifndef __DATAFLOWANALYSIS_HPP__
#define __DATAFLOWANALYSIS_HPP__
#pragma warning(disable:4786)

#include "ValueStructure.hpp"
#include "../CFG/CFGStructure.h"
#include "Assistant.h"
#include "../CPPParser.hpp"
#include "../ConditionTreeWalker.hpp"
//#include "../PATreeWalker.hpp"//cxx 0601
#include "InforFlowCollector.h" // by kong 20100520
#include <list>
#include <set>
#include <map>
#include <deque>
#include <algorithm>
using namespace std;

extern ofstream dataflow;
extern VexValueSet staticValues;
extern IntegerValueSet returnValues;
extern VexValueSet exitValues;

//extern list<SymbolItem*> globalFunctions;//这个变量用来存储具有AST的函数条目 wangyun 2008-11-18, by kong 12.01
void dataFlowAnalysis(CPPParser* p_theparser/*=0*/);
void dataFlowAnalysis(CPPParser* p_theparser,int deep);//by futeng:深度优先遍历CFG中结点，deep参数暂时无实际意义，为了重载
bool shouldNodeBeCheck(PATreeWalker &pawalker,VexNode * pNode); //判断当结点是否需要预遍历的函数

class CFGPaths;
class Path;
ostream& operator<<(ostream&, const Path&);
/************************************************************************/
/* 单条路径用Path表示                                                   */
/************************************************************************/
enum AnalyzedFlagValue{NotAnalyzed=0,//:未分析
	NotReachable=1,//:不可达未分析
	Analyzed=2//:已分析
};

class Path
{
private:
	friend class CFGPaths;
	deque<VexNode*> m_path;			// 路径的节点队列
	//2010-08-11,ZLY,增加相应标志位记录当前路径每个结点是否被分析过
	//用于下条路径取本路径结点信息时进行判断，如要取信息的结点未分析过，则递归向前面的路径查找
	deque<AnalyzedFlagValue> m_AnalyzedFlag;		//default is false
	//2010-08-11,ZLY,END
	
	// FOR CXX & WANG YUN
	struct SubPath					// 私有嵌套结构, 标记头尾两个下标, 表示路径中的一段
	{
		int head; int tail;
	};
	deque<SubPath> m_circles;		// 路径中的所有环
	bool m_circlesCalculated;
	deque<SubPath> m_compounds;		// 路径中所有的分支-汇合块
	bool m_compoundsCalculated;
	/*如果一段路径中某个节点类型为vCircle或者vDoWhile，代表这是某个环的开始，该环的结尾
	仍然是这个节点*/
	void calculateCircles();		// 寻找路径中的环
	/*环也属于分支，，借助于栈求的*/
	void calculateCompounds();		// 寻找路径中的分支-汇合块
	// FOR CXX & WANG YUN END
	
	void pushVex(VexNode* v, int& iVexCount, map<VexNode*, int>& mapVexCovered);		// 某个控制流节点入队列
	void markCoverVex(VexNode* v, int& iVexCount, map<VexNode*, int>& mapVexCovered);		// 标记该结点已被覆盖
	bool findVex(VexNode* v);		// 某个控制流节点是否在路径中
	void backTrace(VexNode* v);		// 将路径回溯到某个控制流节点, 也就是删去路径中该节点之后的所有节点.
	
public:
	Path(){ m_circlesCalculated = false; m_compoundsCalculated = false;}
	Path(deque<VexNode*>& p){
		m_path = p;  
		m_circlesCalculated = false; 
		m_compoundsCalculated = false;
		//2010-08-11,ZLY,BGING,初始化分析标记位
		for(int i=0; i<m_path.size(); i++)
			m_AnalyzedFlag.push_back(NotAnalyzed);
		//2010-08-11,ZLY,END
	}
	Path(const Path& p)
	{
		m_path = p.m_path; 	m_circles = p.m_circles; 
		m_circlesCalculated = p.m_circlesCalculated; 
		m_compoundsCalculated = p.m_compoundsCalculated;
		m_compounds = p.m_compounds;
		//2010-08-11,ZLY,BGING,初始化分析标记位
		for(int i=0; i<m_path.size(); i++)
			m_AnalyzedFlag.push_back(NotAnalyzed);
		//2010-08-11,ZLY,END
	}
	Path& operator = (const Path& p)
	{
		if(this == &p) return *this;
		m_path = p.m_path; m_circles = p.m_circles; 
		m_circlesCalculated = p.m_circlesCalculated; 
		m_compounds = p.m_compounds;
		m_compoundsCalculated = p.m_compoundsCalculated;
		//2010-08-11,ZLY,BGING,初始化分析标记位
		for(int i=0; i<m_path.size(); i++)
			m_AnalyzedFlag.push_back(NotAnalyzed);
		//2010-08-11,ZLY,END
		return *this;
	}

	VexNode* operator[](int cnt);			// 利用下标随机访问路径队列中的节点, 如果越界则返回0
	int firstDifferentVex(Path& p)const;	// 与路径p进行比较, 返回第一个不同的节点的下标

	bool vexIsInCircle(int cnt);			// 队列中的cnt下标节点是否在某个环中
	bool vexIsCircleHead(int cnt);			// 队列中的cnt下标节点是否是某个环的头节点
	bool vexIsCircleHead(VexNode* vex);		// vex节点是否是路径中某个环的头节点
	int length()const;						// 路径的长度
	bool empty()const;						// 路径是否为空

	//2010-08-11,ZLY,BGING,设置与获取指定结点的分析标记位
	//iPathNodeIndex为结点在当前路径中的下标
	void resetAllNodeFlag(AnalyzedFlagValue flag = NotAnalyzed){
		for(int i=0; i<m_AnalyzedFlag.size(); i++){
			m_AnalyzedFlag[i] = flag;
		}
	}
	void setAnalyzedFlag(int iPathNodeIndex, AnalyzedFlagValue flag){
		if(iPathNodeIndex >= 0 && iPathNodeIndex < m_AnalyzedFlag.size())
			m_AnalyzedFlag[iPathNodeIndex] = flag;
	}
	AnalyzedFlagValue getAnalyzedFlag(int iPathNodeIndex){
		if(iPathNodeIndex >= 0 && iPathNodeIndex < m_AnalyzedFlag.size())
			return m_AnalyzedFlag[iPathNodeIndex];
		return NotAnalyzed;
	}
	//2010-08-11,ZLY,END

	
	deque<int> getSwitches(int curIndex);	// 获得走到curIndex下标节点时, 已遍历路径上所有的分支节点的下标
											// FOR CXX & WANGYUN
	
	friend ostream& operator<<(ostream&, const Path&);

	// JUST TEST
	void displayCircle();			// 暂保留
	void displayCompounds();		// 暂保留
	// TEST END

	deque<VexNode*> getVexNodes() { // by kong 20100112
	    return m_path;	
	}
};



/************************************************************************/
/* 类CFGPaths用来保存一个函数条目对应的控制流图的所有路径               */
/************************************************************************/

ostream& operator<<(ostream&, const CFGPaths&);
class CFGPaths
{
private:
	SymbolItem* m_func;				// 哪个函数条目对应的控制流图路径
	deque<Path> m_paths;			// 保存func对应的控制流图的所有路径
	int m_curPathCount;				// getNextPath时用来指返回的是几条路径	
	
	void walkCFG(CFG* cfg);			// 搜索cfg中的所有路径, 并保存入m_paths中 
	void markDeadCircle(CFG*);		// 标记CFG中的死环
	int CalculateCoverCount(CFG *cfg);
	
public:
	CFGPaths(SymbolItem* f=0);		
	CFGPaths& operator=(const CFGPaths&);
	~CFGPaths(){}
	
	bool getNextPath(Path& path, int& cnt);	// 每调用一次获取下一条路径, 保存在path中, 并将当前是第几条路径写入cnt
											// 如果路径已全部取出, 返回false
	Path* operator[](int cnt);				// 利用下标获得第cnt条路径, 若越界则返回一条空路径
	int size()const;						// 路径的数量
	void reset();							// 重置, 使得getNextPath重新开始获得路径
	bool empty()const;
	
	friend ostream& operator<<(ostream&, const CFGPaths&);

	Path* getPath(int cnt); // 获得第cnt条路径，从cnt=0开始编号路径 by kong 20100111
    bool isCFGPaths() {  // by kong 20100119
		if (m_func == NULL) {
		    return false;
		}
		else 
			return true;
	}

	// by kong 20100520
	SymbolItem* getFunc() {
		return m_func;
	}
	void sortPaths();

private:
	class BranchVexStack			// 辅助类, 用来记录分支节点
	{
	private:
		list<VexNode*> stk;
	public:
		void push(VexNode* v);
		void pop();
		VexNode* top();
		bool empty();
		int find(VexNode* vex);		// 节点是否已记录
		void display();
	};	
};


/************************************************************************/
/* 类CFGPathsManager用来保存已分析函数的路径                            */
/************************************************************************/
class CFGPathsManager
{
private:
	static CFGPathsManager* m_instance;		// 单件模式
	map<SymbolItem*, CFGPaths> m_mapPaths;	// 用来保存函数条目和其对应的路径
	CFGPathsManager(){}
public:
	static CFGPathsManager* instance();
	CFGPaths& pathsOfFunction(SymbolItem*);	// 获取某个函数条目的控制流路径
	~CFGPathsManager()
	{
		if(m_instance) delete m_instance;
	}
};




/************************************************************************/
/* 函数调用栈, 用来判断是否发生递归调用                                 */
/************************************************************************/
class CallStack;
ostream& operator<<(ostream&, const CallStack& c);
class CallStack
{
private:
	list<SymbolItem*> callStack;
	//2010-9-16,ZLY,BEGIN,保存调用栈中所有函数的分析路径上的节点
	list< list<int> > curPaths;
	//2010-9-16,ZLY,END,保存调用栈中所有函数的分析路径上的节点
public:
	int getStackSize(){return callStack.size();}
	CallStack(){}
	void push(SymbolItem* s);	
	void pop();
	SymbolItem* top(){return callStack.back();}
	bool find(SymbolItem* s);
	friend ostream& operator<<(ostream&, const CallStack& c);
	list<int> * getCurPathInfo();
	string getAllFunctionInStack();
	string getAllPathsInStack();
};


class PATreeWalker;
class ExprValueTreeWalker;
class InfoTreeWalker;

class analyzedNodeStack
{
private:
	vector<VexNode*> nodeStack;
	deque<int> getSwitch;
	bool updateSwitchDeque;
public:
	inline void push(VexNode* v);
	void pop();
	int findNodeInStack(VexNode* v);
	bool matchNodeAndPop(VexNode* v);
	void popCurrentFuncNode();//当每个被调用函数分析完毕时调用该函数，将该函数在栈中的结点全部清空
	const deque<int>& getSwitchesInStack(VexNode* v,int index,bool reAalculation=false);
	void analyzedNodeStack::getSwitchesInStack(VexNode* v,stack<VexNode*>& circlestk,stack<int>& stk);
	void setUpdateSwitchDeque(bool b){updateSwitchDeque=b;}
	bool getUpdateSwithcDeque(){return updateSwitchDeque;}
	const vector<VexNode*>& getNodeStack()const{return nodeStack;}
	void printStack();//调试用
	size_t size(){return nodeStack.size();}
	void popMore(int n);
};
ostream& operator<<(ostream &o,const analyzedNodeStack& stk);
/************************************************************************/
/* 数据流分析器                                                         */
/************************************************************************/
enum pathToTravel{FalsePath=0,TruePath,BothPath};
extern bool isReachSink;
extern bool g_project_quit_on_sink;
class DataFlowAnalyzer
{
private:
	bool bTainedSinkFlag4VexNode;	//单个控制流结点是否触发了tained sink的标记
	bool bTainedSinkFlag4Func;		//当前函数是否触发了tained sink的标记
public:
	void setTainedSinkFlag4VexNode(bool setVexSink = true){
		if(g_project_quit_on_sink)
		{
			openConsole();
			cout<<endl<<"到达sink点，回退至入口函数"<<endl;
			closeConsole();
		}else
		{
			openConsole();
			cout<<endl<<"到sink点"<<endl;
			closeConsole();
		}
		isReachSink = true;
		bTainedSinkFlag4VexNode = true;
		bTainedSinkFlag4Func = true;	//若某结点触发了sink，则函数触发了sink
		if(setVexSink && nodestack.size()>0)
		{
			const vector<VexNode*>& stk = nodestack.getNodeStack();
			stk.back()->setVexIsSinked();
		}
	}
	bool getTainedSinkFlag4VexNode(){return false /*bTainedSinkFlag4VexNode*/;}
	bool getTainedSinkFlag4Func(){return bTainedSinkFlag4Func;}
	void recoverSinkFlag4VexNode(){
		bTainedSinkFlag4VexNode = false;
		bTainedSinkFlag4Func = false;	//恢复结点的sink信息
	}
private:
	ExprValueTreeWalker* m_treeWalker;	// 分析整型变量的树遍历器
	//InfoTreeWalker *infoTreeWalker;
	SymbolItem* m_func;					// 待分析函数条目
	IntegerValueSet m_returnValues;		// 用来收集被分析函数的整型返回值, 如果有的话 
	MingledSymbolItem m_thisPtr;
	//futeng modify 2015-6-25
	int analyzeNodeIndex;				//结点在单一路径中的编号
	int theRestPathSize;				//待分析的路径数
	int beingAnalysisPathIndex;			//正在被分析的路径的编号
	//futeng modify end
	void pushCallStack();		// 将当前分析的函数条目压入函数调用栈
	void popCallStack();		// 函数条目分析结束后弹栈
	bool needAnalysis();		// 利用函数调用栈判断是否发生递归调用, 
								// 以此判断是否需要对当前函数条目进行分析
	void analyzeNode(VexNode *node,VexNode *preVode,ArcBox *arc,bool isInterprocedural);
	void MarkTainedVar(VexNode *curNode);
	bool recordOnePathInfo(PATreeWalker& pawalker);
public:
	static CallStack m_callStack;
	int pathBranchCount;
	static analyzedNodeStack nodestack;
	//deque<pair<int,VexNode*> > switchHeadDeque;
	pathToTravel choosePath(VexNode* cruNode,VexNode* preNode);
	bool findNodeInNodeStack(VexNode *node);//发现while循环的尾结点，并使其从false边跳出循环，分析下一个节点
	CPPParser* p_parser;        //xulei add, 20100408
	PATreeWalker& pawalker;
	DataFlowAnalyzer(SymbolItem* f ,PATreeWalker & p, CPPParser * p_p);
	~DataFlowAnalyzer();
	deque<pair<int,VexNode*> >& getSwitches(VexNode *node,int curIndex);
	void analyze(int isInterprocedural/*=false*/);				// 对函数条目进行分析    //参数表示是否是跨过程分析，用于进度条控制，没有别的用处,xulei
	void analyze(int isInterprocedural,int deep);//by futeng:deep无实际意思,为了重载
	//xulei add "curVex_id", 表示当前节点在数组中的下标，为了求得下一个节点加入的。下一个节点为curPath[curVex_id+1]。仅此作用。20100408
	bool DataFlowAnalyzer::visitVex(VexNode* v, /*int curVex_id*/ VexNode* preVex , PATreeWalker& pawalker ,int Paths,/*const deque<int>& dq,*/ArcBox *arc, bool is_reachable, int pathSize, bool isInterprocedural,ValueParameter& judgeType); // DAY FOR CXX  
	
	ExprValueTreeWalker* getExprTreeWalker(){return m_treeWalker;}
	SymbolItem* getFunc(){return m_func;}

	static CallStack& getCallStack(){return m_callStack;}	// 返回函数调用栈			
	
	void addReturnValues(const IntegerValueSet& v)	// treeWalker发现被分析函数的return语句时
	{ m_returnValues.addValues(v); }				// 调用该方法将整型返回值信息记录到它的宿主数据流分析器中
			
	IntegerValueSet& getReturnValues()		// 函数调用点处, 利用该方法获得被调函数的整型返回值
	{ return m_returnValues;	}
	
	void setEntranceValues(const VexValueSet&);	// 开始分析一个函数之前, 调用该方法设置函数入口流值
	VexValueSet getExitValues();					// 函数分析结束之后, 调用该方法获取函数出口流值

	void setThisPtr(const MingledSymbolItem& m){m_thisPtr = m;}
	MingledSymbolItem& getThisPtr(){return m_thisPtr;}

	PATreeWalker& getPAWalker(){return pawalker;}

	void travelVex(VexNode* v,bool isFirstNodeOnPath,PATreeWalker& pawalker,ExprValueTreeWalker* expTreewalker,bool IsTrueEdge);
	void handleVexState(VexNode* v,bool isFirstNodeOnPath,PATreeWalker& pawalker,ExprValueTreeWalker* expTreewalker,bool IsTrueEdge);
    void addInforFlow(PATreeWalker& pawalker, bdd curBdd, VexNode* v); // by kong 20100520
	int findRealID(int tempID);	// 20100707 by kong
	//void DataFlowAnalyzer::GetMaxSameSubPath(CFGPaths& paths, int iCurPathIndex, int& iFirstDiffVex, int& iPathIndex);
};




#endif