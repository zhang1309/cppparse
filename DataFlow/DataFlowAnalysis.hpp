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

//extern list<SymbolItem*> globalFunctions;//������������洢����AST�ĺ�����Ŀ wangyun 2008-11-18, by kong 12.01
void dataFlowAnalysis(CPPParser* p_theparser/*=0*/);
void dataFlowAnalysis(CPPParser* p_theparser,int deep);//by futeng:������ȱ���CFG�н�㣬deep������ʱ��ʵ�����壬Ϊ������
bool shouldNodeBeCheck(PATreeWalker &pawalker,VexNode * pNode); //�жϵ�����Ƿ���ҪԤ�����ĺ���

class CFGPaths;
class Path;
ostream& operator<<(ostream&, const Path&);
/************************************************************************/
/* ����·����Path��ʾ                                                   */
/************************************************************************/
enum AnalyzedFlagValue{NotAnalyzed=0,//:δ����
	NotReachable=1,//:���ɴ�δ����
	Analyzed=2//:�ѷ���
};

class Path
{
private:
	friend class CFGPaths;
	deque<VexNode*> m_path;			// ·���Ľڵ����
	//2010-08-11,ZLY,������Ӧ��־λ��¼��ǰ·��ÿ������Ƿ񱻷�����
	//��������·��ȡ��·�������Ϣʱ�����жϣ���Ҫȡ��Ϣ�Ľ��δ����������ݹ���ǰ���·������
	deque<AnalyzedFlagValue> m_AnalyzedFlag;		//default is false
	//2010-08-11,ZLY,END
	
	// FOR CXX & WANG YUN
	struct SubPath					// ˽��Ƕ�׽ṹ, ���ͷβ�����±�, ��ʾ·���е�һ��
	{
		int head; int tail;
	};
	deque<SubPath> m_circles;		// ·���е����л�
	bool m_circlesCalculated;
	deque<SubPath> m_compounds;		// ·�������еķ�֧-��Ͽ�
	bool m_compoundsCalculated;
	/*���һ��·����ĳ���ڵ�����ΪvCircle����vDoWhile����������ĳ�����Ŀ�ʼ���û��Ľ�β
	��Ȼ������ڵ�*/
	void calculateCircles();		// Ѱ��·���еĻ�
	/*��Ҳ���ڷ�֧����������ջ���*/
	void calculateCompounds();		// Ѱ��·���еķ�֧-��Ͽ�
	// FOR CXX & WANG YUN END
	
	void pushVex(VexNode* v, int& iVexCount, map<VexNode*, int>& mapVexCovered);		// ĳ���������ڵ������
	void markCoverVex(VexNode* v, int& iVexCount, map<VexNode*, int>& mapVexCovered);		// ��Ǹý���ѱ�����
	bool findVex(VexNode* v);		// ĳ���������ڵ��Ƿ���·����
	void backTrace(VexNode* v);		// ��·�����ݵ�ĳ���������ڵ�, Ҳ����ɾȥ·���иýڵ�֮������нڵ�.
	
public:
	Path(){ m_circlesCalculated = false; m_compoundsCalculated = false;}
	Path(deque<VexNode*>& p){
		m_path = p;  
		m_circlesCalculated = false; 
		m_compoundsCalculated = false;
		//2010-08-11,ZLY,BGING,��ʼ���������λ
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
		//2010-08-11,ZLY,BGING,��ʼ���������λ
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
		//2010-08-11,ZLY,BGING,��ʼ���������λ
		for(int i=0; i<m_path.size(); i++)
			m_AnalyzedFlag.push_back(NotAnalyzed);
		//2010-08-11,ZLY,END
		return *this;
	}

	VexNode* operator[](int cnt);			// �����±��������·�������еĽڵ�, ���Խ���򷵻�0
	int firstDifferentVex(Path& p)const;	// ��·��p���бȽ�, ���ص�һ����ͬ�Ľڵ���±�

	bool vexIsInCircle(int cnt);			// �����е�cnt�±�ڵ��Ƿ���ĳ������
	bool vexIsCircleHead(int cnt);			// �����е�cnt�±�ڵ��Ƿ���ĳ������ͷ�ڵ�
	bool vexIsCircleHead(VexNode* vex);		// vex�ڵ��Ƿ���·����ĳ������ͷ�ڵ�
	int length()const;						// ·���ĳ���
	bool empty()const;						// ·���Ƿ�Ϊ��

	//2010-08-11,ZLY,BGING,�������ȡָ�����ķ������λ
	//iPathNodeIndexΪ����ڵ�ǰ·���е��±�
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

	
	deque<int> getSwitches(int curIndex);	// ����ߵ�curIndex�±�ڵ�ʱ, �ѱ���·�������еķ�֧�ڵ���±�
											// FOR CXX & WANGYUN
	
	friend ostream& operator<<(ostream&, const Path&);

	// JUST TEST
	void displayCircle();			// �ݱ���
	void displayCompounds();		// �ݱ���
	// TEST END

	deque<VexNode*> getVexNodes() { // by kong 20100112
	    return m_path;	
	}
};



/************************************************************************/
/* ��CFGPaths��������һ��������Ŀ��Ӧ�Ŀ�����ͼ������·��               */
/************************************************************************/

ostream& operator<<(ostream&, const CFGPaths&);
class CFGPaths
{
private:
	SymbolItem* m_func;				// �ĸ�������Ŀ��Ӧ�Ŀ�����ͼ·��
	deque<Path> m_paths;			// ����func��Ӧ�Ŀ�����ͼ������·��
	int m_curPathCount;				// getNextPathʱ����ָ���ص��Ǽ���·��	
	
	void walkCFG(CFG* cfg);			// ����cfg�е�����·��, ��������m_paths�� 
	void markDeadCircle(CFG*);		// ���CFG�е�����
	int CalculateCoverCount(CFG *cfg);
	
public:
	CFGPaths(SymbolItem* f=0);		
	CFGPaths& operator=(const CFGPaths&);
	~CFGPaths(){}
	
	bool getNextPath(Path& path, int& cnt);	// ÿ����һ�λ�ȡ��һ��·��, ������path��, ������ǰ�ǵڼ���·��д��cnt
											// ���·����ȫ��ȡ��, ����false
	Path* operator[](int cnt);				// �����±��õ�cnt��·��, ��Խ���򷵻�һ����·��
	int size()const;						// ·��������
	void reset();							// ����, ʹ��getNextPath���¿�ʼ���·��
	bool empty()const;
	
	friend ostream& operator<<(ostream&, const CFGPaths&);

	Path* getPath(int cnt); // ��õ�cnt��·������cnt=0��ʼ���·�� by kong 20100111
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
	class BranchVexStack			// ������, ������¼��֧�ڵ�
	{
	private:
		list<VexNode*> stk;
	public:
		void push(VexNode* v);
		void pop();
		VexNode* top();
		bool empty();
		int find(VexNode* vex);		// �ڵ��Ƿ��Ѽ�¼
		void display();
	};	
};


/************************************************************************/
/* ��CFGPathsManager���������ѷ���������·��                            */
/************************************************************************/
class CFGPathsManager
{
private:
	static CFGPathsManager* m_instance;		// ����ģʽ
	map<SymbolItem*, CFGPaths> m_mapPaths;	// �������溯����Ŀ�����Ӧ��·��
	CFGPathsManager(){}
public:
	static CFGPathsManager* instance();
	CFGPaths& pathsOfFunction(SymbolItem*);	// ��ȡĳ��������Ŀ�Ŀ�����·��
	~CFGPathsManager()
	{
		if(m_instance) delete m_instance;
	}
};




/************************************************************************/
/* ��������ջ, �����ж��Ƿ����ݹ����                                 */
/************************************************************************/
class CallStack;
ostream& operator<<(ostream&, const CallStack& c);
class CallStack
{
private:
	list<SymbolItem*> callStack;
	//2010-9-16,ZLY,BEGIN,�������ջ�����к����ķ���·���ϵĽڵ�
	list< list<int> > curPaths;
	//2010-9-16,ZLY,END,�������ջ�����к����ķ���·���ϵĽڵ�
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
	void popCurrentFuncNode();//��ÿ�������ú����������ʱ���øú��������ú�����ջ�еĽ��ȫ�����
	const deque<int>& getSwitchesInStack(VexNode* v,int index,bool reAalculation=false);
	void analyzedNodeStack::getSwitchesInStack(VexNode* v,stack<VexNode*>& circlestk,stack<int>& stk);
	void setUpdateSwitchDeque(bool b){updateSwitchDeque=b;}
	bool getUpdateSwithcDeque(){return updateSwitchDeque;}
	const vector<VexNode*>& getNodeStack()const{return nodeStack;}
	void printStack();//������
	size_t size(){return nodeStack.size();}
	void popMore(int n);
};
ostream& operator<<(ostream &o,const analyzedNodeStack& stk);
/************************************************************************/
/* ������������                                                         */
/************************************************************************/
enum pathToTravel{FalsePath=0,TruePath,BothPath};
extern bool isReachSink;
extern bool g_project_quit_on_sink;
class DataFlowAnalyzer
{
private:
	bool bTainedSinkFlag4VexNode;	//��������������Ƿ񴥷���tained sink�ı��
	bool bTainedSinkFlag4Func;		//��ǰ�����Ƿ񴥷���tained sink�ı��
public:
	void setTainedSinkFlag4VexNode(bool setVexSink = true){
		if(g_project_quit_on_sink)
		{
			openConsole();
			cout<<endl<<"����sink�㣬��������ں���"<<endl;
			closeConsole();
		}else
		{
			openConsole();
			cout<<endl<<"��sink��"<<endl;
			closeConsole();
		}
		isReachSink = true;
		bTainedSinkFlag4VexNode = true;
		bTainedSinkFlag4Func = true;	//��ĳ��㴥����sink������������sink
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
		bTainedSinkFlag4Func = false;	//�ָ�����sink��Ϣ
	}
private:
	ExprValueTreeWalker* m_treeWalker;	// �������ͱ�������������
	//InfoTreeWalker *infoTreeWalker;
	SymbolItem* m_func;					// ������������Ŀ
	IntegerValueSet m_returnValues;		// �����ռ����������������ͷ���ֵ, ����еĻ� 
	MingledSymbolItem m_thisPtr;
	//futeng modify 2015-6-25
	int analyzeNodeIndex;				//����ڵ�һ·���еı��
	int theRestPathSize;				//��������·����
	int beingAnalysisPathIndex;			//���ڱ�������·���ı��
	//futeng modify end
	void pushCallStack();		// ����ǰ�����ĺ�����Ŀѹ�뺯������ջ
	void popCallStack();		// ������Ŀ����������ջ
	bool needAnalysis();		// ���ú�������ջ�ж��Ƿ����ݹ����, 
								// �Դ��ж��Ƿ���Ҫ�Ե�ǰ������Ŀ���з���
	void analyzeNode(VexNode *node,VexNode *preVode,ArcBox *arc,bool isInterprocedural);
	void MarkTainedVar(VexNode *curNode);
	bool recordOnePathInfo(PATreeWalker& pawalker);
public:
	static CallStack m_callStack;
	int pathBranchCount;
	static analyzedNodeStack nodestack;
	//deque<pair<int,VexNode*> > switchHeadDeque;
	pathToTravel choosePath(VexNode* cruNode,VexNode* preNode);
	bool findNodeInNodeStack(VexNode *node);//����whileѭ����β��㣬��ʹ���false������ѭ����������һ���ڵ�
	CPPParser* p_parser;        //xulei add, 20100408
	PATreeWalker& pawalker;
	DataFlowAnalyzer(SymbolItem* f ,PATreeWalker & p, CPPParser * p_p);
	~DataFlowAnalyzer();
	deque<pair<int,VexNode*> >& getSwitches(VexNode *node,int curIndex);
	void analyze(int isInterprocedural/*=false*/);				// �Ժ�����Ŀ���з���    //������ʾ�Ƿ��ǿ���̷��������ڽ��������ƣ�û�б���ô�,xulei
	void analyze(int isInterprocedural,int deep);//by futeng:deep��ʵ����˼,Ϊ������
	//xulei add "curVex_id", ��ʾ��ǰ�ڵ��������е��±꣬Ϊ�������һ���ڵ����ġ���һ���ڵ�ΪcurPath[curVex_id+1]���������á�20100408
	bool DataFlowAnalyzer::visitVex(VexNode* v, /*int curVex_id*/ VexNode* preVex , PATreeWalker& pawalker ,int Paths,/*const deque<int>& dq,*/ArcBox *arc, bool is_reachable, int pathSize, bool isInterprocedural,ValueParameter& judgeType); // DAY FOR CXX  
	
	ExprValueTreeWalker* getExprTreeWalker(){return m_treeWalker;}
	SymbolItem* getFunc(){return m_func;}

	static CallStack& getCallStack(){return m_callStack;}	// ���غ�������ջ			
	
	void addReturnValues(const IntegerValueSet& v)	// treeWalker���ֱ�����������return���ʱ
	{ m_returnValues.addValues(v); }				// ���ø÷��������ͷ���ֵ��Ϣ��¼������������������������
			
	IntegerValueSet& getReturnValues()		// �������õ㴦, ���ø÷�����ñ������������ͷ���ֵ
	{ return m_returnValues;	}
	
	void setEntranceValues(const VexValueSet&);	// ��ʼ����һ������֮ǰ, ���ø÷������ú��������ֵ
	VexValueSet getExitValues();					// ������������֮��, ���ø÷�����ȡ����������ֵ

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