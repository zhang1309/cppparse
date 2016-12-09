//////////////////////////////////////////////////////////////////////////
//	Author: Day															//
//																		//
//////////////////////////////////////////////////////////////////////////
#define CRTDBG_MAP_ALLOC
#pragma warning(disable:4786)
#include<assert.h>
#include<crtdbg.h>
#include <iostream>
#include <stack>
#include "DataFlowAnalysis.hpp"
#include "../ExprValuetreeWalker.hpp"
#include "../GlobalVariableExprWalker.hpp"
#include "../PA/PAanalysis.h"
#include "..\Covert Channel\TraveCFGGetInforFlow.h"
#include "..\Covert Channel\flowGraph.h"
#include "..\FuncSort\FuncSort.h"
#include <sstream>
#include "..\SM\ProgramStateManager.h"
#include "InforFlowCollector.h"
#include "../ASTInsTreeWalkerTokenTypes.hpp"        //xulei add. 20100414
#include "../STDCTokenTypes.hpp"    //xulei add. 20100414
#include "../ConditionTreeWalker.hpp"
#include "../InfoTreeWalker.hpp"
#include "../progressbar.h"
#include "travelCircle.h"
#include "../NewFuncSort/FuncCallScaner.h"
#include <set>
#include<unordered_map>
#include <queue>
#include "../ConfigFileParser/ConfigInfo_Cpp.h"
#include "../golbalSupport.h"
#include "..\FunctionManager.h"
#include "../TraverseTreeWalker.hpp"
#include "../ModuleManager.h"
#include "../integerInfo.h"
//#define SM_RESTORE_TRACE
#ifdef SM_RESTORE_TRACE
ofstream smStateRestore("smStateRestore.log");
#endif
extern  ofstream runTimeLog;

#define Branch_Predict 1 //  //������֧����Ԥ��--�ж���ٷ�֧�˿�����ͼ�ν�����ƣ������ﱣ��Ϊ1
#define xulei_dataflow_output 0
#define Traverse_Advanced_Output 0//�������ʱ��һ��Ҫ�ص�
#define  Traverse_Advance 1      //����Ԥ������1���ر���0 

#define Node_Trace 1   //smy   //��ӡ����ڵ�
//����ģʽ�µĿ���
#define Debug_Mode  1 //����ģʽ�Ŀ���
#define MaxLine 0  //���������Ϣ���Ͻ�
#define MinLine 0    //���������Ϣ���½�
#define SpecialLine 0//�������һ�е���Ϣ

#define Memory_Debug 0; //����ʱ����ڴ�ģ������bdd��Ϣ
#define Taint_Debug 0; //����ʱ�����Ⱦ����ģ������bdd��Ϣ
#define leak_resource 0;//��Դй©�ĵ��Կ��أ��������·������ʱ��Bdd��Ϣ

#define debugSpecialFunction 1  //�����ض������Ŀ���
bool PrintSpecialFunctionFlag = false;  //���Դ�Χ����ʱ����ӡ���⺯��ָ��ѿռ�BDD���ϵı�־

#define functionNeedTodebug "" //Ҫ���Եĺ���������

//����ģʽ�µĿ���--end
int g_cueernt_visited_line = 0;//��¼��ǰ���ʽڵ���к�
bool isReachSink = false;  //�Ƿ񵽴�sink��
extern bool g_project_use_condition_check; //�Ƿ�����֧�ж�  add 2016-11-16
extern int  g_project_loop_check_count;    //ѭ��ɨ�����         add 2016-11-16
extern bool g_project_quit_on_sink;       ////Sink���˿���"     add 2016-11-18
//#define ANALYZE_OPT_TRACE

//add analysize pattern shot

 long long  FunctionCallCount=1;          //����̴���
 long long  FunctionCallPattternShotCount =0;  //�����ģʽƥ�����

 long long  BranchVexNodeCount=1;             //��ֱ�ӵ����
 long long  BranchVexNodePatternShotCount =0; //��֧�ڵ�ģʽƥ�����
//add analysize pattern shot edd



//ZLY, BEGIN, 2010-9-1,��·������������Ż�
#define PATH_OPT_NO_OPT				0	//�������Ż�
#define PATH_OPT_ONLY_COVER			1	//����·�����Ƿ񳬹���ֵ�����������п�������㼴�ɣ���֧���Ҫ����true��֧��false��֧��
#define PATH_OPT_ONLY_COUNT			2	//��·����������ֵʱ��ֱ�ӽ���������·����
#define PATH_OPT_COVER_AND_COUNT	3	//��·����������ֵʱ�����������п�������㼴�ɣ���֧���Ҫ����true��֧��false��֧��
int giOptimalPaths = PATH_OPT_COVER_AND_COUNT;//�Ƿ�����Ż�
int gi_PATH_COUNT_OPT_THRESHOLD = 200; //·������ֵ
#define MAX_PATH_COUNT				50000 //��˴�������·����
#define CIRCLE_VISITED_MAX_NUM 1           //
//ZLY, END, 2010-9-1,��·������������Ż�

#define FUTENG_DEBUG
//#define CCDebug // by kong
#define TEMPVERSION
#define ASSISTANT

using namespace std;

//pattern map begin
map<VexNode*,map<int,bool>* > gLoopVexPatterns;  //��¼ѭ���ڵ��ƥ��ģʽ
map<SymbolItem*,map<int,bdd>* > gFunctionSensitivePattern;
map<SymbolItem*,map<int,bdd>* > gFunctionTaintPattern;
map<SymbolItem*,map<int,bdd>* > gFunctionPAPattern;
map<SymbolItem*,map<int,bdd>* > gFunctionPAPatternVHeapChange;//xxy
// pattern map end
ofstream dataflow("dataflow.log");
//by wangzhiqiang
extern ofstream wang;

//module_function_check_flag  --start
extern bool memory_function_check_on;
extern bool danger_function_check_on;
extern bool sensitive_function_check_on;
extern bool taint_function_check_on;
//module_function_check_flag  --end

PATreeWalker * TraversePaTreeWalker =NULL ;  //��ΪԤ������Ҫ�õ�PATreeWalker�еķ�������һ��ָ�룬��PATreeWalker���������ʹ�á�


ofstream paTrace;//("paTrace.log");
ofstream debug("debug.log",ofstream::trunc);

ofstream luxiao("luxiao.log",ofstream::trunc); //luxiao

 ofstream function_call("function_call.log",ofstream::trunc);
extern ofstream sm_debug;
//#define PA_TRACE
#ifdef PA_TRACE
#define PA_TRACE_LITTLE
#else
#define PA_TRACE_LITTLE
#endif
ofstream xulei_dataflow("PA/xulei_datafolw.log");
//dzh
extern ProgramStateManager ProgramState;
extern ofstream DzhTest;
extern streambuf* pOld;
extern ofstream paError;
//dzh end
extern int futengfuteng;
//cxx
extern ofstream paprocess;
extern ofstream paCovertChannel;
extern ofstream paError;
extern ofstream padataflow;

extern ofstream xulei_process;

extern int pathBranchTravelCount;
#include <ctime>
//#define PAcout//cout!!!
//cxx end

//VexValueSet staticValuesOfClasses;
//VexValueSet staticValuesOfFunctions;
VexValueSet staticValues;
IntegerValueSet returnValues;
VexValueSet exitValues;
CallStack DataFlowAnalyzer::m_callStack;
list<int> NodeList;        //�ڵ�������ŵ�ǰ����·���ϵĽڵ㬴

//2010-9-16,ZLY,BEGIN,�������ջ�����к����ķ���·���ϵĽڵ�
list<int>* gp_CurPath;//��ǰ������·��ָ��
//2010-9-16,ZLY,END,�������ջ�����к����ķ���·���ϵĽڵ�

SymbolItem* currFuncItem=0;         //ȫ�ֱ�������¼��ǰ�����ĺ�����Ŀ


//���� 2009-6-17
extern SymRoot* root;//!0327 added by dengfan 
extern unordered_map<int ,SymbolItem*> VariableMap;
//extern list<SymbolItem*> globalFunctions;//������������洢����AST�ĺ�����Ŀ wangyun 2008-11-18
extern FlowGraph flowGraph;
bool wangyunOk=false;

extern FuncMatrix funcMatrix;
map<SymbolItem*, CFGPaths> dataFlowPaths; // 20100106 by kong
extern list<PathBddMgr* > pathBddMgrList; // by kong 
extern InforFlowCollector* IFCollector;
extern map<int, int> TmpToReal; // 20100707 by kong 
extern list<ConstToReal> ConstToRealList; // 20100707 by kong
int countListSize = 0; // 20100707 by kong

extern ProcessBar processbar;    //���������ƶ���
int back_process_begin=0;
int back_process_end=0;

ofstream func("function.log");
//by wangzhiqiang
ofstream infoRuntime("infoTreeRuntime.log");
//end
ofstream futeng_path("futeng_path.log",ofstream::trunc);
#define futeng_path_output 0
int giCompatibleCount = 0;//���ڷ���Ч�ʷ���ʱͳ�Ʊ��η����ļ��ݺ�������
int giHistoryPathVexCount = 0;//���ڷ���Ч�ʷ���ʱͳ�Ʊ��η������÷�����·���ڵ���

extern ProcessBar processbar_Cur;

bool gb_OnlyOneEntry = false;//���ڱ���Ƿ�ֻ��һ����ں�����������ʾ����ϸ�Ľ�����Ϣ
long gl_TotalEntryNodes = 0;//��gb_OnlyOneEntryΪtrueʱ����¼��ں����е����н����
long gl_CurrentAnalyzedNodes = 0;

void closeConsole();
void openConsole();

int getAllNodesNum(CFGPaths& paths)  //�õ����д������ڵ����
{
	  cout<<"��������"<<endl;
	  //���������Ҫ�����Ľڵ����
	  int all_nodes_num=0;    //Ҫ�����Ľڵ��ܸ���
	  int pathindex=0;       //·���±�
	  Path *curPathPtr=0;    //��ǰ·��
	  Path *prePathPtr=0;    //ǰһ��·��
	  while(curPathPtr = paths[pathindex])  //��ǰ·��
	  {
			int vexindex=0;
			if(prePathPtr) vexindex = curPathPtr->firstDifferentVex(*prePathPtr);
			if(vexindex>0) vexindex--;   //�õ��ֲ�ڵ�

			while((*curPathPtr)[vexindex])
			{
				  //cout<<(*curPathPtr)[vexindex]->getLocalIndex()<<endl;
				  ++vexindex;
				  ++all_nodes_num;
			}

			prePathPtr = curPathPtr;

			++pathindex;   //��һ��·��
	  }

	  return all_nodes_num;
}


//xulei 20100414
RefAST ReConstructAST(RefAST ast, CPPParser* p_parser)      //�ع�AST����eת��Ϊ!e
{
	  if(!p_parser)
			return ast;
	  RefAST temp_Expression=RefAST(ANTLR_USE_NAMESPACE(antlr)nullAST);	//����Expression�ڵ�	

	  temp_Expression=p_parser->getASTFactory()->create(STDCTokenTypes::Expression,"Expression");

	  RefAST temp_unaryoperator=RefAST(ANTLR_USE_NAMESPACE(antlr)nullAST);	//����unaryoperator�ڵ�	
	  temp_unaryoperator=p_parser->getASTFactory()->create(STDCTokenTypes::UnaryOperator,"UnaryOperator");

	  RefAST temp_not=RefAST(ANTLR_USE_NAMESPACE(antlr)nullAST);	 //����not�ڵ�			
	  temp_not=p_parser->getASTFactory()->create(STDCTokenTypes::NOT,"!");

	  ANTLR_USE_NAMESPACE(antlr)RefAST(p_parser->getASTFactory()->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(temp_not)->add(ast)));
	  ANTLR_USE_NAMESPACE(antlr)RefAST(p_parser->getASTFactory()->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(temp_unaryoperator)->add(temp_not)));
	  ANTLR_USE_NAMESPACE(antlr)RefAST(p_parser->getASTFactory()->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(temp_Expression)->add(temp_unaryoperator)));

	  return temp_Expression;         //����ȡ��    
}

//�������������໥���ã�����AST�ĵȼ�ת����������Ч�������ĸ��Ӷȡ�
void visitAST(RefAST ast, CPPParser* p_parser);

void EquivalentConversion(RefAST ast, CPPParser* p_parser)  //��AST����һ�εȼ۱任������!!e <=> e�����
{
	  if (!p_parser)
	  {
			return;
	  }
	  RefAST root=ast->getFirstChild();

	  if(root && root->getType() == ASTInsTreeWalkerTokenTypes::UnaryOperator)    //���ڵ�ΪUnaryOperator�ڵ�
	  {   
			RefAST child=root->getFirstChild();   //���ӽڵ�

			if (child && child->getType()==ASTInsTreeWalkerTokenTypes::NOT)   //������ӽڵ�ΪNOT�ڵ�
			{
				  if (child->getFirstChild() && child->getFirstChild()->getType()==ASTInsTreeWalkerTokenTypes::Expression)  //������ӽڵ����һ���ڵ�ΪExpression�ڵ�
				  {
						child=child->getFirstChild();
				  }

				  RefAST grandchild=child->getFirstChild();   //���ӽڵ�
				  if (grandchild && grandchild->getType()==ASTInsTreeWalkerTokenTypes::UnaryOperator)
				  {
						RefAST grandgrandchild=grandchild->getFirstChild();   //���ӵĶ��ӽڵ�
						if (grandgrandchild && grandgrandchild->getType()==ASTInsTreeWalkerTokenTypes::NOT)
						{
							  //��ɵȼ�ת��
							  ANTLR_USE_NAMESPACE(antlr)RefAST(p_parser->getASTFactory()->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ast)->add(grandgrandchild->getFirstChild())));
							  //���ڶԽṹ�����˸ı䣬������Ҫ���±����µ�AST
							  visitAST(ast, p_parser);
						}
				  }
			}
	  }

}

void visitAST(RefAST ast, CPPParser* p_parser)    //������ȣ��ݹ����ast��ÿ���ڵ�
{
	  if (ast)    //1 �ȴ����ڵ�
	  {
			EquivalentConversion(ast, p_parser);
	  }

	  RefAST child=ast->getFirstChild();         //�õ���һ�����ӽڵ�
	  if (child)           //������ӽڵ㲻�գ������亢�ӽڵ�
	  {
			visitAST(child, p_parser);       //2 ���ȱ�����һ�����ӽڵ�
			RefAST brother=child->getNextSibling();  //�õ����ֵܽڵ�
			while(brother)     //3 ����ʣ��ĺ��ӽڵ�
			{
				  visitAST(brother, p_parser);
				  brother=brother->getNextSibling();
			}
	  }
}


bool isTrueEdge(VexNode* cur, VexNode * nextnode)
{
	  cout<<"enter function isTrueEdge"<<endl;
	  ArcBox * childEdge = cur->getFirstOutArc();
	  VexNode *childVexNode = childEdge->getTailVex();
	  CFGEdgeType edgeType = childEdge->getEdgeType();
	  if(nextnode ==childVexNode)
	  {
			return !(edgeType -1);
	  }
	  else
	  {
			return edgeType -1;
	  }
}

void resetCurrentAnalysis(PATreeWalker& pawalker, VexValueSet& globalValues){
	//2016-08-18,ZLY,����ڴ�һ�£��ȼ���ȫ�ֱ���ֵ���ټ���ȫ��ָ����Ϣ
	  pawalker.resetCurrentAnalysis();
	  calculateGlobalVariableValues(&globalValues);
	  genGlobelPointto(pawalker);
	  //�����¼��ģʽ
	  gFunctionPAPattern.clear();
	  gFunctionSensitivePattern.clear();
	  gFunctionTaintPattern.clear();
	  gLoopVexPatterns.clear();
	  gFunctionPAPatternVHeapChange.clear();
	  //2016-09-11  smy-add ���newLocation�洢�ĺ�������λ����Ϣ�������������ں������Ӱ��
	  pawalker.newLocation.clear();
	  pawalker.stringMap.clear();
	  //smy-end
}
/*
���ã�������ں����Ĳ���������Ϊָ��Ĳ���
������  pawalker:ָ�����walker����Ҫ��pawalker.mypa�е���Ӧbdd������Ϣ
bddIt��������Ӧ��bdd
type�����������͵ķ�����Ŀ

by futeng 2015-08-26
*/
void processFunctionPara(PATreeWalker &pawalker,const vbdd &explist,int recursiveDepth);
void typeIsPointer(Type *type,bdd bddIt,PATreeWalker& pawalker,int recursiveDepth)
{
	  //int paraID=symbolID;
	  bdd paraBdd=bddIt;
	  bool isPointer=true;
	  while(isPointer)
	  {
			Type *t=type->getElementType();
			ItemKind k=t->getItemKind();
			int n=fdd_scanvar(paraBdd,F1);
			if(n>0)
			{
				  pawalker.mypa.f_point|=fdd_ithvar(F1,n);
			}else
			{
				  pawalker.mypa.v_point|=paraBdd;
			}
			if(TYPE_POINTER==k)//��һ����Ȼ�Ǹ�ָ��
			{
				  pawalker.mypa.pointto|=paraBdd&fdd_ithvar(V2,++(pawalker.curvarnum))&fdd_ithvar(F2,0);//����һ�����������Ϊ��ǰָ�����һ��ָ��
				  paraBdd=fdd_ithvar(V1,pawalker.curvarnum)&fdd_ithvar(F1,0);
				  pawalker.heap_type[pawalker.curvarnum] = "Entry_Function_ParaBDD";               
				  pawalker.mypa.v_heap |= fdd_ithvar(V2,pawalker.curvarnum) & fdd_ithvar(F2,0);     //�����������ʱ���������ѿռ�
 				  type=t;
			}else if(TYPE_STRUCT==k){//�ñ�����ʵ��һ��ָ��ṹ�����͵�ָ�룬��Ҫ����ṹ���ڵĳ�Ա
				  ScopeSymbolTable* sc=static_cast<ScopeSymbolTable*>( t->getPointtoSymTable());
				  if(sc==NULL)
				  {
						return;
				  }
				  //����һ���ṹ�����--   ���ڵ���778297sssssss
				  pawalker.mypa.v_struct|=fdd_ithvar(V1,++(pawalker.curvarnum));
				  pawalker.mypa.pointto|=paraBdd&fdd_ithvar(V2,pawalker.curvarnum)&fdd_ithvar(F2,0);
				  pawalker.heap_type[pawalker.curvarnum] = "Entry_Function_ParaBDD";
				  pawalker.mypa.v_heap |= fdd_ithvar(V2,pawalker.curvarnum) & fdd_ithvar(F2,0);
				  //����ṹ���ڵĳ�Ա
				  list<SymbolItem*> memberList=sc->getClassAttributeList();
				  list<SymbolItem*>::iterator memberListIt=memberList.begin();
				  vbdd explistPara;
				  paraBdd=fdd_ithvar(V1,pawalker.curvarnum);
				  while(memberListIt!=memberList.end())
				  {
						bdd tmp=paraBdd&fdd_ithvar(F1,(*memberListIt)->getSymbolID());
						explistPara.push_back(tmp);
						memberListIt++;
				  }
				  //�ѽṹ���Ա�ݹ鴦��
				  processFunctionPara(pawalker,explistPara,recursiveDepth+1);
				  isPointer=false;
			}else if(TYPE_FUNC==k){//�Ǻ���ָ��
				  isPointer=false;
			}else{
				  pawalker.mypa.v_heap|=fdd_ithvar(V2,++(pawalker.curvarnum))&fdd_ithvar(F2,0);
				  //heapsize??
				  int iHeapCode = pawalker.curvarnum;  //add 2016-10-09
				  pawalker.heap_type[iHeapCode] = "Entry_Function_ParaBDD";   //smy ������ں�������BDD���������� 
				  pawalker.mypa.pointto|=paraBdd&fdd_ithvar(V2,(pawalker.curvarnum))&fdd_ithvar(F2,0);
				  isPointer=false;
			}			

	  }
}
/*
���ã�������ں����Ĳ�������Ҫ�ǽṹ�塢ָ�롢�������ͣ�����������ָ����Ϣ��������Ӧ��bdd��
������  pawalker:ָ�����walker����Ҫ��pawalker.mypa�е���Ӧbdd������Ϣ
explist:�βε�bdd�б�

by futeng 2015-08-26
*/
void processFunctionPara(PATreeWalker &pawalker,const vbdd &explist,int recursiveDepth)
{
	  if(recursiveDepth > ConfigInfo_Cpp::getMaxRecursiveDepth4EntryPara())
	  {
			return;
	  }
	  vbdd::const_iterator bddIt=explist.begin();
	  int v1,f1,symbolID;
	  while(bddIt!=explist.end())
	  {
			//1��ͨ��bdd�ҵ����ŵ�ID
			v1=fdd_scanvar(*bddIt,V1);
			f1=fdd_scanvar(*bddIt,F1);//���F1����ֵ��˵�����ڽṹ����
			if(f1>0) 
			{
				  symbolID=f1;
			}else{
				  symbolID=v1;
			}
			//2��ͨ��VariableMap�ҵ��βζ�Ӧ��SymbolItem
			unordered_map<int,SymbolItem*>::iterator sblIt=VariableMap.find(symbolID);
			if(VariableMap.end()==sblIt)//�Ҳ���������ȥ
			{
				  continue;
			}
			SymbolItem* sbl=sblIt->second;
			//3���жϲ��������Ͳ�����
			Type *type=sbl->getTypeItem();//�ҵ��βε����͵ķ�����Ŀ
			ItemKind kind=type->getItemKind();
			if(TYPE_STRUCT==kind)//�ǽṹ��
			{
				  if(f1>0)//����ṹ����Ƕ�ṹ���Ա�����
				  {
						pawalker.mypa.f_struct|=fdd_ithvar(F1,f1);//��¼����
						//�������۵�
						pawalker.mypa.fieldOpe|=(*bddIt)&fdd_ithvar(V2,++(pawalker.curvarnum));
						symbolID=pawalker.curvarnum;
				  }
				  //����ṹ���ڵĳ�Ա
				  Type *structType=sbl->getTypeItem();
				  ScopeSymbolTable* sc=static_cast<ScopeSymbolTable*>( structType->getPointtoSymTable());
				  if(sc==NULL)
				  {
					  bddIt++;
					  continue;
				  }
				  list<SymbolItem*> memberList=sc->getClassAttributeList();
				  list<SymbolItem*>::iterator memberListIt=memberList.begin();
				  vbdd explistPara;
				  while(memberListIt!=memberList.end())
				  {
						bdd tmp=fdd_ithvar(V1,symbolID)&fdd_ithvar(F1,(*memberListIt)->getSymbolID());
						explistPara.push_back(tmp);
						memberListIt++;
				  }
				  //�ѽṹ���Ա�ݹ鴦��
				  processFunctionPara(pawalker,explistPara,recursiveDepth+1);
			}
			else if(TYPE_POINTER==kind)//���������ָ��
			{
				  typeIsPointer(type,*bddIt,pawalker,recursiveDepth);
			}else if(TYPE_ARRAY==kind)//�������������
			{
				  pawalker.mypa.pointto|=(*bddIt)&fdd_ithvar(V2,++(pawalker.curvarnum))&fdd_ithvar(F2,0);
				  int n=fdd_scanvar(*bddIt,F1);
				  if(n>0)
				  {
						pawalker.mypa.v_array|=fdd_ithvar(V1,v1);//��v_array�Ƚϣ�����Ϊ֮ǰ�����������ж�������ȫ���浽��v_array��
				  }else{
						pawalker.mypa.v_array|=fdd_ithvar(V1,v1);
				  }

				  pawalker.mypa.v_array|=fdd_ithvar(V1,pawalker.curvarnum);
			}
			bddIt++;
	  }
}
extern map<string,set<int> > sensitiveMap;
extern map<string,set<int> > tainedTraceMap;

/*******************************************************************
* �������ƣ�switchTaintedSensitive
* ��    �ܣ�����������Ϣ����ȾԴ��������Ϣ
* �� �� ֵ��
* ��    ע��������Ϣ����ȾԴ����һ�״��룬����һ�ڵ�ʱ��������Ϣ�Ƿ���sensitiveMap�У�
�����ڽ�����Ⱦ��Ϣ���ʱ��Ϊ�˸���������Ϣ�Ĵ��룬��Ҫ��taintedTraceMap
�е����ݸ��Ƶ�sensitiveMap�У���ģ�������Ժ��ٻ�����
* �ϴ��޸ģ�
* �� �� �ˣ�futeng
* ��    �ڣ�2016.5.5 15:5
*******************************************************************/
void switchTaintedSensitive()
{
	  map<string,set<int> > temp;
	  temp = sensitiveMap;
	  sensitiveMap = tainedTraceMap;
	  tainedTraceMap = temp;
}

/*******************************************************************
* �������ƣ�printTraverseAdvancedResult
* ��    �ܣ���ӡԤ���������Ϣ
* �� �� ֵ��
* ��    ע
* �ϴ��޸ģ�
* �� �� �ˣ�shenmengyuan
* ��    �ڣ�2016-10-19
*******************************************************************/
void printTraverseAdvancedResult(PATreeWalker &pawalker)
{
	FunctionManager* pFuncManager = FunctionManager::functionManagerInstance();
	list<SymbolItem*>& sortedFunctionList = pFuncManager->getSortedFunctionList();   //��ȡ������ں���
	openConsole();
	cout<<"---------------Ԥ�������--------------"<<endl;
	closeConsole();

	//��ӡ������
	for(list<SymbolItem*>::iterator it = sortedFunctionList.begin();it!=sortedFunctionList.end();it++)
	{
		
		openConsole();
	
			if((*it)->getName()=="")
				continue;
		FunctionFlagSet * tempFlagSet = TraverseInfo::findModuleFunctionInformation[(*it)->getName()];
		if(tempFlagSet==NULL)
			return;
		cout<<"+++++++++++++++++++++++++++++��ǰ������"<<(*it)->getName()<<"����Ԥ�������++++++++++++++++++++++++++++++++++++"<<endl;
		cout<<"---------������ʶ������Ϣ----------"<<endl;
		cout<<"b_change_pointer:"<<tempFlagSet->b_change_pointer<<endl;
		cout<<"b_danger_sink:"<<tempFlagSet->b_danger_sink<<endl;
		cout<<"b_memory_sink:"<<tempFlagSet->b_memory_sink<<endl<<endl;

		cout<<"b_taint_change:"<<tempFlagSet->b_taint_change<<endl;
		cout<<"b_taint_sink:"<<tempFlagSet->b_taint_sink<<endl;
		cout<<"b_taint_source:"<<tempFlagSet->b_taint_source<<endl<<endl;


		cout<<"b_sensitive_change:"<<tempFlagSet->b_sensitive_change<<endl;
		cout<<"b_sensitive_source:"<<tempFlagSet->b_sensitive_source<<endl;
		cout<<"b_sensitive_sink:"<<tempFlagSet->b_sensitive_sink<<endl;
		cout<<"++++++++++++++++++++++++++++����++++++++++++++++++++++++++++++++++++"<<endl;
		
		closeConsole();
		CFG* currentCFG = (CFG*)(*it)->getCFG();  //�õ�������CFG
		if(currentCFG != NULL)
		{


			VexNode* pNode = currentCFG->getHead();//�õ�cfg����ڽڵ�
			while(pNode)
			{
				if(NULL !=pNode)
				{
					CFGNode* node = pNode->getData();
					if(NULL == node)
					{
						continue;
					}
					RefAST ast = node->getAST();
					if(NULL != ast)
					{
						int line = static_cast<RefMyAST>(ast)->getLineNum();
						openConsole();
#if 0
						cout<<"����кţ�"<<line<<" " <<static_cast<RefMyAST>(ast)->getText()<<
							"\t�Ƿ���Ҫ���ʣ�:"<<shouldNodeBeCheck(pawalker,pNode)<<endl;
#else
						cout<<"����кţ�"<<line<<" " <<static_cast<RefMyAST>(ast)->getText()<<endl;
						cout<<"---------������ʶ������Ϣ----------"<<endl;
/*						cout<<"is_function_need_visited:"<<pNode->is_function_need_visited<<endl;*/
						cout<<"b_change_pointer:"<<pNode->b_change_pointer<<endl;
						cout<<"b_danger_sink:"<<pNode->b_danger_sink<<endl;
						cout<<"b_memory_sink:"<<pNode->b_memory_sink<<endl<<endl;

						cout<<"b_taint_change:"<<pNode->b_taint_change<<endl;
						cout<<"b_taint_sink:"<<pNode->b_taint_sink<<endl;
						cout<<"b_taint_source:"<<pNode->b_taint_source<<endl<<endl;


						cout<<"b_sensitive_change:"<<pNode->b_sensitive_change<<endl;
						cout<<"b_sensitive_source:"<<pNode->b_sensitive_source<<endl;
						cout<<"b_sensitive_sink:"<<pNode->b_sensitive_sink<<endl<<endl;

						cout<<"is_entry_functionNode:"<<pNode->is_entry_functionNode<<endl;
// 						cout<<"bCheck:"<<pNode->getIsNeedCheck()<<endl;    //����ڵ�ר�ñ��
// 						cout<<"needVisited:"<<pNode->getNeedVisit()<<endl;  //��ں���ר�ñ��

						
						cout<<"shouldNodeBeCheck$$$$$$$$$$$:"<<"line:  "<<line<<"->>"<<shouldNodeBeCheck(pawalker,pNode)<<endl;
#endif
// 						if(pNode->traverseInfo!=NULL)
// 							;//cout<<pNode->traverseInfo->type<<endl;
// 						else
// 							cout<<"0--"<<endl;

						closeConsole();
					}
				}
				pNode = pNode->nextVexNode;
			}
		}
	}

}

void printCurrentNodeTraverseAdvancedResult(PATreeWalker &pawalker,VexNode* pNode)
{
	if(pNode==NULL)
		return ;
	CFGNode* node = pNode->getData();
	if(node==NULL)
		return;
	RefAST ast = node->getAST();
	if(ast==NULL)
		return;
	int line = static_cast<RefMyAST>(ast)->getLineNum();
	openConsole();

	cout<<"����кţ�"<<line<<" " <<static_cast<RefMyAST>(ast)->getText()<<endl;
	cout<<"---------������ʶ������Ϣ----------"<<endl;
	cout<<"b_change_pointer:"<<pNode->b_change_pointer<<endl;
	cout<<"b_danger_sink:"<<pNode->b_danger_sink<<endl;
	cout<<"b_memory_sink:"<<pNode->b_memory_sink<<endl<<endl;
	
	cout<<"b_taint_change:"<<pNode->b_taint_change<<endl;
	cout<<"b_taint_sink:"<<pNode->b_taint_sink<<endl;
	cout<<"b_taint_source:"<<pNode->b_taint_source<<endl<<endl;
	
	
	cout<<"b_sensitive_change:"<<pNode->b_sensitive_change<<endl;
	cout<<"b_sensitive_source:"<<pNode->b_sensitive_source<<endl;
	cout<<"b_sensitive_sink:"<<pNode->b_sensitive_sink<<endl<<endl;

	cout<<"is_entry_functionNode:"<<pNode->is_entry_functionNode<<endl;

	cout<<"shouldNodeBeCheck$$$$$$$$$$$:"<<"line:  "<<line<<"->>"<<shouldNodeBeCheck(pawalker,pNode)<<endl;
	closeConsole();


}
void dataFlowAnalysis(CPPParser* p_parser)
{
	  // 1. ���㲢����ȫ�ֱ���ֵ
	  VexValueSet globalValues;	// ��������ȫ�ֱ���������ֵ��Ϣ, ��Ϊÿ����������ʱ�������ֵ
	  calculateGlobalVariableValues(&globalValues);
	  //IFCollector = new InforFlowCollector();
	  //cxx
	  PATreeWalker pawalker(1);
	  genGlobelPointto(pawalker);
	  //cxx end

	  //��ӡ���еĺ���
	  /*sensitive_error<<"����������"<<g_FunctionList.size()<<endl;
	  for(list<SymbolItem*>::iterator it_func = g_FunctionList.begin(); it_func != g_FunctionList.end(); ++it_func)
	  {
	  sensitive_error<<(*it_func)->getName()<<endl;
	  }
	  */



	  // 20100508
	  //2010-10-9,ZLY,�����Ƿ���main,�����ɵ���ͼ��������������ں���
	  FunctionManager* pFuncManager = FunctionManager::functionManagerInstance();
	  list<SymbolItem*>& entrySbl = pFuncManager->getAllEntryFunc();
	  int total_func_numbers = entrySbl.size(); 
	  int curr_func_number=0;		

	  if(1 == total_func_numbers)
			gb_OnlyOneEntry = true;
	  bool bFirstFunc = true;

	  ModuleManager* pModuleManager = ModuleManager::moduleManagerInstance();//ģ�������

	  bool bSwitched = false;
	  TraverseTreeWalker traverseWalker;
	  int curvarnumBackUp = pawalker.curvarnum;
	  pModuleManager->openSM();//ִ���Զ���ģ��
	  int iTotalModules = pModuleManager->getTotalModuleCount();
	  int iModuleCount = 0;
	 // while(pModuleManager->nextModule() != ModuleManager::MODULE_FINISHED)//ÿһ�����ģ��
	//  {
// 			if(ModuleManager::MODULE_TAINTED == ModuleManager::runningType)  //�ⲿ����ʱ��������
// 			{
// 				  switchTaintedSensitive();//������������˵��
// 				  bSwitched = true;
// 			}


			SymbolItem* funcItem;
			pawalker.setAnalzingModuleType(ModuleManager::runningType);//���øôη�����ģ��
			curr_func_number = 0;
// 			openConsole();
// 			cout<<"���ڷ�����("<<pModuleManager->getAnalyzingModuleIndex()<<"/"<<pModuleManager->getTotalModuleCount()<<")��ģ�飺"<<pModuleManager->getModuleName()<<endl;
// 			closeConsole();
			/*ft ����Ԥ����*/
			time_t traverseBegin,traverseEnd;
			time(&traverseBegin);
			openConsole();
			cout<<"Ԥ������ʼ"<<endl;
			closeConsole();
			TraversePaTreeWalker  = &pawalker;  //��ȫ��Ψһ��pawalker��ֵ��ȫ�ֶ�����Ԥ������ʹ��
			traverseAdvanceEntry(traverseWalker,ModuleManager::ALL_MODULE);  //
			time(&traverseEnd);
			openConsole();
			cout<<"Ԥ������������ʱ:"<<(traverseEnd - traverseBegin)<<endl;
			closeConsole();
			runTimeLog<<"Ԥ��������ʱ:��" << (traverseEnd - traverseBegin) << " ��.\n";
			runTimeLog.flush();
#if Traverse_Advanced_Output
			printTraverseAdvancedResult(pawalker);
#endif
		//	exit(0);
			/*end*/
			list<SymbolItem*>& sortedFunctionList = pFuncManager->getSortedFunctionList();   //��ȡ��������ĺ���
			pawalker.generateFuncTempParaForAll(sortedFunctionList);
			for (list<SymbolItem*>::iterator it = entrySbl.begin(); it != entrySbl.end(); it++) //ÿһ����ں���
			{
				  funcItem =*it;//��ǰҪ��������ں���
				  if(bFirstFunc){
						bFirstFunc = false;
				  }else{
						resetCurrentAnalysis(pawalker, globalValues);
						pawalker.curvarnum = curvarnumBackUp;
						if (funcItem)
							  (((FuncSymbolItem*)(funcItem))->function_int_map).clear();
				  }
				  ++curr_func_number;


				  if(!(funcItem->getASTNode()))						// ����û�ж�Ӧ��AST, �ܿ���Ϊ������Ŀ
				  {			
						continue; 
				  }
				  add2FuncStack(pawalker,funcItem);

				  cout.rdbuf(pOld);
				  cout<<"���ڷ�����("<<curr_func_number<<"/"<<total_func_numbers<<")����ں�����"<<funcItem->getName()<<endl;
				  pOld = cout.rdbuf(DzhTest.rdbuf());	 
				  processbar_Cur.setCurrentAnalysisInfo(string("����:") + funcItem->getName(), 100, 50);

				  GlobalTool::clearAllPatterns();
				  ScopeSymbolTable* sc=static_cast<ScopeSymbolTable*>( funcItem->getPointtoSymTable());
				  list<SymbolItem*> parameterList=sc->getFuncParaList();
				  list<SymbolItem*>::iterator paraIt = parameterList.begin();
				  vbdd explist;
				  while(paraIt!=parameterList.end())
				  {
						//�����ж���ָ�롢���顢������������
						bdd tmp=fdd_ithvar(V1,(*paraIt)->getSymbolID())&fdd_ithvar(F1,0);
						explist.push_back(tmp);
						pawalker.mypa.v_funcEntryPara |=tmp;
						paraIt++;
				  }
				  processFunctionPara(pawalker,explist,1);  //Ϊ��ں�����ָ�����ͱ�������BDD

				  DataFlowAnalyzer analyzer(funcItem,pawalker,p_parser);	// Ϊ������Ŀ����������������
				  analyzer.setEntranceValues(globalValues);		// ���ô����������������ֵ
				  analyzer.analyze(false,1);	
				  processbar_Cur.setCurrentAnalysisInfo(string("���� ") + funcItem->getName(), 100, 100);
				  processbar.setBackOne(curr_func_number , total_func_numbers);
				  openConsole();
				  cout<<endl<<"��ǰ�������ô���:"<<FunctionCallCount<<"��"<<endl;
				  cout<<"��������ģʽƥ�����:"<<FunctionCallPattternShotCount<<"��"<<endl;
				  cout<<"��ǰɨ��ķ�ֱ�ӵ���:"<<BranchVexNodeCount<<"��"<<endl;
				  cout<<"��ǰ��֧�ڵ�ģʽƥ����:"<<BranchVexNodePatternShotCount<<"��"<<endl;

				  cout<<endl<<"��������ģʽƥ�������ʣ�"<< (FunctionCallPattternShotCount*1.0/FunctionCallCount)<<endl;
				  cout<<"��֧���ģʽƥ�������ʣ�"<<(BranchVexNodePatternShotCount*1.0/BranchVexNodeCount)<<endl;
				  closeConsole();
			}

// 			if(bSwitched)
// 			{
// 				  switchTaintedSensitive();
// 				  bSwitched = false;
// 			}
			//processbar.setBcck_OneFunction(curr_func_number,total_func_numbers);
			
			// �����������������Ժ�����Ŀ���з���
			//break;
			pModuleManager->closeSM();//�Զ���ģ��ִֻ��һ��
			++ iModuleCount;
	//  }

	  processbar.setBackEnd();

	  paTrace<<"���η����ĺ���������Ϊ:"<<giCompatibleCount<<endl;
	  paTrace<<"���η�����ʹ����ʷ�����Ϊ:"<<giHistoryPathVexCount<<endl;

	  //2010-08-12,ZLY,BEGIN,����Դй©���ŵ�ÿ��·��������ִ��
	  //outputFinalError(pawalker);      //20100508, xulei
	  //2010-08-12,ZLY,END

	  //list<SymbolItem*>::iterator begin=globalFunctions.begin(); // ��Ӧg_functionList
	  //list<SymbolItem*>::iterator end=globalFunctions.end();

	  cout.rdbuf(pOld);
	  cout<<endl<<"��˷���������"<<endl;
	  processbar_Cur.setCurrentAnalysisInfo(string("û�е�ǰ������Ϣ"),1);
	  //��˴������
	  /*processbar.setBackEnd();    
	  cout<<"���ڹ�����Ϣ��ͼ..."<<endl;
	  cout.rdbuf(DzhTest.rdbuf());	

	  int iFunctionCount = 0;
	  int iTotalFunctions = globalFunctions.size();

	  //��ÿ����������Ϣ����ӵ���Ϣ��ͼ�С�
	  for(;begin!=end;++begin)
	  {
	  BranchDepthMgr* branchDepthMgr = NULL; //new BranchDepthMgr(*begin);
	  //branchDepthMgr->genBranchDepth();
	  //branchDepthMgr->printNodeType();

	  PathBddMgr* pathBddMgr = new PathBddMgr(*begin);
	  pathBddMgr->genPathBdd();
	  pathBddMgr->printBdd(pathBddMgr->getBdd(),1);
	  //pathBddMgr->checkDeduce();
	  pathBddMgrList.push_back(pathBddMgr);

	  InformationFlow flow((FuncSymbolItem*)*begin);
	  // 		cout<<"------------------------------������Ϣ�����ɺ���--------------------------------"<<endl;

	  flow.ScanAFunc(pawalker, branchDepthMgr);
	  // 		cout << "ScanAFunc" << endl;
	  // 		flow.PutNodeNumber();
	  //���õ�����Ϣ����ӵ�ͼ�ϡ�
	  vector<Flow> temp=flow.GetGlobalInforFlow();
	  vector<Flow>::iterator begin=temp.begin();
	  vector<Flow>::iterator end=temp.end();
	  for(;begin!=end;++begin)
	  {
	  bool ifExist1=flowGraph.IfExistSameNode((*begin).GetOutVar());
	  FlowNode* node1;
	  FlowNode* node2;
	  if(ifExist1==true)
	  node1=flowGraph.GetSameNameNode((*begin).GetOutVar());
	  else
	  {
	  node1=new FlowNode((*begin).GetOutVar(), (*begin).getFileName(), (*begin).getLineNum());
	  flowGraph.AddFlowNode(node1);
	  }

	  bool ifExist2=flowGraph.IfExistSameNode((*begin).GetInVar());
	  if(ifExist2==true)
	  node2=flowGraph.GetSameNameNode((*begin).GetInVar());
	  else
	  {
	  node2=new FlowNode((*begin).GetInVar(), (*begin).getFileName(), (*begin).getLineNum());
	  flowGraph.AddFlowNode(node2);
	  }

	  //�������ӵ�����������߻��߳�����
	  FlowArc* arc=new FlowArc(node1,node2,(*begin).GetFuncName(), (*begin).getFileName(), (*begin).getLineNum()); 
	  //�鿴�ñ��Ƿ��Ѿ���ͼ�г���
	  bool ifExist3=flowGraph.IFArcExist(arc);
	  if(ifExist3==false)
	  {
	  //�޸�ÿ����������߼��Ϻͳ��߼���
	  node1->AddInArc(arc);
	  node2->AddOutArc(arc);
	  flowGraph.AddFlowArc(arc);
	  }
	  // 			cout << "flowGraph.deleteNull()" << endl;
	  //flowGraph.deleteNull();
	  //cout<<"���ڵ㣺"<<(*begin).GetOutVar()<<"����ڵ㣺"<<(*begin).GetInVar()<<"��������: "<<(*begin).GetFuncName()<<endl;
	  }
	  //flow.printFlows();
	  iFunctionCount++;
	  processbar.setCCEnd_stage(0.5*((float)iFunctionCount/iTotalFunctions));
	  }
	  cout.rdbuf(pOld);
	  cout<<"������Ϣ��ͼ����"<<endl;*/
	  cout.rdbuf(DzhTest.rdbuf());	
}


/************************************************************************/
/* ��DataFlowAnalyzer��������                                           */
/************************************************************************/
DataFlowAnalyzer::DataFlowAnalyzer(SymbolItem* f,PATreeWalker& p,CPPParser* p_p):pawalker(p),p_parser(p_p)
{	
	  bTainedSinkFlag4VexNode = false;	//��������������Ƿ񴥷���tained sink�ı��
	  bTainedSinkFlag4Func = false;		//��ǰ�����Ƿ񴥷���tained sink�ı��

	  m_treeWalker = new ExprValueTreeWalker;		// ����AST������
	  //by wangzhiqiang
	  //infoTreeWalker= new InfoTreeWalker(&p);

	  m_treeWalker->setDataFlowAnalyzer(this);	// ���ñ�����
	  m_treeWalker->setFuncInfoCollect(false);
	  m_func = f;									// ����������
	  analyzeNodeIndex=0;
	  beingAnalysisPathIndex=1;
	  theRestPathSize=1;
	  if(m_func)
	  {
			CFG* cfg = (CFG*)(m_func->getCFG());
			if(!cfg) return;
			cfg->getEnd()->inValues.clear();
			cfg->getEnd()->outValues.clear();
	  }
	  pathBranchCount=0;
}

DataFlowAnalyzer::~DataFlowAnalyzer()
{
	  delete m_treeWalker;
}

//2010-08-23,ZLY,��������������Ϊ�������ܵ�����������·������ķ�֧����ֳ���ʹ���Զ����ķ�������ȷ
//������������ָ��ָ����Ϣ��˵Ӧ������ȷ��
//Ӧ��ͨ����·���ĺ�������ﵽ��߷���Ч�ʵ�Ŀ��
/*
//2010-08-22,ZLY,BEGIN,��ǰ������ͬ���������һ��·����ʹ�ø�·������ʷ��Ϣ
void DataFlowAnalyzer::GetMaxSameSubPath(CFGPaths& paths, int iCurPathIndex, int& iFirstDiffVex, int& iPathIndex)
{
Path * curPathPtr, *prePathPtr;
iFirstDiffVex = 0;
iPathIndex = 0;
if(iCurPathIndex < 0 || iCurPathIndex >= paths.size())
return;
curPathPtr = paths[iCurPathIndex];
if(curPathPtr == NULL)
return;
int iPreIndex = iCurPathIndex - 1;
while(iPreIndex >= 0){
prePathPtr = paths[iPreIndex];
if(prePathPtr == NULL)
return;
int iTmp = curPathPtr->firstDifferentVex(*prePathPtr);
//��ǰ·�������ӷ��ص�iFirstDifVex-1��ʼ
//���ص�iFirstDifVex-2��Ҫȡ��ʷ�ķ���������
while(iTmp > iFirstDiffVex && iTmp >= 2 && prePathPtr->getAnalyzedFlag(iTmp-2) != Analyzed){
iTmp --;
}
if(iTmp >= 2 && iTmp > iFirstDiffVex){
iFirstDiffVex = iTmp;
iPathIndex = iPreIndex;
}
--iPreIndex;
}
if(iFirstDiffVex == 0){
iFirstDiffVex = 2;
iPathIndex = 0;

}

}
//2010-08-22,ZLY,END,��ǰ������ͬ���������һ��·����ʹ�ø�·������ʷ��Ϣ
*/
void DataFlowAnalyzer::analyze(int isInterprocedural,int deep)
{
	  if(!needAnalysis())	//��ǰ������Ŀ����ջ��
	  {
			return;
	  }
	  pushCallStack();// ����ǰ������Ŀѹ�뺯������ջ
	  if(m_func)
	  {
			if(isInterprocedural)
			{
				  int callNum=m_callStack.getStackSize();
				  for(int i=0;i<callNum;i++)
						function_call<<"\t";
				  function_call<<"�����ú�����"<<m_func->getName()<<"("<<m_func->getFileLineNum()<<")"<<endl;
			}
			else
			{
				  function_call<<"��ں�����"<<m_func->getName()<<"("<<m_func->getFileLineNum()<<")"<<endl;
				  /*	
				  openConsole();
				  cout<<"��ں�����"<<m_func->getName()<<"("<<m_func->getFileLineNum()<<")"<<endl;
				  closeConsole();
				  */
			}

	  }	
	  currFuncItem=m_callStack.top();
	  //IFCollector->CCAFunc = bddfalse;
	  pawalker.setAnalyzer(this);
	  CFG *cfg=(CFG *)m_func->getCFG();
	  if(!cfg)
	  {
			function_call<<"����"<<m_func->getName()<<" cfgΪ��"<<endl;
			popCallStack();
			return;
	  }
#if debugSpecialFunction
	  if (m_func->getName()==functionNeedTodebug)
	  {
		  PrintSpecialFunctionFlag =true;  //����Ϊ�˵��Դ�Χ����ʱ�����ٴ�ӡ��׼��Ϣ�ı�־
	  }
#endif
	  analyzeNode(cfg->getHead(),NULL,NULL,isInterprocedural);
	  getParentOutInfor_ALL(NULL,pawalker);   //����������·������Ϣ����
	  popCallStack();
}

State* FindTainedState(_SM* sm)
{
	  State* tainedState = NULL;
	  sm_debug<<"Finding tained state in sm '"<<sm->getName()<<"'"<<endl;
	  for(map<SM_State*,map<Pattern*,SM_State*>* >::iterator itTrans=sm->TransitionTable.begin(); itTrans!=sm->TransitionTable.end(); itTrans++)
	  {
			SM_State* sm_state = (*itTrans).first;
			if(sm_state->getStateType() == CONDITION)
			{
				  sm_debug<<"static_cast<ConditionState*>(sm_state)->getTrueState()->getliteral():"<<static_cast<ConditionState*>(sm_state)->getTrueState()->getliteral()<<endl;
				  if(static_cast<ConditionState*>(sm_state)->getTrueState()->getliteral() == "tained")
				  {
						tainedState = static_cast<ConditionState*>(sm_state)->getTrueState();
						break;
				  }
				  sm_debug<<"static_cast<ConditionState*>(sm_state)->getFalseState()->getliteral():"<<static_cast<ConditionState*>(sm_state)->getFalseState()->getliteral()<<endl;
				  if(static_cast<ConditionState*>(sm_state)->getFalseState()->getliteral() == "tained")
				  {
						tainedState = static_cast<ConditionState*>(sm_state)->getFalseState();
						break;
				  }
			}else if(sm_state->getStateType() == NORMAL)
			{
				  sm_debug<<"static_cast<NormalState*>(sm_state)->getState()->getliteral():"<<static_cast<NormalState*>(sm_state)->getState()->getliteral()<<endl;
				  if(static_cast<NormalState*>(sm_state)->getState()->getliteral() == "tained")
				  {
						tainedState = static_cast<NormalState*>(sm_state)->getState();
						break;
				  }
			}
	  }
	  if(tainedState != NULL)
			sm_debug<<"Found!"<<endl;
	  else
			sm_debug<<"NOT Found!"<<endl;
	  return tainedState;
}
void DataFlowAnalyzer::MarkTainedVar(VexNode *curNode)
{
	  //����ÿ����Ⱦ������ɨ�������Զ���������Щ�Զ���ʹ����tained״̬
	  //�����ж���Ⱦ�����Ƿ���ProgramState��Symbol_list�У��粻�����������
	  //���Ҫ��֤��Ⱦ������״̬��tained
	  //ProgramState
	  /*
	  ����־ǿд��infoTreeWalker.g����Ŀ��ɾ���ˣ������tainedInfo����������.g�ж���ı������������һ���ֲ�����
	  ʹ����ͨ����
	  */
	  bdd tainedInfo = bddfalse;
	  if(tainedInfo==bddfalse)
	  {
			return;
	  }
	  bdd tainedInfoCopy=tainedInfo;
	  bdd tainedInfoOne=pawalker.bddsatone(tainedInfoCopy);
	  while(bddfalse!=tainedInfoOne)
	  {
			sm_debug<<"tainedInfoOne"<<fddset<<tainedInfoOne<<endl;
			int v1,f1;
			SymbolItem *sbl=NULL;
			unordered_map<int ,SymbolItem*>::iterator it;
			v1=fdd_scanvar(tainedInfoOne,V1);
			f1=fdd_scanvar(tainedInfoOne,F1);
			if(f1>0)
			{
				  it=VariableMap.find(f1);
				  if(it!=VariableMap.end())
				  {
						sbl=it->second;
				  }
			}else{
				  it=VariableMap.find(v1);
				  if(it!=VariableMap.end())
				  {
						sbl=it->second;
				  }
			}
			if(sbl==NULL)
			{
				  cout<<"�Զ�����ǣ�û���ҵ�������Ŀ"<<endl;
				  continue;
			}

			//1
			//�жϸ���Ŀ�Ƿ��Ѿ���״̬��û�оͼӽ�ȥ Symbol_list���������״̬�ķ�����Ŀ
			list<SymbolItem*>::iterator it2;
			for(it2=ProgramState.Symbol_list.begin();it2!=ProgramState.Symbol_list.end();it2++)
			{
				  if(*it2==sbl)
				  {
						break;//�ҵ�
				  }
			}
			if(it2==ProgramState.Symbol_list.end())
			{
				  ProgramState.Symbol_list.push_back(sbl);//����Ҳ������ͰѸ���Ŀ��ӽ�ȥ
			}
			//2
			list<AttachToSymbolItem*>* lstAttach = (list<AttachToSymbolItem*>*)(sbl->getSM_State());
			if(lstAttach != NULL)
			{
				  //Sm_list��ŵ��������Զ���
				  for(list<_SM*>::iterator it3=ProgramState.Sm_list.begin(); it3!=ProgramState.Sm_list.end(); it3++)
				  {
						//�����ҵ��Զ����е�tained״̬
						State* tainedState = FindTainedState((*it3));
						if(tainedState==NULL)
							  continue;

						bool bHasState = false;
						for(list<AttachToSymbolItem*>::iterator it4=lstAttach->begin(); it4!=lstAttach->end(); it4++)
						{
							  if((*it4)->Sm_list->find(*it3) != (*it4)->Sm_list->end())
							  {//�����ڸ��Զ�������״̬
									bHasState = true;
									//ȷ����״̬Ϊtained
									(*(*it4)->Sm_list)[(*it3)] = tainedState;
									break;
							  }
						}
						if(bHasState==false)
						{//�����Զ���״̬����һ��AttachToSymbolItem��
							  (*(lstAttach->begin()))->Sm_list->insert(attach_value((*it3), tainedState));
						}
				  }
			}else//�ñ�����״̬
			{//����AttachToSymbolItem����¼״̬
				  for(list<_SM*>::iterator it3=ProgramState.Sm_list.begin(); it3!=ProgramState.Sm_list.end(); it3++)
				  {
						//�����ҵ��Զ����е�tained״̬
						State* tainedState = FindTainedState((*it3));
						if(tainedState==NULL)
							  continue;
						if(lstAttach == NULL)
						{
							  lstAttach = new list<AttachToSymbolItem*>();
							  sbl->setSM_State(lstAttach);
							  /* �˴���ʱ����һ��ֻ��һ���MingleSymbolItem��ʵ����Ҫ����������MingleSymbolItem
							  * ʵ�ʴ�������������߼����ȸ���sbl����MingleSymbolItem��Ȼ����MingleSymbolItemȥƥ��list<AttachSymbolImte>�е�ÿһ��
							  */
							  AttachToSymbolItem * tmp = new AttachToSymbolItem();
							  tmp->Mingled_Item.addSubSblItem(sbl);
							  tmp->Sm_list->insert(attach_value(*it3,tainedState));
							  lstAttach->push_back(tmp);
							  sm_debug<<"״̬ת�ƣ�"<<endl;
						}
				  }
			}
			sm_debug<<"sbl:"<<sbl->getName()<<endl;
			tainedInfoCopy-=tainedInfoOne;
			tainedInfoOne=pawalker.bddsatone(tainedInfoCopy);
	  }

}
extern ofstream sensitive_error;
extern ofstream taint_error;
extern ofstream tained_error;

int getFileLine(VexNode* node){
	  CFGNode* data = node->getData();
	  if(NULL == data){
			return -1;
	  }
	  RefAST ast = data->getAST();
	  if(NULL == ast){
			return -2;
	  }
	  return static_cast<RefMyAST>(ast)->getLineNum();
}

/*******************************************************************
* �������ƣ�
* ��    �ܣ���ͷ��㿪ʼ�����η���һ��������CFG���
* ��    ����
* �� �� ֵ��
* ��    ע��
* �ϴ��޸ģ�
* �� �� �ˣ�
* ��    �ڣ�2016.8.30 10:36
*******************************************************************/
void DataFlowAnalyzer::analyzeNode(VexNode *curNode,VexNode *preNode,ArcBox *arc,bool isInterprocedural)
{
	/************************************************************************/
	/* int fun()
	{
	  int a = malloc();
	  if(....)//���������ģʽƥ��ɹ���ֱ�ӷ��أ�ָ��ָ����Ϣ�ᱻ��ʧ
	  free(a);
	
	
	}*/
	/************************************************************************/
	  if(curNode != NULL && false == isContinueRun(pawalker,curNode)) //�����ǰ�ڵ㲻Ϊ�գ������Ƿ�ֱ�ӵ㣬���ģʽƥ��ɹ�������������2016-11-2
	  {      
		    BranchVexNodePatternShotCount++;                              //ģʽƥ���ʵ�ʣ��ϴ��ߵ������֧����mypa.pointto���ָ����Ϣ������ߵ�����ڵ��mypa.pointto���ָ����Ϣ��ȫ��ͬ��ֻҪmypa.pointto��Rootֵ��ͬ����//����Ϊ��ģʽƥ�䣬����������
		   if( !shouldNodeBeCheck(pawalker,curNode))                   //ֱ�ӷ��ػᶪʧָ��ָ����Ϣ�����뱣֤����Ľڵ���ȫ�����ʣ��ſ�ֱ�ӷ���
		   {
#if Node_Trace
			   openConsole();
			   cout<<endl<<"�ڵ�ģʽƥ�䣺��ǰ�ڵ�["<<*curNode<<"]ģʽƥ���Һ���·���������,·����ֹ��";
#if leak_resource 
			   cout<<"line(��ʱ):"<<line<<endl;
			   cout<<"v_heap:"<<fddset<<pawalker.mypa.v_heap<<endl;
			   cout<<"pointto:"<<fddset<<pawalker.mypa.pointto<<endl;

#endif
			    closeConsole();
#endif
			   --theRestPathSize;                                      
			   recordOnePathInfo(pawalker);
			   return;

		   }
		  
	  }
	  int iPushNodeCount=0;
	  //static int pathCnt=1;//·��������
	  //static int pathSize=1;//�����ж�·���Ƿ�����
	  //static int vexIndex=0;//�ڵ�����,��1��ʼ����
	  bool bFinalPath=false;
	  bool isFirstNodeOnPath=false;   //��ǰ�ڵ��Ƿ�Ϊ��ǰ·���ĵ�һ���ڵ�
	  ArcBox* arcFalse=NULL;
	  ArcBox* arcTrue=NULL;
	  map<string, PaIntValue> backup_map;//�����жϽڵ��ʱ���������ݵ�map
	  ValueParameter integerOperationType;
	  int line;
	  while(curNode!=NULL)
	  {	
		  line = getFileLine(curNode);
#if Traverse_Advanced_Output 
		    
				printCurrentNodeTraverseAdvancedResult(pawalker,curNode); //��ӡ��һ�ε�Ԥ�������
#endif
			analyzeNodeIndex++;  //����ڵ�һ·���еı��
			nodestack.push(curNode);
			iPushNodeCount++;//��¼�������ѹջ�ڵ���

		//	openConsole();
		//	cout<<"line:"<<line<<" : "<<"pointto:"<<fddset<<pawalker.mypa.pointto<<endl;
		//	closeConsole();
#if 0
			20160530 ������������Ƶ�visitVex()��
				  //pawalker.findSensitiveSink=false;
				  bTainedSinkFlag4VexNode = false;
			pawalker.analyzingNodeType=curNode->getNodeType();
#endif
#if 0 
			20160530 ע�͵��ϰ汾�����η���
				  m_treeWalker->setVexIsCircleHead(false);
			m_treeWalker->walkAstOnVex(curNode,preNode);
#endif
#if Traverse_Advance
			//Ԥ����ģ���Ǹýڵ�֮�󶼲�����  
			if( !shouldNodeBeCheck(pawalker,curNode))
			{
#ifdef ANALYZE_OPT_TRACE
				openConsole();
				cout<<"advance tarverse mark"<<endl; 
				closeConsole();
#endif
				
#if  futeng_path_output
					futeng_path<<"һ��·��������ϣ���Ϣ�ռ�..."<<endl;
#endif

#if Node_Trace
					openConsole();
					cout<<endl<<"Ԥ��������ǰ�ڵ�["<<*curNode<<"]��������ڵ�������ʣ�·����ֹ��";
#if leak_resource 
					cout<<"line(��ʱ):"<<line<<endl;
					cout<<"v_heap:"<<fddset<<pawalker.mypa.v_heap<<endl;
					cout<<"pointto:"<<fddset<<pawalker.mypa.pointto<<endl;

#endif
					closeConsole();
#endif
					//2010-08-12,ZLY,BEGIN,���һ��·��������Ϣ���ܣ���ں�����·���ɴ�ʱ������Դй©���
					if(isInterprocedural == false){  //isInterprocedural:������㱻��Ϊfalse
						bool bFinalPath=false;
						if(1==theRestPathSize)
						{
							bFinalPath=true;
						}
						outputFinalErrorByVHeap(pawalker);
					}

					/*һ��·��������ϣ���¼����ָ����Ϣ�Ͷѿռ����Ϣ���Ըú�����id��Ϊ���*/
					recordOnePathInfo(pawalker);

					if(1 == theRestPathSize && isInterprocedural == false)
					{
#if futeng_path_output
						futeng_path<<"\t\t��ں������һ��·��������������Դ����..."<<endl;
#endif
						releaseLeakCheckResource(pawalker); //��Դ����
					}
				
					--theRestPathSize;
				nodestack.popMore(iPushNodeCount);//��ѹ��ջ�еġ�ֱ�ߡ��ڵ㴮����
				return;
			}   
#endif

#if Node_Trace  
			openConsole();                     //��ӡ���
			if(1==analyzeNodeIndex)//�׽ڵ�
			{
				isFirstNodeOnPath=true;
				futeng_path<<*curNode;
				cout<<*curNode<<flush;
			}else{
				futeng_path<<"->"<<*curNode;

				cout<<"->"<<*curNode<<flush;
			}

			// 			
			// 			cout<<"pointto:"<<fddset<<pawalker.mypa.pointto<<endl;
			// 			cout<<"heap:"<<fddset<<pawalker.mypa.v_heap<<endl;
			line = getFileLine(curNode);
			//cout<<"line:"<<line<<"	"<<fddset<<pawalker.mypa.pointto<<endl;
			closeConsole();
#endif


			if(2==curNode->getOutNum())//����Ƿ�֧�ڵ㣬����ѭ��
			{
				BranchVexNodeCount++;
				//++theRestPathSize;//������֧��㣬��ô��Ҫ������·����һ
				break;
			}
			visitVex(curNode,preNode,pawalker,beingAnalysisPathIndex,
				curNode->getFirstOutArc(),
				true,theRestPathSize,isInterprocedural,integerOperationType);
			openConsole();


			cout<<"line(���з��ʺ�):"<<line<<endl;
			cout<<"v_heap:"<<fddset<<pawalker.mypa.v_heap<<endl;
			cout<<"pointto:"<<fddset<<pawalker.mypa.pointto<<endl;



			closeConsole();
#if Node_Trace 
#if Debug_Mode
#if debugSpecialFunction
			if((curNode->theFunctionThisNodeBelongTo==functionNeedTodebug)&&((line>=MinLine && line<=MaxLine) || line ==SpecialLine) )
#else
			if((line>=MinLine && line<=MaxLine) || line ==SpecialLine )
#endif
			{


			}
#endif
#endif

#if 0
			//��������sink�������ڲ��ã������������Ϊ���˵���ں�����sink����
			if( 1 && getTainedSinkFlag4VexNode())//����һ��sink�㣬�ҷ��ִ��󣬸�·�����ټ�������ߣ�ֱ�ӷ���
			{
#ifdef ANALYZE_OPT_TRACE
				  openConsole();
				  cout<<"tained sink triggered, quit current path...."<<endl;
				  closeConsole();
#endif
				  --theRestPathSize;
				  recordOnePathInfo(pawalker);
				  nodestack.popMore(iPushNodeCount);
				  recoverSinkFlag4VexNode();//sink���ֻ�ڱ����һ��·���Ͻ����ضϣ���Ӱ��һ�������ڵ�����·��
				  return;
			}
#endif

			/*
			//�Զ���
			//�����ȾԴBEGIN
			MarkTainedVar(curNode);
			//�����ȾԴEND
			handleVexState(curNode,isFirstNodeOnPath,pawalker,m_treeWalker,false);
			travelVex(curNode,false,pawalker,m_treeWalker,false);
			//�Զ���end
			*/
			if(curNode->getLocalIndex()==1)//β��㣨ͷ�����0��β�����1��
			{
#if Node_Trace
				   openConsole();
				   cout<<"->"<<*curNode<<"[β�ڵ�]"<<endl;
#if leak_resource 
				   cout<<"line(��ʱ):"<<line<<endl;
				   cout<<"v_heap:"<<fddset<<pawalker.mypa.v_heap<<endl;
				   cout<<"pointto:"<<fddset<<pawalker.mypa.pointto<<endl;

#endif
				   closeConsole();
#endif
// 				  futeng_path<<"->"<<*curNode<<endl;
				  theRestPathSize--;
				  /*
				  if(theRestPathSize==0)//���һ��·�������һ���ڵ�����꣬����������ջ
				  {
				  popCallStack();
				  }
				  */
				  nodestack.popMore(iPushNodeCount);
				  return;
			}
			preNode=curNode;
			if(NULL == curNode->getFirstOutArc()){
				return;
			}
			curNode=curNode->getFirstOutArc()->getTailVex();
	  }

	  /*��ʼ�����֧�ڵ�*/
	  VexNodeType nodeType = curNode->getNodeType();
	  unsigned char backUpCircleNum = 0;
	  if(vCircle == nodeType || vDoWhile == nodeType){
			backUpCircleNum = nodestack.findNodeInStack(curNode);//��ǰcircle�ڵ㱻�����Ĵ���
	  }
	  if(++pathBranchCount > pathBranchTravelCount)
	  {
			openConsole();
			cout<<"function branch count greater than threshold "<<pathBranchTravelCount<<", quit current path...."<<endl;
			closeConsole();
			nodestack.popMore(iPushNodeCount);
			return;
	  }
	  //��֧
#if 0
	  20160601ע�ͣ�������ģʽ�ȶԣ��߼�������
			/*�ж�iPushNodeCount>1 ����Ϊ��������ĵ�һ������Ƿ�֧��㣬���ظ�ģʽƥ�����Σ��뺯����ڴ���ģʽƥ���ͻ��
			���ڶ��ο϶��ͻ������*/
			if((vIf == nodeType || vSwitch ==nodeType) //�жϷ�֧�ڵ���ߵ�һ�ν���ѭ���ڵ�֮ǰ����ƥ���ģʽ����ô��ֱ�ӷ�����
				  || ((vDoWhile == nodeType || vCircle == nodeType) && 1 == backUpCircleNum)
				  ){
						if(iPushNodeCount > 1 && false == isContinueRun(pawalker,curNode))//��ģʽ�Ѿ����ˣ�����
						{
							  nodestack.popMore(iPushNodeCount);
							  futeng_path<<"find same pattren,return."<<endl;
							  return;
						}
			}
#endif
			arcFalse=curNode->getFirstOutArc();
			if(NULL == arcFalse){
				return;
			}
			arcTrue=arcFalse->getNextEdgeWithSameHeadVex();
			ArcBox* arctemp;
			if(arcFalse->getEdgeType()!=eFalse)//���ȴ������֧
			{
				  arctemp=arcFalse;
				  arcFalse=arcTrue;
				  arcTrue=arctemp;
			}
#if 0
			switch(path)
			{
			case TruePath:
				  futeng_path<<"true";
				  break;
			case FalsePath:
				  futeng_path<<"false";
				  break;
			case BothPath:
				  futeng_path<<"both";
				  break;
			default:
				  futeng_path<<"error";
			}
#endif
			//VexNode* backUpBranchVex;
			bdd ccaPath,ccBranch,pointCOut,heapCOut;
			bdd backUpHeap; 
			bdd backUpPointto;
			bdd sensitiveBackUp;
			bdd taintBackUp;
			//unsigned char backUpCircleNum = curNode->circleNum;
			/*����ڵ���circle�ڵ㣬���Ѿ�ѭ��������������ô��false��֧����;����֧�ڵ�����������������ٷ�֧*/
			if(((vDoWhile == nodeType || vCircle == nodeType) && g_project_loop_check_count >= backUpCircleNum)
				|| (vIf == nodeType || vSwitch == nodeType)
				){
						/*����Ƿ�֧�ڵ���ߵ�һ�ν���ѭ���ڵ㣬��ô��Ҫ�ڷ���֮ǰ��������Ϣ�������������ɴ�ѭ������Ҫ���ݣ���false��֧ʱ�ָ�*/
						if(vIf == nodeType || vSwitch == nodeType ||
							  ((vCircle== nodeType || vDoWhile == nodeType) && 1 == backUpCircleNum)
							  ){
								    ++theRestPathSize;//������֧��㣬��ô��Ҫ������·����һ
									backUpHeap = pawalker.mypa.v_heap;
									backUpPointto = pawalker.mypa.pointto;
									sensitiveBackUp = pawalker.mypa.isSensitivedSource;
									taintBackUp = pawalker.mypa.isTaintedSource;
									//pointCOut=pawalker.mypa.pointtoCOut;
									//heapCOut=pawalker.mypa.v_heapCOut;
									backup_map = ((FuncSymbolItem*)DataFlowAnalyzer::getCallStack().top())->function_int_map;
									//xxy_backUp����
						}
						//backUpBranchVex=curNode;
						/*���֧*/
						integerOperationType.treeValue = TV_TRUE; 
						visitVex(curNode,preNode,pawalker,beingAnalysisPathIndex,
							curNode->getFirstOutArc(),
							true,theRestPathSize,isInterprocedural,integerOperationType);
						//if( 1 && pawalker.findSensitiveSink)//����һ��sink�㣬�ҷ��ִ��󣬸�·�����ټ�������ߣ�ֱ�ӷ���
						if( 1 && getTainedSinkFlag4VexNode())//����һ��sink�㣬�ҷ��ִ��󣬸�·�����ټ�������ߣ�ֱ�ӷ���
						{
							  openConsole();
							  cout<<"tained sink triggered, quit current path...."<<endl;
							  closeConsole();
							  --theRestPathSize;
							  recordOnePathInfo(pawalker);
							  nodestack.popMore(iPushNodeCount);
							  recoverSinkFlag4VexNode();//sink���ֻ�ڱ����һ��·���Ͻ����ضϣ���Ӱ��һ�������ڵ�����·��
							  return;
						}
#if Branch_Predict 
						if((!g_project_use_condition_check) || (TV_FALSE != integerOperationType.treeValue)){//Ԥ���true��֧���л�ȷ��������������ǰ·���ĺ����ڵ�
#else
						if(true||TV_FALSE != integerOperationType.treeValue){//Ԥ���true��֧���л�ȷ��������������ǰ·���ĺ����ڵ�
#endif
							  analyzeNode(arcTrue->getTailVex(),curNode,NULL,isInterprocedural); 
#if Branch_Predict
							   if((g_project_use_condition_check)&&TV_TRUE == integerOperationType.treeValue)//�������ȷ������Ϊ�棬��ô����Ҫ��false��֧
#else
							   if(false&&TV_TRUE == integerOperationType.treeValue)//�������ȷ������Ϊ�棬��ô����Ҫ��false��֧
#endif
							  {
								    --theRestPathSize;
									nodestack.popMore(iPushNodeCount);
									return;
							  }
						}
			}
			/*�ٷ�֧*/

			/*if��֧��̽����false��֧����circle�ڵ�ֻ������������߼ٷ�֧����1��ֱ������ѭ�����·����2��ѭ���˹涨��������ô��false��֧
			����*/
			if(((vDoWhile == nodeType || vCircle == nodeType) && (g_project_loop_check_count  < backUpCircleNum|| 1 == backUpCircleNum))
				  || (vIf == nodeType || vSwitch == nodeType)
				  ){
						/*if��֧�ڵ��߼ٷ�֧ʱ��Ҫ�ָ���Ϣ��ѭ���ڵ���ֱ����false��֧ʱ��Ҫ�ָ���Ϣ*/
						if(vIf == nodeType || vSwitch == nodeType 
							  || ((vCircle == nodeType || vDoWhile == nodeType) && 1 == backUpCircleNum)
							  ){
									//�ָ���֧�ڵ���Ϣ������bdd���й�·����ŵ���Ϣ
									//analyzeNodeIndex=backUpVexIndext;

									curNode->circleNum = 0;
									//theRestPathSize++;
									beingAnalysisPathIndex++;
									pawalker.mypa.v_heap=backUpHeap;
									pawalker.mypa.pointto=backUpPointto;
									pawalker.mypa.isSensitivedSource = sensitiveBackUp;
									pawalker.mypa.isTaintedSource = taintBackUp;
									

									//pointCOut=bdd_exist(pointCOut,fdd_ithset(P));
									// pawalker.mypa.pointtoCOut|=pointCOut&fdd_ithvar(P,beingAnalysisPathIndex);
									//heapCOut=bdd_exist(heapCOut,fdd_ithset(P));
									//pawalker.mypa.v_heapCOut|=heapCOut&fdd_ithvar(P,beingAnalysisPathIndex);
									((FuncSymbolItem*)DataFlowAnalyzer::getCallStack().top())->function_int_map = backup_map;

									//xxy_backUp�ָ�
						}
						if((vCircle == nodeType || vDoWhile == nodeType) && 1 != backUpCircleNum)//ǿ�ƴ�ѭ����false��֧����
						{
							  integerOperationType.treeValue = TV_MUST_RETURN_TRUE; 
						}else{
							  integerOperationType.treeValue = TV_FALSE; 
						}
						visitVex(curNode,preNode,pawalker,beingAnalysisPathIndex,
							  curNode->getFirstOutArc(),
							  true,theRestPathSize,isInterprocedural,integerOperationType);

						//if( 1 && pawalker.findSensitiveSink)//����һ��sink�㣬�ҷ��ִ��󣬸�·�����ټ�������ߣ�ֱ�ӷ���
						if( 1 && getTainedSinkFlag4VexNode())//����һ��sink�㣬�ҷ��ִ��󣬸�·�����ټ�������ߣ�ֱ�ӷ���
						{
							  openConsole();
							  cout<<"tained sink triggered, quit current path...."<<endl;
							  closeConsole();
							  --theRestPathSize;
							  nodestack.popMore(iPushNodeCount);
							  recoverSinkFlag4VexNode();//sink���ֻ�ڱ����һ��·���Ͻ����ضϣ���Ӱ��һ�������ڵ�����·��
							  return;
						}
#if Branch_Predict
						if((!g_project_use_condition_check)||TV_FALSE != integerOperationType.treeValue){//Ԥ���·�����л�ȷ��������������ǰ·���ĺ����ڵ�
#else
							if(true||TV_FALSE != integerOperationType.treeValue){//Ԥ���·�����л�ȷ��������������ǰ·���ĺ����ڵ�
#endif
#if futeng_path_output
							  futeng_path<<endl<<"A path:"<<endl;
#endif
							  nodestack.printStack();
							  analyzeNode(arcFalse->getTailVex(),curNode,NULL,isInterprocedural);
						}else{
							--theRestPathSize;
						}
			}
			nodestack.popMore(iPushNodeCount);
			return;
}

pathToTravel DataFlowAnalyzer::choosePath(VexNode* curNode,VexNode* preNode)
{
	  pathToTravel returnType=BothPath;
	  if(NULL==curNode||NULL==preNode)
	  {
			return BothPath;
	  }
	  VexValueSet* pPreOutValue=&(preNode->outValues);
	  ConditionTreeWalker con_treewalker(&(pawalker.mypa), pPreOutValue); //��һ����������ָ�����Ϣ���ڶ��������������͵���Ϣ
	  CFGNode* p=curNode->getData();
	  RefAST ast=p->getAST();
	  ConstraintResultType type=con_treewalker.start(ast);
	  switch(type)
	  {
	  case conflict: 
			//returnvalue=false;        //��ֹ��ǰ·������������һ��·����ʼ����
			returnType=FalsePath;
			break;
	  case satiable:
			//returnvalue=true;  
			returnType=TruePath;//��������
			break;
	  default: // case uncertain
			//�����µ���Ϣ��
			//returnvalue=true;         //��������
			returnType=BothPath;
	  }
	  //if(vCircle==curNode->getNodeType())
	  //{
	  //	if(FalsePath==returnType)
	  //	{
	  //		//�����ǰѭ������ȷ��Ϊ�٣���ֱ����false��֧������Ҫ��
	  //	}else{
	  //		returnType=BothPath;
	  //	}
	  //}
	  return returnType;
}
//deque<pair<int,VexNode*> >& DataFlowAnalyzer::getSwitches(VexNode *node,int curIndex)
//{
//	if(node->getNodeType()==vCircle||node->getNodeType()==vDoWhile)
//	{
//		pair<int,VexNode*> p(curIndex,node);
//		switchHeadDeque.push_back(p);
//		return switchHeadDeque;
//	}
//	int outNum = node->getOutNum(); 
//	int inNum = node->getInNum();
//	ArcBox* arc = node->getFirstInArc();
//	while(arc)
//	{	
//		VexNode* preVex = arc->getHeadVex();
//		if(preVex->getRank() >= node->getRank()
//			&& node->getData()->getNodeType()!=eNormalExitNode)
//			inNum--;
//		arc = arc->getNextEdgeWithSameHeadVex();
//	}
//	VexNodeType nodeType = node->getNodeType();
//	if(inNum > 1)
//	{
//		if(!switchHeadDeque.empty())//��Ϊǰ����bug,������ʱ���������������
//			switchHeadDeque.pop_back();	
//	}
//	if(outNum == 1 || outNum == 0)
//	{
//		return switchHeadDeque;
//	}
//	if( nodeType != vCircle && nodeType != vDoWhile )
//	{
//		pair<int,VexNode*> p(curIndex,node);
//		switchHeadDeque.push_back(p);
//	}
//	return switchHeadDeque;
//}
//������ȱ���ÿ�������Ľ��(����֮ǰ�ı���)
//void DataFlowAnalyzer::analyze(int isInterprocedural,int deep)
//{
//	if(!needAnalysis())	//��ǰ������Ŀ����ջ��
//	{
//		return;
//	}
//	pushCallStack();// ����ǰ������Ŀѹ�뺯������ջ
//	
//	if(m_func)
//	{
//		if(isInterprocedural)
//			depth<<"�����ú�����"<<m_func->getName()<<endl;
//		else
//			depth<<"��ں�����"<<m_func->getName()<<endl;
//	}	
//	
//	IFCollector->CCAFunc = bddfalse;
//	pawalker.setAnalyzer(this);
//	
//	CFG *cfg=(CFG *)m_func->getCFG();
//	analyzeNode(cfg->getHead(),NULL,NULL);
//	
//}
//void DataFlowAnalyzer::analyzeNode(VexNode *curNode,VexNode *preNode,ArcBox *arc)
//{
//	static int pathCnt=1;//�����жϵ�ǰ�м���·��Ҫ����
//	//bool bFinalPath=false;
//	while(curNode!=NULL&&curNode->getOutNum()<=1)
//	{
//		depth<<*curNode<<endl;
//		if(curNode->getLocalIndex()==1)
//		{
//			pathCnt--;
//			if(0==pathCnt)
//			{
//				depth<<"pathCnt==0 ����������"<<endl;
//				bFinalPath=true;
//			}
//			return;
//		}
//		m_treeWalker->walkAstOnVex(curNode,preNode);
//		preNode=curNode;
//		curNode=curNode->getFirstOutArc()->getTailVex();
//	}
//	ArcBox* arcTrue=curNode->getFirstOutArc();
//	ArcBox* arcFalse=arcTrue->getNextEdgeWithSameHeadVex();
//	ArcBox* arctemp;
//	if(arcTrue->getEdgeType()!=eTrue)
//	{
//		arctemp=arcTrue;
//		arcTrue=arcFalse;
//		arcFalse=arctemp;
//	}
//	set<VexNode*>::iterator it=analyzedNodeSet.find(curNode);
//	if(it!=analyzedNodeSet.end())
//	{
//		analyzedNodeSet.erase(it);
//		analyzeNode(arcFalse->getTailVex(),preNode,NULL);
//		return;
//	}else{
//		if(curNode->nodeType==vCircle||curNode->nodeType==vIf)
//		{
//			if(curNode->nodeType==vCircle)
//			{
//				analyzedNodeSet.insert(curNode); 
//			}
//			m_treeWalker->pathToTravel=pathBoth;
//			m_treeWalker->setbDecidePathToTravel(true);//���ÿ��أ���һ�α�������ڵ㣬����ȷ��Ҫ�ߵ�·��
//			//m_treeWalker->expressionTree((RefMyAST)(curNode->getData()->getAST()),MingledSymbolItem());
//			m_treeWalker->walkAstOnVex(curNode,preNode);
//			m_treeWalker->setbDecidePathToTravel(false);
//			m_treeWalker->setVexIsCircleHead(true);//��ֵ����
//			m_treeWalker->walkAstOnVex(curNode,preNode);
//		}
//
//		if(m_treeWalker->pathToTravel==pathTrue)
//		{
//			depth<<"��֧��"<<*curNode<<"ֻ�����true��֧"<<endl;
//			m_treeWalker->walkAstOnVex(curNode,preNode);
//			analyzeNode(arcTrue->getTailVex(),curNode,NULL);
//		}else if(m_treeWalker->pathToTravel==pathFalse){
//			depth<<"��֧��"<<*curNode<<"ֻ�����false��֧"<<endl;
//			m_treeWalker->walkAstOnVex(curNode,preNode);
//			analyzeNode(arcFalse->getTailVex(),curNode,NULL);
//		}else{
//			depth<<"��֧��"<<*curNode<<"������֧�������"<<endl;
//			pathCnt++;
//			m_treeWalker->walkAstOnVex(curNode,preNode);
//			analyzeNode(arcTrue->getTailVex(),curNode,NULL);
//			analyzeNode(arcFalse->getTailVex(),curNode,NULL);
//		}
//		//pathCnt++;
//		//depth<<"��֧��"<<*curNode<<endl;
//		//analyzeNode(arcTrue->getTailVex(),curNode,NULL);
//		//analyzeNode(arcFalse->getTailVex(),curNode,NULL);
//	}
//}

//void DataFlowAnalyzer::analyzeNode(VexNode *curNode,VexNode *preNode,ArcBox *arc)
//{
//	static int pathCnt=0;
//	if(curNode==NULL)
//	{
//		depth<<"finish one roda"<<++pathCnt<<endl;
//		return;
//	}else if(curNode->getOutNum()<=1){
//		depth<<*curNode<<endl;
//		analyzeNode(curNode->getFirstOutArc()->getTailVex(),curNode,NULL);
//	}else if(2==curNode->getOutNum()){
//		ArcBox* arcTrue=curNode->getFirstOutArc();
//		ArcBox* arcFalse=arcTrue->getNextEdgeWithSameHeadVex();
//		ArcBox* arctemp;
//		if(arcTrue->getEdgeType()!=eTrue)
//		{
//			arctemp=arcTrue;
//			arcTrue=arcFalse;
//			arcFalse=arctemp;
//		}
//		set<VexNode*>::iterator it=analyzedNodeSet.find(curNode);
//		if(it!=analyzedNodeSet.end())
//		{
//			analyzedNodeSet.erase(it);
//			analyzeNode(arcFalse->getTailVex(),preNode,NULL);
//			return;
//		}
//		if(curNode->nodeType==vCircle)
//		{
//			analyzedNodeSet.insert(curNode);
//					depth<<"��֧��"<<*curNode<<endl;
//		analyzeNode(arcTrue->getTailVex(),curNode,NULL);
//		analyzeNode(arcFalse->getTailVex(),curNode,NULL);
//		}
//		//ArcBox* arcTrue=curNode->getFirstOutArc();
//		//ArcBox* arcFalse=arcTrue->getNextEdgeWithSameHeadVex();
//		//ArcBox* arctemp;
//		//if(arcTrue->getEdgeType()!=eTrue)
//		//{
//		//	arctemp=arcTrue;
//		//	arcTrue=arcFalse;
//		//	arcFalse=arctemp;
//		//}
//		depth<<"��֧��"<<*curNode<<endl;
//		analyzeNode(arcTrue->getTailVex(),curNode,NULL);
//		analyzeNode(arcFalse->getTailVex(),curNode,NULL);
////		if(curNode->nodeType==vCircle)
//		//{
//		//	analyzeNode(arcFalse->getTailVex(),curNode,NULL);
//		//}
//	}
//}
//void DataFlowAnalyzer::analyzeNode(VexNode *curNode,VexNode *preVode,ArcBox *arc)
//{
//	static int pathCnt=-1;//·����ż���������������������·����
//	static int analyzeBranch=false;//�ñ�־Ϊtrueʱ��������֧�ڵ�����ݹ飬��ֹ������ѭ��
//	int curPath;
//	VexNode *pre=preVode;
//	//�洢��ǰ��һ·������Ϣ���Ժ�ϲ���IFCollector
//	bdd ccaPath=bddfalse;
//	bdd ccBranch=bddfalse;
//	curPath=++pathCnt;//��ǰ·�����
//	while(curNode!=NULL)
//	{
//		if(curNode->getOutNum()<2||(2==curNode->getOutNum()&&analyzeBranch))
//		{
//			if(2==curNode->getOutNum()&&arc->getEdgeType()==eTrue&&findNodeInNodeStack(curNode))//�ڵ��Ѿ���ջ�У�˵����һ��ѭ����β��㣬ֱ������������false��֧
//			{
//				//??��ȫ�����ý��ķ�������
//				analyzeBranch=false;
//				pre=curNode;
//				ArcBox *myarc=curNode->getFirstOutArc();
//				while(myarc->getEdgeType()!=eFalse)
//					myarc=myarc->getNextEdgeWithSameHeadVex();
//				if(myarc->getEdgeType()!=eFalse)
//					depth<<"Ѱ�Ҵ���·������"<<endl;
//				curNode=myarc->getTailVex();
//				continue;
//			}
//			curNode->addAnalysisNum();//�ý�����������һ
//			if(analyzeBranch)
//			{
//				analyzeBranch=false;
//			}
//
//			depth<<*curNode<<endl;
//			
//			//�ռ���ǰ���������Ϣ
//			bool isTrue=false;
//			if(2==curNode->getOutNum())
//				isTrue=isTrueEdge(curNode,arc->getTailVex());
//			handleVexState(curNode,false,pawalker,m_treeWalker,isTrue);	
//			travelVex(curNode,false,pawalker,m_treeWalker,isTrue);//��δ����	if( true == isFirstNodeOnPath)
//				//�ж��Ƿ���ѭ��,��ջ
//			m_treeWalker->walkAstOnVex(curNode, preVode);
//			//�ռ���ǰ���������Ϣ end
//			if(1==curNode->getLocalIndex())//end ���
//			{
//				depth<<*curNode<<endl;
//				//һ��·�����꣬��Ҫ�ռ��������Ϣ
//				return;
//			}
//			pre=curNode;
//			if(curNode->getOutNum()!=2)
//			{
//				curNode=curNode->getFirstOutArc()->getTailVex();
//			}else{
//				curNode=arc->getTailVex();
//			}
//		}//outNum=1
//		else if(2==curNode->getOutNum()&&!analyzeBranch){			
//			ccaPath=IFCollector->CCAPath;//���ݵ�ǰ·����֧�ڵ�ǰ����Ϣ����Ϣ
//			ccBranch=IFCollector->CCBranch;
//			analyzeBranch=true;
//			ArcBox *arc=curNode->getFirstOutArc();
//			analyzeNode(curNode,pre,arc);
//			analyzeBranch=true;
//			/*
//			//��ԭ��֧�ڵ㴦��Ϣ
//			//??����P���е�����temp_pointto�е������Ƴ�
//			bdd temp_pointto=pawalker.mypa.pointtoCOut&fdd_ithvar(C, pre->getGlobalIndex())&fdd_ithvar(P,curPath+1); //xulei modify, 20100412.  vexCnt=>vexCnt-1
//			//��������֧���ǰһ�����ָ����Ϣ���Ƶ���ǰҪ������·��
//			bdd temp_vheap=pawalker.mypa.v_heapCOut&fdd_ithvar(C, pre->getGlobalIndex())&fdd_ithvar(P,curPath+1);
//			if (temp_pointto!=bddfalse)
//			{
//				temp_pointto=bdd_exist(temp_pointto,fdd_ithset(P));
//				pawalker.mypa.pointtoCOut|=temp_pointto&fdd_ithvar(P,pathCnt+1);
//	
//				temp_vheap=bdd_exist(temp_vheap,fdd_ithset(P));
//				pawalker.mypa.v_heapCOut|=temp_vheap&fdd_ithvar(P,pathCnt+1);
//			}
//
//			IFCollector->CCAPath=ccaPath;
//			IFCollector->CCBranch=ccBranch;
//			//��ԭ��֧�ڵ㴦��Ϣend*/
//			arc=arc->getNextEdgeWithSameHeadVex();
//			analyzeNode(curNode,pre,arc);
//			break;
//		}
//	}
//}
bool DataFlowAnalyzer::findNodeInNodeStack(VexNode *node)
{
	  //depth<<"##############  "<<*node<<endl;
	  //if(nodeStack.empty())
	  //{
	  //	nodeStack.push(node);
	  //	return false;
	  //}
	  //VexNode *curNode=nodeStack.top();
	  //if(curNode==node)
	  //{
	  //	nodeStack.pop();
	  //	return true;
	  //}else{
	  //	nodeStack.push(node);
	  //	return false;
	  //}
	  return false;
}
// �÷��������Ժ�����Ŀ��������������
void DataFlowAnalyzer::analyze(int isInterprocedural)	
{	

	  string sCurFuncName = "<NULL>";
	  //dzh
	  //streambuf* pOld = cout.rdbuf(DzhTest.rdbuf());
	  //end
	  if(!needAnalysis())						// �����ǰ������Ŀ����Ҫ�����򷵻�
	  {
			return;
	  }
	  pushCallStack();						// ����ǰ������Ŀѹ�뺯������ջ

	  //2010-9-16,ZLY,BEGIN,����·���б��з��뵱ǰ����·������
	  gp_CurPath = m_callStack.getCurPathInfo();
	  //2010-9-16,ZLY,BEGIN,����·���б��з��뵱ǰ����·������

	  currFuncItem=m_callStack.top();

	  if (currFuncItem)
	  {
			cout.rdbuf(pOld);
			if(isInterprocedural){
				  int iCallNum = m_callStack.getStackSize();
				  for(int i=0; i<iCallNum; i++)
						cout<<"  ";
				  cout<<"���ڷ��������ú���"<<currFuncItem->getName()<<"..."<<endl;	
			}
			/*
			else
			cout<<endl<<"���ڷ�����ں���"<<currFuncItem->getName()<<"..."<<endl;	
			*/
			if(isInterprocedural)
				  func<<"���ڷ��������ú���"<<currFuncItem->getName()<<"..."<<endl;	
			else
				  func<<"���ڷ�����ں���"<<currFuncItem->getName()<<"..."<<endl;	
			cout.rdbuf(DzhTest.rdbuf());	 
#ifdef PA_TRACE_LITTLE
			sCurFuncName = currFuncItem->getName();
			if(isInterprocedural)
				  paTrace<<"���ڷ��������ú���"<<sCurFuncName<<"..."<<endl;
			/*
			else
			paTrace<<"���ڷ�����ں���"<<sCurFuncName<<"..."<<endl;	
			*/
#endif
	  }

	  time_t begin, end;
	  time(&begin);

	  //dataflow<<getCallStack();				// ��ӡ����ջ, �ݱ��� 			
	  CFGPathsManager* pathsMgr = CFGPathsManager::instance();	// ���·��������ʵ��
	  CFGPaths& paths = pathsMgr->pathsOfFunction(m_func);		// ��·���������л�ȡ������������·��
	  dataFlowPaths[m_func] = paths;  // 20100106 by kong
	  //dataflow << dataFlowPaths[m_func] << "kongdelan " << endl; // 20100106 by kong
	  //dataflow<<paths;							// ��ӡ����·��
#if futeng_path_output
	  futeng_path<<"����·����\n"<<paths<<endl;
#endif
	  if(gb_OnlyOneEntry && isInterprocedural == false){
			int i;
			gl_TotalEntryNodes = 0;
			for(i=0; i<paths.size(); i++)
				  gl_TotalEntryNodes += paths[i]->length();
			gl_CurrentAnalyzedNodes = 0;
	  }
	  //	Path prePath, curPath;						// �ֱ���������ǰһ��·���͵�ǰ·���Ľڵ����
	  //cout << "current function is: " << paths.getFunc()->getName() << endl;

	  Path *prePathPtr=0, *curPathPtr=0;
	  int pathCnt=0;
	  bdd temp=bddfalse;//cxx
	  //	while(paths.getNextPath(curPath, pathCnt))	// ������·���в�ͣ�Ļ�ȡ��һ��·�����з���

	  int allNodesNum = getAllNodesNum(paths);    //�õ����нڵ�������ú������ڽ��������ƣ�û�б����;
	  int oneNode=0;    //��ǰ�ڼ����ڵ㣬���ڿ���������

	  //IFCollector->CCAFunc = bddfalse;//IFCollectorȫ�ֱ���
	  bool bFinalPath = false;
	  processbar_Cur.setCurrentAnalysisInfo(string("���� ") + sCurFuncName, paths.size(), pathCnt);

	  pawalker.setAnalyzer(this);
	  while(curPathPtr=paths[pathCnt])
	  {
			cout.rdbuf(pOld);
			if(isInterprocedural){
				  int iCallNum = m_callStack.getStackSize();
				  for(int i=0; i<iCallNum; i++)
						cout<<"  ";
				  cout<<"  ���ڷ������� "<<sCurFuncName <<" �ĵ�("<<pathCnt+1<<"/"<<paths.size()<<")��·��..."<<endl;	
			}
			else
				  cout<<"  ���ڷ������� "<<sCurFuncName <<" �ĵ�("<<pathCnt+1<<"/"<<paths.size()<<")��·��..."<<endl;	
			cout.rdbuf(DzhTest.rdbuf());	 

			//2010-08-12,ZLY,BEGIN,�ж��Ƿ������һ��·��
			if(pathCnt == paths.size() - 1)
				  bFinalPath = true;
			//2010-08-12,ZLY,END

#ifdef PA_TRACE
			paTrace<<"Begin a new path:"<<endl;
#endif
#ifdef PA_TRACE_LITTLE
			paTrace<<"\t2010-08-11,ZLY, ������"<<pathCnt+1<<"��·��:"<<endl;
			paTrace<<"\t\t"<<(*curPathPtr)<<endl;
#endif
			//2010-08-11,ZLY,BGING,�������н��ķ������λΪfalse
			curPathPtr->resetAllNodeFlag();
			//2010-08-11,ZLY,END
			NodeList.clear();           //ÿ�δ�һ����·����ʼ�������ڵ�������

			//2010-9-16,ZLY,BEGIN,����ǰ����·����Ϣ
			gp_CurPath->clear();
			//2010-9-16,ZLY,BEGIN,����ǰ����·����Ϣ


			if (IFCollector->CCAPath != bddfalse) {
				  IFCollector->CCAFunc |= IFCollector->CCAPath;
			}
			IFCollector->CCAPath = bddfalse; // by kong 20100520
			IFCollector->CCBranch = bddfalse; // by kong 20100520
			if (IFCollector->CCAPath == bddfalse)
				  cout << "bddfalse bddfalse" << endl;
			//dzh
			bool isFirstNodeOnPath = true;
			//dzh end
			int vexCnt = 0;

			if(prePathPtr) vexCnt = curPathPtr->firstDifferentVex(*prePathPtr);
			//2010-08-23,ZLY,�����������⣬���������GetMaxSameSubPath����˵��
			/*
			//2010-08-22,ZLY,BEGIN,����ǰ��·���������ͬ·��
			int iFirstDiffIndex, iFirstDiffIndex_PathIndex;
			Path* pTargetHistoryPath;
			if(prePathPtr){
			GetMaxSameSubPath(paths, pathCnt, iFirstDiffIndex, iFirstDiffIndex_PathIndex);
			if(iFirstDiffIndex > 0){
			vexCnt = iFirstDiffIndex;
			pTargetHistoryPath = paths[iFirstDiffIndex_PathIndex];
			}
			}
			//2010-08-22,ZLY,END
			*/

			if(vexCnt>0) vexCnt--;
			/*
			for(int i = 0; i<vexCnt; i++) {						// ��ӡ��ǰһ·���������ֵ���������Ϣ
			dataflow<<" *VexNode "<<(*curPathPtr)[i]->getLocalIndex()<<": "
			<<(*curPathPtr)[i]->inValues<<endl;
			NodeList.push_back((*curPathPtr)[i]->getLocalIndex());     //��ʼ���ֵĹ����ڵ��ȼ��뵽�ڵ������
			IFCollector->CCAPath |= IFCollector->CCAFunc & fdd_ithvar(C, (*curPathPtr)[i]->getGlobalIndex());
			}
			*/
			//cout << "IFCollector->CCAFunc: " << IFCollector->CCAPath << endl;

			VexNode* preVex, *curVex;
			VexNode* tmpVex = NULL;
			//cxx
			//xulei �޸�ԭ����Ϊ�Ǵӷֲ�ڵ㿪ʼ������������Ҫ�õ��ֲ�ڵ����һ���ڵ��Out��Ϣ�������Ƿֲ�ڵ��Out��Ϣ��
			bdd temp_pointto=bddfalse;
			bdd temp_CovertChannelAAll=bddfalse;
			bdd temp_vheap=bddfalse;

			if (vexCnt)
			{
#ifdef PA_TRACE
				  paTrace<<"\t\tvexCnt="<<vexCnt<<endl;
#endif
#ifdef PA_TRACE_LITTLE
				  tmpVex = (*curPathPtr)[vexCnt-1];
				  if(tmpVex == NULL){
						paTrace<<"2010-08-13,ZLY,�������δ�ҵ���ǰ·���ϲ����ڽ���±�"<<vexCnt-1<<""<<endl;
				  }else{
						paTrace<<"\t\t2010-08-13,ZLY, ȡ����ʷ·���Ͻ��"<<tmpVex->getLocalIndex()<<"����ʷ�������..."<<endl;
				  }
#endif
				  Path * targetPath;
				  //2010-08-11,ZLY,�ҵ�֮ǰ·�������һ����������Ŀ���㣬ȡ����ָ����Ϣ
				  int iTargetPathIndex = pathCnt - 1;
				  // 			int iTargetPathIndex = pathCnt-1;
				  while(iTargetPathIndex >= 0){
						targetPath = paths[iTargetPathIndex];
						if(targetPath == NULL){
							  paTrace<<"2010-08-11,ZLY,�������δ�ҵ���ʷ���������·��Ϊ�գ�!"<<endl;
							  iTargetPathIndex = -1;
							  break;
						}
						tmpVex = (*targetPath)[vexCnt-1];
						if(targetPath->getAnalyzedFlag(vexCnt-1) == Analyzed)
							  break;
						if(targetPath->getAnalyzedFlag(vexCnt-1) == NotReachable){
							  if(tmpVex == NULL){
									paTrace<<"2010-08-11,ZLY,������󣺵�"<<iTargetPathIndex+1<<"��·���ϲ����ڽ���±�:"<<vexCnt-1<<endl;
							  }else{
#ifdef PA_TRACE_LITTLE
									paTrace<<"\t\t2010-08-11,ZLY, ���ֵ�"<<iTargetPathIndex+1<<"��·�����"<<tmpVex->getLocalIndex()<<"���ɴ�δ����!"<<endl;
#endif
							  }
							  iTargetPathIndex = -1;
							  break;
						}
						if(tmpVex == NULL){
							  paTrace<<"2010-08-11,ZLY,������󣺵�"<<iTargetPathIndex+1<<"��·���ϲ����ڽ���±�:"<<vexCnt-1<<endl;
						}else{
#ifdef PA_TRACE_LITTLE
							  paTrace<<"\t\t2010-08-11,ZLY, ���ֵ�"<<iTargetPathIndex+1<<"��·�����"<<tmpVex->getLocalIndex()<<"δ������!"<<endl;
#endif
						}
						iTargetPathIndex--;
				  }

				  if(iTargetPathIndex < 0){
						/*
						VexNode* tmpCurVex = (*curPathPtr)[vexCnt-1];
						if(tmpCurVex != NULL)
						paTrace<<"2010-08-11,ZLY,�������δ�ҵ����"<<tmpCurVex->getGlobalIndex()<<"����ʷ�������!"<<endl;
						else
						paTrace<<"2010-08-11,ZLY,������󣺵�ǰ·��(��"<<pathCnt+1<<"��)�ϲ������±�Ϊ"<<vexCnt-1<<"�Ľ��!"<<endl;
						prePathPtr = curPathPtr;
						pathCnt++;
						continue;
						*/
						//2010-08-13,ZLY,BEGIN,���ܴ�ͷ��ʼ������Ҫȡ������ڽ��(0)����ʷ���������
						//                     ����·����ʼ��״̬���Ǻ�����ڴ���״̬
						/*
						paTrace<<"\t\t2010-08-11,ZLY,δȡ����ʷ�������������·���ӿ�ʼ������!"<<endl;
						vexCnt = 0;
						//2010-08-10,ZLY,����ʹ��temp������temp_pointto�᲻��������???
						temp=pawalker.mypa.pointtoCOut&fdd_ithvar(C, (*curPathPtr)[vexCnt]->getGlobalIndex())&fdd_ithvar(P,pathCnt); 
						//temp_CovertChannelAAll=pawalker.mypa.CovertChannelAAllCout&fdd_ithvar(C, (*curPathPtr)[vexCnt]->getGlobalIndex())&fdd_ithvar(P,pathCnt);
						temp_vheap=pawalker.mypa.v_heapCOut&fdd_ithvar(C, (*curPathPtr)[vexCnt]->getGlobalIndex())&fdd_ithvar(P,pathCnt);
						*/
						paTrace<<"\t\t2010-08-11,ZLY,δȡ����ʷ�������������·���ӿ�ʼ����ʼ����!"<<endl;
						vexCnt = 1;
						iTargetPathIndex = 0;
						targetPath = paths[iTargetPathIndex];//ȡ����1��·��
						if(targetPath == NULL){
							  paTrace<<"2010-08-13,ZLY,�������δ�ҵ���1��·����·��Ϊ�գ�! ������ǰ·������!"<<endl;
							  prePathPtr = curPathPtr;
							  pathCnt++;
							  continue;
						}
						if(targetPath->getAnalyzedFlag(vexCnt-1) != Analyzed){
							  paTrace<<"2010-08-13,ZLY,������󣬷��ֵ�1��·�����0δ������! ������ǰ·������!"<<endl;
							  prePathPtr = curPathPtr;
							  pathCnt++;
							  continue;
						}

						//2010-08-13,ZLY,END
				  }
				  //2010-08-13,ZLY,BEGIN,��ǰ�淢��û�ҵ���ʷ������������ȡ��һ��·�����(0)����ʷ�������
				  //                     ��˱ض���ȡ��ʷ�������
				  //else
				  //2010-08-13,ZLY,END
				  {
#ifdef PA_TRACE
						paTrace<<"\t\tiTargetPathIndex="<<iTargetPathIndex<<endl;
						paTrace<<"\t\tvexCnt="<<vexCnt<<endl;
#endif
						for(int i = 0; i<vexCnt; i++) {						// ��ӡ��ǰһ·���������ֵ���������Ϣ
							  dataflow<<" *VexNode "<<(*curPathPtr)[i]->getLocalIndex()<<": "
									<<(*curPathPtr)[i]->inValues<<endl;
							  NodeList.push_back((*curPathPtr)[i]->getLocalIndex());     //��ʼ���ֵĹ����ڵ��ȼ��뵽�ڵ������

							  //2010-9-16,ZLY,BEGIN,����ǰ����·����Ϣ
							  gp_CurPath->push_back((*curPathPtr)[i]->getLocalIndex());
							  //2010-9-16,ZLY,BEGIN,����ǰ����·����Ϣ

							  //IFCollector->CCAPath |= IFCollector->CCAFunc & fdd_ithvar(C, (*curPathPtr)[i]->getGlobalIndex());
						}

						tmpVex = (*(paths[iTargetPathIndex]))[vexCnt-1];
						//				if(tmpVex == NULL){
						//					paTrace<<"2010-08-11,ZLY,�������(2)����"<<iTargetPathIndex+1<<"��·���ϲ����ڽ���±�:"<<vexCnt-1<<endl;
						//				}else{
						//#ifdef PA_TRACE_LITTLE
						//					paTrace<<"\t\t2010-08-11,ZLY, ȡ��"<<iTargetPathIndex+1<<"��·�����"<<tmpVex->getLocalIndex()<<"����ʷ�������..."<<endl;
						//#endif
						//				}
						giHistoryPathVexCount += vexCnt;
						//temp_pointto=pawalker.mypa.pointtoCOut&fdd_ithvar(C, (*curPathPtr)[vexCnt-1]->getGlobalIndex())&fdd_ithvar(P,iTargetPathIndex+1); //xulei modify, 20100412.  vexCnt=>vexCnt-1
						//temp_CovertChannelAAll=pawalker.mypa.CovertChannelAAllCout&fdd_ithvar(C, (*curPathPtr)[vexCnt-1]->getGlobalIndex())&fdd_ithvar(P,iTargetPathIndex);
						//temp_vheap=pawalker.mypa.v_heapCOut&fdd_ithvar(C, (*curPathPtr)[vexCnt-1]->getGlobalIndex())&fdd_ithvar(P,iTargetPathIndex+1);
#ifdef PA_TRACE
						paTrace<<"\t\tfutengȡ�����  temp_pointto is��"<<endl;
						paTrace<<"\t\t\t"<<fddset<<temp_pointto<<endl;
#endif

						/*
						temp_pointto=pawalker.mypa.pointtoCOut&fdd_ithvar(C, (*curPathPtr)[vexCnt-1]->getGlobalIndex())&fdd_ithvar(P,pathCnt); //xulei modify, 20100412.  vexCnt=>vexCnt-1
						//temp_CovertChannelAAll=pawalker.mypa.CovertChannelAAllCout&fdd_ithvar(C, (*curPathPtr)[vexCnt-1]->getGlobalIndex())&fdd_ithvar(P,pathCnt);
						temp_vheap=pawalker.mypa.v_heapCOut&fdd_ithvar(C, (*curPathPtr)[vexCnt-1]->getGlobalIndex())&fdd_ithvar(P,pathCnt);
						*/
						//2010-08-11,ZLY,END
#ifdef PA_TRACE
						paTrace<<"Cur pointtoCOut is:"<<endl<<"\t"<<fddset<<pawalker.mypa.pointtoCOut<<endl;
						paTrace<<"Get temp_pointto is:"<<endl<<"\t"<<fddset<<temp_pointto<<endl;
#endif
				  }
			}

			//else//vexCnt==0
			//{
			//	//2010-08-13,ZLY,ÿ����·������ǰ�����������ͬ�ģ���˲����ܳ���vexCnt����0�����
			//	if(prePathPtr){//��ǰ���ǵ�һ��·��
			//		paTrace<<"2010-08-13,ZLY,������󣬷���·���쳣û����ͬ����! ������ǰ·������!"<<endl;
			//		prePathPtr = curPathPtr;
			//		pathCnt++;
			//		continue;
			//	}
			//	//2010-08-13.ZLY
			//	//2010-08-10,ZLY,����ʹ��temp������temp_pointto�᲻��������???
			//    temp=pawalker.mypa.pointtoCOut&fdd_ithvar(C, (*curPathPtr)[vexCnt]->getGlobalIndex())&fdd_ithvar(P,pathCnt); 
			//          //temp_CovertChannelAAll=pawalker.mypa.CovertChannelAAllCout&fdd_ithvar(C, (*curPathPtr)[vexCnt]->getGlobalIndex())&fdd_ithvar(P,pathCnt);
			//    temp_vheap=pawalker.mypa.v_heapCOut&fdd_ithvar(C, (*curPathPtr)[vexCnt]->getGlobalIndex())&fdd_ithvar(P,pathCnt);
			//}

			if (temp_pointto!=bddfalse)
			{
				  //temp_pointto=bdd_exist(temp_pointto,fdd_ithset(P));//??����P���е�����temp_pointto�е������Ƴ�
				  //pawalker.mypa.pointtoCOut|=temp_pointto&fdd_ithvar(P,pathCnt+1);//��������֧���ǰһ�����ָ����Ϣ���Ƶ���ǰҪ������·��

				  //temp_CovertChannelAAll=bdd_exist(temp_CovertChannelAAll,fdd_ithset(P));
				  //pawalker.mypa.CovertChannelAAllCout|=temp_CovertChannelAAll&fdd_ithvar(P,pathCnt+1);

				  //temp_vheap=bdd_exist(temp_vheap,fdd_ithset(P));
				  //pawalker.mypa.v_heapCOut|=temp_vheap&fdd_ithvar(P,pathCnt+1);
			}


#ifdef PA_TRACE
			paTrace<<"\tpointto is:"<<endl<<"\t\t"<<fddset<<pawalker.mypa.pointto<<endl;
			paTrace<<"\tv_heapCOut is:"<<endl<<"\t\t"<<fddset<<pawalker.mypa.v_heapCOut<<endl;
#endif

			bool is_feasible_path=true; //�Ƿ��ǿɴ�·��
			//cxx end

			if(gb_OnlyOneEntry && isInterprocedural == false){
				  if(vexCnt){
						gl_CurrentAnalyzedNodes += vexCnt;
						processbar.setBcck_OneFunction(gl_CurrentAnalyzedNodes, gl_TotalEntryNodes);
				  }
			}

			while(curVex = (*curPathPtr)[vexCnt])						// �ӵ�ǰ·����ǰһ·���ķǹ������ֿ�ʼ, ����ÿ���ڵ�
			{
				  /*
				  cout.rdbuf(pOld);
				  cout<<".";	
				  cout.rdbuf(DzhTest.rdbuf());	 
				  */

				  NodeList.push_back(curVex->getLocalIndex());

				  //2010-9-16,ZLY,BEGIN,����ǰ����·����Ϣ
				  gp_CurPath->push_back(curVex->getLocalIndex());
				  //2010-9-16,ZLY,BEGIN,����ǰ����·����Ϣ

				  preVex = (*curPathPtr)[vexCnt-1];
				  //paTrace<<"Before SM"<<endl;
				  {
						//dzh begin
						bool isTrue = false;
						VexNode* nextnode =  (*curPathPtr)[vexCnt+1];
						cout<<"the number is "<<curVex->getOutNum()<<endl;
						if(curVex->getOutNum() ==2)
							  isTrue = isTrueEdge(curVex,nextnode);

						time_t smNode_begin, smNode_end;
						smNode_begin = clock();
						//2010-08-11,ZLY,�����Ƿ�ɴ�������֧����״̬������ָ�
						handleVexState(curVex,isFirstNodeOnPath,pawalker,m_treeWalker,isTrue);
						//2010-08-11,ZLY,END
						if (is_feasible_path)   //�����ǰ·���ǿɴ�·����������ڵ�
							  travelVex(curVex,isFirstNodeOnPath,pawalker,m_treeWalker,isTrue);
						smNode_end = clock();
						func << "SM��㣺" << smNode_end - smNode_begin << endl;
						if( true == isFirstNodeOnPath) 
							  isFirstNodeOnPath = false;
						//dzh end
				  }
				  //paTrace<<"After SM"<<endl;

				  // cxx
				  if (!isInterprocedural)   //���Ǳ��������ڵĽڵ㣬��Ҫ������������
				  {
						++oneNode;
						processbar.setBackOne(oneNode, allNodesNum);
				  }

				  //paTrace<<"Before get int value"<<endl;
				  m_treeWalker->setVexIsCircleHead(curPathPtr->vexIsCircleHead(curVex));
				  m_treeWalker->walkAstOnVex(curVex, preVex);	
				  //			infoWalker->walkAstOnVex(curVex, preVex);// �����ڵ��Ӧ��AST, ��������ҵ��ڵ��ϣ�������ֵ������������
				  //paTrace<<"After get int value"<<endl;


				  time_t node_begin, node_end;
				  node_begin=clock();
				  //if(!visitVex(curVex,vexCnt,preVex,pawalker,pathCnt+1,(*curPathPtr).getSwitches(vexCnt),(*curPathPtr), true))
				  //if(!visitVex(curVex,vexCnt,preVex,pawalker,pathCnt+1,(*curPathPtr).getSwitches(vexCnt),(*curPathPtr), is_feasible_path, bFinalPath, isInterprocedural))
				  //{//����ֵ��false����ʾ��ǰ·���ǲ��ɴ�·��������һ��·����ʼ����
				  //	if(is_feasible_path == true){
				  //		paTrace<<"\t\t����һ�����ɴ�·�����ӽ�� ";
				  //		paTrace<<curVex->getLocalIndex()<<"/";
				  //		if(curVex->getLocalIndex() == 0)
				  //			paTrace<<"Start";
				  //		else if(curVex->getLocalIndex() == 1)
				  //			paTrace<<"End";
				  //		else if(curVex->getData()->getNodeType() == eNormalExitNode)
				  //			paTrace<<"Exit";
				  //		else if(curVex->getData()->getNodeType() == eScopeOutNode)
				  //			paTrace<<"}";
				  //		else if(curVex->getData()->getAST() != NULL)
				  //			paTrace<<((RefMyAST)(curVex->getData()->getAST()))->getLineNum();
				  //		else
				  //			paTrace<<"_"<<curVex->getLocalIndex();
				  //		paTrace<<" ��ʼ"<<endl;
				  //	}
				  //	xulei_dataflow<<"�ҵ�һ�����ɴ�·��"<<endl;
				  //	//2010-08-10,ZLY, ������·���Ĵ������BUG���ᵼ�·�֧������ջ�����ǳ���״̬δ����
				  //	is_feasible_path=false;
				  //	curPathPtr->setAnalyzedFlag(vexCnt, NotReachable);
				  //	//2010-08-10,ZLY, END
				  //    //break;
				  //}else{
				  //	//2010-08-11,ZLY,BGING,���õ�ǰ���ķ������λΪAnalyzed
				  //	curPathPtr->setAnalyzedFlag(vexCnt, Analyzed);
				  //	//2010-08-11,ZLY,END
				  //}
				  node_end=clock();
				  func<<"����:"<<curVex->getLocalIndex()<<",��ʱ�䣺"<<node_end-node_begin<<endl;
				  //putOutCCForEVex(curVex,pawalker);
				  // cxx end

				  dataflow<<" *VexNode "<<curVex->getLocalIndex()<<": ";
				  dataflow<<(curVex->inValues)<<endl;


				  vexCnt++;
				  if(gb_OnlyOneEntry && isInterprocedural == false){
						gl_CurrentAnalyzedNodes++;
						processbar.setBcck_OneFunction(gl_CurrentAnalyzedNodes, gl_TotalEntryNodes);
				  }
			} //.while(curVex = curPath[vexCnt])
			//genSwithCovertChannelInfor(pawalker,fdd_ithvar(C,v->getGlobalIndex()), dq,curPath ); // by kong 20100520
			//genBranchCovertChannelInfor(pawalker, (*curPathPtr).getSwitches(vexCnt),(*curPathPtr), IFCollector);
			dataflow<<endl;
			//	prePath=curPath;		// ��ǰ·����Ϊǰһ��·��, ������һ��ѭ��
			prePathPtr = curPathPtr;
			pathCnt++;
			processbar_Cur.setCurrentAnalysisInfo(string("���� ") + sCurFuncName, paths.size(), pathCnt);
	  } //.while(paths.getNextPath(curPath, pathCnt))
	  popCallStack();				// ��������, ��ǰ������ջ


	  /*2010-08-27,������ȡջ����Ҳ����Ҫȡ����Ϊÿ�η���һ����������ѹջ����currFuncItem,
	  �����ջ����ȡ�ᵼ��ջΪ��ʱ��Ȼȡtop������CallStack���в�û���ж�ʹ�õ�list�Ƿ�Ϊ��
	  paTrace<<"m_callStack size is:"<<m_callStack.getStackSize()<<endl;
	  */
	  if(m_callStack.getStackSize() > 0){
			currFuncItem=m_callStack.top();
			gp_CurPath = m_callStack.getCurPathInfo();
	  }
	  /*
	  if (IFCollector->CCToBeDeleted != bddfalse) {
	  #ifdef CCDebug
	  cout << "IFCollector->CCToBeDeleted" << endl;
	  #endif
	  IFCollector->CCAll -= IFCollector->CCToBeDeleted;
	  }
	  */
	  time(&end);
	  time_t diff=end-begin;
	  func<<"ʱ�䣺"<<diff<<endl;

#ifdef PA_TRACE_LITTLE
	  paTrace<<"��������"<<sCurFuncName<<"����"<<endl;	
#endif
	  cout.rdbuf(pOld);
	  if(isInterprocedural){
			int iCallNum = m_callStack.getStackSize() + 1;
			for(int i=0; i<iCallNum; i++)
				  cout<<"  ";
			cout<<"���������ú�������"<<sCurFuncName<<"����"<<endl;	
	  }
	  else
			cout<<"������ں�������"<<sCurFuncName<<"����"<<endl;	
	  pOld = cout.rdbuf(DzhTest.rdbuf());	 

	  //	cout.rdbuf(pOld);
	  //��Ϣ���Ѿ��ϲ� ���� 2009-09-18
	  //cout<<"111111111111111111111"<<pawalker.mypa.pointtoCOut<<endl;
}  

//2010-08-11,ZLY,BEGIN:�����֧����ϵ�״̬������ָ�
//����������еĴ���ԭ��λ��travelVex�Ŀ�ʼ����
void DataFlowAnalyzer::handleVexState(VexNode* v,bool isFirstNodeOnPath,PATreeWalker& pawalker,ExprValueTreeWalker* expTreewalker,bool IsTrueEdge)
{
	  //	cout<<"dzh...................1"<<endl;
	  //����Ƿ�֧�ڵ㣬ͬʱ�Ǹ÷�֧�ڵ��Ѿ������������÷�֧�ڵ��In��Ϣ�Ѿ��洢
	  //	if (isFirstNodeOnPath && v->pInfo != NULL)
	  if(2==v->getAnalysisNum()&&2==v->getOutNum()&&v->pInfo!=NULL)
	  {
			AllStateInfo* pState = static_cast<AllStateInfo*>(v->pInfo);
			map<SymbolItem*,list<AttachToSymbolItem*>*> symbolState = pState->symbolStates;
			map<_SM*,list<State*> > virtualState = pState->virtualStates;		
			//���ȫ��״̬����
			ProgramState.Symbol_list.clear();
			ProgramState.Virtual_Set.clear();
			//������֧�ڵ���Ϣ��������Ŀ��(������Ϣ�ǰ󶨵�������Ŀ�ϵ���Ϣ)
			map<SymbolItem*,list<AttachToSymbolItem*>*>::iterator iter1 = symbolState.begin();
			while(iter1 != symbolState.end())
			{
				  ProgramState.Symbol_list.push_back((*iter1).first);

				  //�ͷŹҽ��ڷ�����Ŀ�ϵ�״̬��Ϣ
				  list<AttachToSymbolItem*>* tmp = (list<AttachToSymbolItem*>*)((*iter1).first->getSM_State());
				  list<AttachToSymbolItem*>::iterator iter_att = tmp->begin();
				  while(iter_att != tmp->end())
				  {
						//	delete (*iter_att);
						++iter_att;
				  }
				  //�������ڷ�֧�ڵ��״̬��Ϣ�ҽӵ�������Ŀ��
				  ((*iter1).first)->setSM_State((*iter1).second);
				  ++iter1;
			}

			//������֧��Ϣ��ά��ȫ�ֳ���״̬�Ķ���
			//��Ϊ���󶨵���������Щ״̬�޷��ҽӵ�������Ŀ�ϣ������һ��ȫ�ּ�����ά��

			map<_SM*,list<State*> >::iterator iter2 = virtualState.begin();
			while(iter2 != virtualState.end())
			{
				  ProgramState.Virtual_Set.insert(virtual_value((*iter2).first,(*iter2).second));
				  ++iter2;
			}
#ifdef SM_RESTORE_TRACE
			smStateRestore<<"Restored state for vetex:"<<v->getLocalIndex()<<endl;
			smStateRestore<<"vetex ast is:"<<endl;
			smStateRestore<<v->getData()->getAST()->toRealStringTree()<<endl;
			smStateRestore<<"\tVirtual states:"<<endl;
			map<_SM*,list<State*> >::iterator itVirtual;
			for(itVirtual=ProgramState.Virtual_Set.begin(); itVirtual!=ProgramState.Virtual_Set.end(); itVirtual++){
				  _SM* sm = (_SM*)(itVirtual->first);
				  list<State*> smStates = (list<State*>)(itVirtual->second);
				  smStateRestore<<"\t\tSM '"<<sm->getName()<<"':"<<endl;
				  for(list<State*>::iterator itVirtualState = smStates.begin(); itVirtualState != smStates.end(); itVirtualState++){
						smStateRestore<<"\t\t\t"<<(*itVirtualState)->getliteral()<<endl;
				  }
			}
			smStateRestore<<"\tVariable states:"<<endl;
			for(list<SymbolItem*>::iterator itSybolItem = ProgramState.Symbol_list.begin();itSybolItem != ProgramState.Symbol_list.end(); itSybolItem++){
				  smStateRestore<<"\t\tSymbol '"<<(*itSybolItem)->getName()<<"'"<<endl;
				  list<AttachToSymbolItem*>* pSymbStates = (list<AttachToSymbolItem*>*)((*itSybolItem)->getSM_State());
				  int iTmp = 0;
				  for(list<AttachToSymbolItem*>::iterator itSymbolAttached = pSymbStates->begin(); itSymbolAttached != pSymbStates->end(); itSymbolAttached++){
						smStateRestore<<"\t\t\tAttached "<<(*itSymbolAttached)->Mingled_Item<<endl;
						map<_SM*,State*>* stateList = (*itSymbolAttached)->Sm_list;
						for(map<_SM*,State*>::iterator itFinalState = stateList->begin(); itFinalState != stateList->end(); itFinalState++){
							  _SM* sm1 = (_SM*)(itFinalState->first);
							  State * smState = (State *)(itFinalState->second);
							  smStateRestore<<"\t\t\t\tsm='"<<sm1->getName()<<"', state='"<<smState->getliteral()<<"'"<<endl;
						}
				  }
			}
#endif
	  } 
	  else
	  {
			//��ǰ�ڵ��Ƿ�֧�ڵ㣬ͬʱ��û������
			//����ýڵ���2�����ߣ���ô�ýڵ�Ϊ��֧�ڵ㣬���������Ϣ��
			//�Ƿ�֧�ڵ���Ϣ��ʱ�����ǣ����ڵ�ǰ����������Ƿ�֧�ڵ��In,Out��Ϣû��ʹ��
			//�����ÿ���ڵ��In,Out��Ϣ�����д洢�Ļ�������̫���ˣ�����ֻ�洢���õ�In,Out��Ϣ
			if(2 == v->getOutNum())
			{
#ifdef SM_RESTORE_TRACE
				  smStateRestore<<"Saving state for vetex:"<<v->getLocalIndex()<<endl;
				  smStateRestore<<"vetex ast is:"<<endl;
				  smStateRestore<<v->getData()->getAST()->toRealStringTree()<<endl;
				  smStateRestore<<"\tVirtual states:"<<endl;
				  map<_SM*,list<State*> >::iterator itVirtual;
				  for(itVirtual=ProgramState.Virtual_Set.begin(); itVirtual!=ProgramState.Virtual_Set.end(); itVirtual++){
						_SM* sm = (_SM*)(itVirtual->first);
						list<State*> smStates = (list<State*>)(itVirtual->second);
						smStateRestore<<"\t\tSM '"<<sm->getName()<<"':"<<endl;
						for(list<State*>::iterator itVirtualState = smStates.begin(); itVirtualState != smStates.end(); itVirtualState++){
							  smStateRestore<<"\t\t\t"<<(*itVirtualState)->getliteral()<<endl;
						}
				  }
				  smStateRestore<<"\tVariable states:"<<endl;
				  for(list<SymbolItem*>::iterator itSybolItem = ProgramState.Symbol_list.begin();itSybolItem != ProgramState.Symbol_list.end(); itSybolItem++){
						smStateRestore<<"\t\tSymbol '"<<(*itSybolItem)->getName()<<"'"<<endl;
						list<AttachToSymbolItem*>* pSymbStates = (list<AttachToSymbolItem*>*)((*itSybolItem)->getSM_State());
						int iTmp = 0;
						for(list<AttachToSymbolItem*>::iterator itSymbolAttached = pSymbStates->begin(); itSymbolAttached != pSymbStates->end(); itSymbolAttached++){
							  smStateRestore<<"\t\t\tAttached "<<(*itSymbolAttached)->Mingled_Item<<endl;
							  map<_SM*,State*>* stateList = (*itSymbolAttached)->Sm_list;
							  for(map<_SM*,State*>::iterator itFinalState = stateList->begin(); itFinalState != stateList->end(); itFinalState++){
									_SM* sm1 = (_SM*)(itFinalState->first);
									State * smState = (State *)(itFinalState->second);
									smStateRestore<<"\t\t\t\tsm='"<<sm1->getName()<<"', state='"<<smState->getliteral()<<"'"<<endl;
							  }
						}
				  }
#endif
				  if(v->pInfo != NULL)
						delete v->pInfo;
				  v->pInfo = new AllStateInfo;
				  list<SymbolItem*> &SymList = ProgramState.Symbol_list;
				  map<_SM*,list<State*> > &virtualState = ProgramState.Virtual_Set;

				  //���µ�ǰ�󶨵����������״̬��Ϣ��
				  list<SymbolItem*>::iterator iter3 = SymList.begin();
				  while(iter3 != SymList.end())
				  {
						list<AttachToSymbolItem*>* pItem = (list<AttachToSymbolItem*>*)((*iter3)->getSM_State());
						list<AttachToSymbolItem*>::iterator iter_item = pItem->begin();
						list<AttachToSymbolItem*>* newList = new list<AttachToSymbolItem*>;
						while(iter_item != pItem->end())
						{
							  newList->push_back(new AttachToSymbolItem((*iter_item)));
							  ++iter_item;
						}
						((AllStateInfo*)v->pInfo)->symbolStates.insert(sym2list_value(*iter3,newList));

						++iter3;
				  }

				  //���µ�ǰ����δ�󶨵�����״̬����Ϣ
				  map<_SM*,list<State*> >::iterator iter4 = virtualState.begin();
				  while(iter4 != virtualState.end())
				  {

						((AllStateInfo*)v->pInfo)->virtualStates.insert(virtual_value((*iter4).first,(*iter4).second));
						++iter4;
				  }//while
			}//if
	  }//else
}
//2010-08-11,ZLY,END

void DataFlowAnalyzer::travelVex(VexNode* v,bool isFirstNodeOnPath,PATreeWalker& pawalker,ExprValueTreeWalker* expTreewalker,bool IsTrueEdge)
{

	  //��ÿ���ڵ�(��ÿ�����)�������趨�İ�ȫ��������顣
	  list<_SM*> &smlist = ProgramState.Sm_list;
	  for(list<_SM*>::iterator index = smlist.begin() ; index != smlist.end() ; ++index)
	  {
			_SM*	pSM = *index;		
			if(v->getData()->getAST()!=nullAST)
			{

				  if(v->getData()->getNodeType()==eReturnNode)
				  {
						RefAST r = v->getData()->getAST()->getFirstChild();
						if(r)
						{
							  pSM->Check(r,v,pawalker,expTreewalker,IsTrueEdge);
						}
				  }
				  else if(v->getData()->getNodeType()==eScopeOutNode)
				  {
						// 				cout<<"-----------------------------------------------"<<endl;
						// 				cout<<v->getData()->getAST()->toRealStringTree()<<endl;
						// 				cout<<"-----------------------------------------------"<<endl;
						break;
				  }
				  else if(v->getData()->getNodeType() == eNormalExitNode)
				  {
						break;
				  }
				  /*
				  else if(v->getData()->getNodeType() == eDeclaNode)
				  {
				  break;
				  }
				  */
				  /*
				  else if(v->getData()->getNodeType() == eSc)
				  {
				  break;
				  }
				  */
				  else
				  {
						pSM->Check(v->getData()->getAST(),v,pawalker,expTreewalker,IsTrueEdge);
				  }

			}

	  }

}
/*
class AllStateInfo
{
public:
AllStateInfo();
map<SymbolItem*,list<AttachToSymbolItem*>*> symbolStates;
map<_SM*,list<SM_State*> > virtualStates;
};
*/

/*
class  ProgramStateManager
{
public:
ProgramStateManager();
list<_SM*>					Sm_list;	    //άϵ��ǰ�Զ������ϡ�
list<SymbolItem*>			Symbol_list;	//άϵ��ǰ����㴦��Ŀ����(�ü����е���Ŀ�Ѿ�����״̬�ı仯)											������״̬�ı仯)
map<_SM*,list<SM_State*> >	Virtual_Set;	//���󶨵�������ֻ��άϵ����״												̬��ÿ���Զ�������������һ�����ϡ�
}
*/
// (������ʼǰ)���ô���������CFG�������ֵ
void DataFlowAnalyzer::setEntranceValues(const VexValueSet& v)
{
	  CFG* cfg = 0;
	  if(!m_func || !(cfg = (CFG*)(m_func->getCFG())) ) return;
	  VexNode* head = cfg->getHead();
	  int result=_CrtCheckMemory();
	  _ASSERTE(result);
	  head->inValues = v;
	  result=_CrtCheckMemory();
	  _ASSERTE(result);
}

// (����������)��ȡ������������CFG�ĳ�����ֵ
VexValueSet DataFlowAnalyzer::getExitValues()
{
	  CFG* cfg = 0;
	  if(!m_func || !(cfg = (CFG*)(m_func->getCFG())) ) return VexValueSet();
	  VexNode* ex = cfg->getEnd();
	  return ex->outValues;
}

// ��ǰ�����ĺ�����Ŀѹ�뺯������ջ
void DataFlowAnalyzer::pushCallStack()
{
	  m_callStack.push(m_func);
}

// �Ӻ�������ջ�е���ջ����Ŀ
void DataFlowAnalyzer::popCallStack()
{
	  m_callStack.pop();
}

// ��ȡ��������ջ	
/*
inline CallStack DataFlowAnalyzer::getCallStack()
{
return m_callStack;
}
*/

// �Ƿ���Ҫ�Ժ�����Ŀ���з���
bool DataFlowAnalyzer::needAnalysis()
{
	 /* cout<<"���������"<<endl;*/

	  return (!(m_callStack.find(m_func)));
}
/*һ��·��������ϣ���¼��·����ָ����Ϣ�Ͷѿռ������Ϣ*/
bool DataFlowAnalyzer::recordOnePathInfo(PATreeWalker& pawalker)
{
	SymbolItem* analyzingSbl = DataFlowAnalyzer::m_callStack.top();
	if(NULL != analyzingSbl){
		int id = analyzingSbl->getSymbolID();
		if(bddfalse != pawalker.mypa.pointto){
			pawalker.mypa.pointtoCOut |= pawalker.mypa.pointto & fdd_ithvar(M1,id);    //��·�����һ���ڵ��ID���Ӧ��ָ����Ϣ�Ͷѿռ������Ϣ
		}
		if(bddfalse != pawalker.mypa.v_heap){
			pawalker.mypa.v_heapCOut |= pawalker.mypa.v_heap & fdd_ithvar(M1,id);
		}
	}
	return true;
}

// DAY FOR CXX BEGINvisitVex(VexNode* v,PATreeWalker& pawalker ,int Paths,deque<int> dq,Path& curPath )
bool DataFlowAnalyzer::visitVex(VexNode* v, /*int curVex_id*/ VexNode* preVex , PATreeWalker& pawalker ,int Paths,/*const deque<int>& dq,*/ArcBox *arc, bool is_reachable, int pathSize, bool isInterprocedural,
	  ValueParameter& judgeType//���η���ģ����Ҫ���Ĳ���
	  )
{ 
	/*
	//�Զ���luxiao
	//�����ȾԴBEGIN

	MarkTainedVar(v);

	//�����ȾԴEND
	handleVexState(v,false,pawalker,NULL,false);
	travelVex(v,false,pawalker,NULL,false);
	//�Զ���end
	 */
	  bTainedSinkFlag4VexNode = false;
	  pawalker.analyzingNodeType=v->getNodeType();
	  bool returnvalue=true;         //add by xulei, ָʾ��ǰ·���Ƿ��ǿɴ���߲��ɴ����ֵΪfalse��ʾ���ɴ�
	  pawalker.mypa.VexMethod |= fdd_ithvar(M1, m_func->getSymbolID())&fdd_ithvar(C,v->getGlobalIndex());

		if(v->getLocalIndex()==1)//�ں�����β������ ��Ϣ�ϲ�����
		{
// 		  openConsole();
// 		  cout<<"һ��·��������ϣ���Ϣ�ռ�..."<<endl;
// 		  closeConsole();
#if  futeng_path_output
			futeng_path<<"һ��·��������ϣ���Ϣ�ռ�..."<<endl;
#endif
			//2010-08-12,ZLY,BEGIN,���һ��·��������Ϣ���ܣ���ں�����·���ɴ�ʱ������Դй©���
			if(is_reachable == true && isInterprocedural == false){  //isInterprocedural:������㱻��Ϊfalse
				  bool bFinalPath=false;
				  if(1==pathSize)
				  {
						bFinalPath=true;
				  }
				  /* #20160615
				  outputFinalError(pawalker, preVex, Paths, bFinalPath);//��鵱ǰ·����Դй©
				  */
				  outputFinalErrorByVHeap(pawalker);
			}

			/*һ��·��������ϣ���¼����ָ����Ϣ�Ͷѿռ����Ϣ���Ըú�����id��Ϊ���*/
			recordOnePathInfo(pawalker);
#if 0
			SymbolItem* analyzingSbl = DataFlowAnalyzer::m_callStack.top();
			if(NULL != analyzingSbl){
				  int id = analyzingSbl->getSymbolID();
				  if(bddfalse != pawalker.mypa.pointto){
					pawalker.mypa.pointtoCOut |= pawalker.mypa.pointto & fdd_ithvar(M1,id);
				  }
				  if(bddfalse != pawalker.mypa.v_heap){
					pawalker.mypa.v_heapCOut |= pawalker.mypa.v_heap & fdd_ithvar(M1,id);
				  }
			}
#endif
// 			if(1 == pathSize){//���һ��·�������һ�����
// 				  //function_call<<"��ջ��"<<m_func->getName()<<endl;
// 				  //getParentOutInfor_ALL(v,pawalker);   //����������·������Ϣ����
// 			}

			if(1 == pathSize && isInterprocedural == false){
// 				openConsole();
// 				cout<<"\t\t��ں������һ��·��������������Դ����..."<<endl;
// 				closeConsole();
#if futeng_path_output
				  futeng_path<<"\t\t��ں������һ��·��������������Դ����..."<<endl;
#endif
				  releaseLeakCheckResource(pawalker); //��Դ����
			}
	  }
	  else if (v->getLocalIndex()!=0)  //��β�ڵ㣬��ͷ�ڵ�
	  {
#if 0
			//futeng modify ע����20160612���ú���û��ʵ�����ã�����������pointto��߶������Ϣ
			getParentOutInfor(v,preVex,pawalker,Paths);   //�õ���һ�ڵ�ĳ���Ϣ,ftδ����
#endif
	  }
	  if (!is_reachable) //�ýڵ㲻�ɴ�
	  {
			//2010-08-12,ZLY,BEGIN,�統ǰ��㲻�ɴ�������Ϣ��Ϊ��
			pawalker.mypa.pointto = bddfalse;
			pawalker.mypa.v_heap = bddfalse;
			//2010-08-12,ZLY,END

			//pawalker.mypa.pointtoCOut|=pawalker.mypa.pointto&fdd_ithvar(C,v->getGlobalIndex())&fdd_ithvar(P,Paths)&fdd_ithvar(K,pawalker.funcStack.back()->callNum);
			//pawalker.mypa.CovertChannelAAllCout |=pawalker.mypa.CovertChannelAAll&fdd_ithvar(C,v->getGlobalIndex())&fdd_ithvar(P,Paths)&fdd_ithvar(K,pawalker.funcStack.back()->callNum);
			//pawalker.mypa.v_heapCOut|=pawalker.mypa.v_heap&fdd_ithvar(C,v->getGlobalIndex())&fdd_ithvar(P,Paths)&fdd_ithvar(K,pawalker.funcStack.back()->callNum);

			//2010-08-11,ZLY,�����Լ����ɴֱ�ӷ���false��ͬһ·�������������ɴ�
			return false;
			//return returnvalue;
			//2010-08-11,ZLY,END
	  }	
	  CFGNode* cfgnode=v->getData();
	  //xulei, 201004�� add.
	  //2010-08-12,ZLY,BEING,����������Ҫ�������������ķ��������Դ˴����ܲ���������PATreeWalker�е��������˳�������������
	  //2010-08-10,ZLY,���˵�����������
	  if (cfgnode)   
			//if (cfgnode && cfgnode->getNodeType() != eScopeOutNode)   
			//2010-08-10,ZLY:END,���˵�����������
			//2010-08-12,ZLY,END
	  {
			// 		cout<<"heap2: "<<pawalker.mypa.v_heap<<endl;
			//cfgnode->getNodeType()����1��ʾ�Ƿ�֧�ڵ㣬��Ҫ�ڷ�֧�ڵ����AST��Ϣ
			/*
			enum CFGNodeType { eNullNode = 0, ePredicateNode = 1, eStatementNode = 2, eReturnNode = 3,eDeclaNode = 4,
			eJudgeExceptionNode = 5,eNormalExitNode = 6, eExceptionExitNode = 7, eScopeOutNode = 8,eGotoNode = 9};
			*/
#if 0
			if (cfgnode->getNodeType()==ePredicateNode)   //ePredicateNode
			{
				  RefAST ePredicateNode_ast=cfgnode->getAST();    //�����ж�����AST
#ifdef PA_TRACE
				  paTrace<<"�ж�·���Ƿ�ɴ�..."<<endl;
				  paTrace<<"��ǰָ����ϢpointtoΪ��"<<endl;
				  paTrace<<fddset<<pawalker.mypa.pointto<<endl;
				  paTrace<<"ast is:"<<endl;
				  paTrace<<ePredicateNode_ast->toRealStringTree()<<endl;
#endif
#if xulei_dataflow_output
				  xulei_dataflow<<"��֧�ڵ�"<<v->getLocalIndex()<<endl<<ePredicateNode_ast->toStringTree()<<endl;
#endif
				  //xulei_dataflow<<"��һ���ڵ㣺"<<curPath[curVex_id+1]->getLocalIndex()<<endl;
				  CFGEdgeType edge_type = eTrue;  /*enum CFGEdgeType { eNull = 0, eTrue = 1, eFalse = 2,eException = 3};*/
				  if(NULL != arc){
					edge_type=arc->getEdgeType();
				  }
#if xulei_dataflow_output
				  xulei_dataflow<<"�õ�����������"<<endl;
#endif
				 /* cout<<"�õ�����������"<<endl;*/
				  if (edge_type==eFalse)   //F��֧����Ҫ��AST��һ��ת�����õ�!AST
				  {
						ePredicateNode_ast=ReConstructAST(ePredicateNode_ast, p_parser);      //�ع�AST����eת��Ϊ!e
#ifdef PA_TRACE
						paTrace<<"p_parser is :"<<p_parser<<endl;
						paTrace<<"false��֧astת�������"<<endl<<ePredicateNode_ast->toRealStringTree()<<endl;
#endif	
#if xulei_dataflow_output
						xulei_dataflow<<"����F��֧���ع�AST��"<<ePredicateNode_ast->toStringTree()<<endl;
#endif
				  }

				  /*cout<<"�����F��֧�����Ѿ��ع���"<<endl;*/

				  //��AST����һ�εȼ۱任������!!e <=> e�����
				//  visitAST(ePredicateNode_ast, p_parser);  //temp comment
#if xulei_dataflow_output
				  xulei_dataflow<<"�ȼ�ת��AST��"<<ePredicateNode_ast->toStringTree()<<endl<<endl;
#endif

				 /* cout<<"�Ѿ����˵ȼ�ת����"<<endl;*/

				  //VexValueSet* p_Value_set=&(preVex->outValues);   //������ǰһ���ڵ��������Ϣ��(Out��Ϣ)


				  //ConditionTreeWalker con_treewalker(&(pawalker.mypa), p_Value_set); //��һ����������ָ�����Ϣ���ڶ��������������͵���Ϣ
				  //// 			cout<<"Walk ast:"<<endl;
				  //// 			xulei_dataflow <<"Walk ast:"<<endl;
				  //// 			cout<<ePredicateNode_ast->toRealStringTree()<<endl;
				  //// 			xulei_dataflow<<ePredicateNode_ast->toRealStringTree()<<endl;
				  //ConstraintResultType type=con_treewalker.start(ePredicateNode_ast);
				  //switch(type)
				  //{
				  //case conflict: 
				  //	returnvalue=false;        //��ֹ��ǰ·������������һ��·����ʼ����
				  //	//returnvalue=true;       
				  //	break;
				  //case satiable:
				  //	returnvalue=true;         //��������
				  //	break;
				  //default: // case uncertain
				  //	//�����µ���Ϣ��
				  //	returnvalue=true;         //��������
				  //}
#ifdef PA_TRACE
				  paTrace<<"·���ɴ��ж������"<<returnvalue<<endl;
#endif			
			}
#endif
			// 		cout<<"heap3: "<<pawalker.mypa.v_heap<<endl;

	  }
	  else   //!cfgnode
	  { 
#ifdef PA_TRACE
			paTrace<<"cfgnode is NULL!"<<endl;
			paTrace<<"Old pointto is:"<<endl<<"\t"<<fddset<<pawalker.mypa.pointto<<endl;
#endif
			//pawalker.mypa.pointtoCOut|=pawalker.mypa.pointto&fdd_ithvar(C,v->getGlobalIndex())&fdd_ithvar(P,Paths)&fdd_ithvar(K,pawalker.funcStack.back()->callNum);
			//pawalker.mypa.CovertChannelAAllCout |=pawalker.mypa.CovertChannelAAll&fdd_ithvar(C,v->getGlobalIndex())&fdd_ithvar(P,Paths)&fdd_ithvar(K,pawalker.funcStack.back()->callNum);
			//pawalker.mypa.v_heapCOut|=pawalker.mypa.v_heap&fdd_ithvar(C,v->getGlobalIndex())&fdd_ithvar(P,Paths)&fdd_ithvar(K,pawalker.funcStack.back()->callNum);
#ifdef PA_TRACE
			paTrace<<"New pointto is:"<<endl<<"\t"<<fddset<<pawalker.mypa.pointto<<endl;
			paTrace<<"New pointtoCOut is:"<<endl<<"\t"<<fddset<<pawalker.mypa.pointtoCOut<<endl;
#endif

			return returnvalue;
	  }

	  RefAST ast=cfgnode->getAST();
	  //2010-08-11,ZLY,�����Լ����ɴֱ�ӷ���false��������ָ�����
	  //if (!ast)
	  if (!ast || returnvalue == false)
			//2010-08-11,ZLY,END
	  {
#ifdef PA_TRACE
			paTrace<<"ast is NULL!"<<endl;
			paTrace<<"Old pointto is:"<<endl<<"\t"<<fddset<<pawalker.mypa.pointto<<endl;
#endif

			//2010-08-12,ZLY,BEGIN,�統ǰ��㲻�ɴ�������Ϣ��Ϊ��
			if(returnvalue == false){
				  pawalker.mypa.pointto = bddfalse;
				  pawalker.mypa.v_heap = bddfalse;
			}
			//2010-08-12,ZLY,END

			//         cout<<"heap4: "<<pawalker.mypa.v_heap<<endl;
			//pawalker.mypa.pointtoCOut|=pawalker.mypa.pointto&fdd_ithvar(C,v->getGlobalIndex())&fdd_ithvar(P,Paths)&fdd_ithvar(K,pawalker.funcStack.back()->callNum);
			//pawalker.mypa.v_heapCOut|=pawalker.mypa.v_heap&fdd_ithvar(C,v->getGlobalIndex())&fdd_ithvar(P,Paths)&fdd_ithvar(K,pawalker.funcStack.back()->callNum);
#ifdef PA_TRACE
			paTrace<<"New pointto is:"<<endl<<"\t"<<fddset<<pawalker.mypa.pointto<<endl;
			paTrace<<"New pointtoCOut is:"<<endl<<"\t"<<fddset<<pawalker.mypa.pointtoCOut<<endl;
#endif
			//pawalker.mypa.CovertChannelAAllCout |=pawalker.mypa.CovertChannelAAll&fdd_ithvar(C,v->getGlobalIndex())&fdd_ithvar(P,Paths)&fdd_ithvar(K,pawalker.funcStack.back()->callNum);
			// #ifdef PAcout
			// 		cout<<"ast in visitVex function is null"<<endl;
			// 			paCovertChannel<<"@@vexnode123: "<<v->getGlobalIndex()<<" "<<v->getLocalIndex() <<endl;
			// 			if(v->getLocalIndex()==1)
			// 	paCovertChannel<<fddset<<" end "<<fddset<<"CovertChannelA : "<<pawalker.mypa.CovertChannelA
			// 		<<"\n<<CovertChannelAAll : "<<(pawalker.mypa.CovertChannelAAllCout&fdd_ithvar(C,v->getGlobalIndex()))
			// 		<<"\n<<CovertChannelACFG : "<<pawalker.mypa.CovertChannelACFG
			// 			<<"\n<<CovertChannelAAllCout : "<<pawalker.mypa.CovertChannelAAllCout
			// 		<<"\n--------------------------"<<endl;
			// #endif

			// 		cout<<"heap5: "<<pawalker.mypa.v_heap<<endl;
			return returnvalue;
	  }


	  //  	cout.rdbuf(pOld);
	  //  	cout<<"������䣺"<<ast->toStringTree()<<endl;
	  //  	pOld = cout.rdbuf(DzhTest.rdbuf());	 
#ifdef PA_TRACE
	  paTrace<<"After ast:"<<endl;
	  paTrace<<ast->toRealStringTree()<<endl;
	  paTrace<<"Old pointto is:"<<endl<<"\t"<<fddset<<pawalker.mypa.pointto<<endl;
	  //paTrace<<"Old pointtoArray is:"<<endl<<"\t"<<fddset<<pawalker.mypa.pointtoArray<<endl;
#endif

	 // paTrace<<"Analyzing file '"<<((RefMyAST)ast)->getFileName()<<"', line "<<((RefMyAST)ast)->getLineNum()<<endl;  //remark 2016-10-08
	  /*
	  paTrace<<"Old pointto is:"<<endl<<"\t";
	  dumpBDD_ioformat(paTrace, fddset);
	  dumpBDD(paTrace, pawalker.mypa.pointto);
	  paTrace<<endl;
	  */

	  time_t begin,end;
	  begin=clock();
	  // 	cout<<"000000000000000000000 vexnode\n"<<v->getGlobalIndex()<<endl;
	  // 	cout<<ast->toStringTree()<<endl;
	  //	paprocess<<"000000000000000000000 vexnode\n" <<ast->toStringTree()<<"\n before "<<fddset<<pawalker.mypa.pointto<<endl;
	  //pawalker.mypa.CovertChannelA=bddfalse;
	  // #ifdef CCDebug
	  // 	paCovertChannel<<fddset<<"000000000000000000000 vexnode\n" <<ast->toStringTree()<<"\n before mmmm "<<fddset<<pawalker.mypa.CovertChannelAAll<<endl;
	  // #endif
	  //	displayast(ast,0);

	  try{
			// day
			pawalker.setCurVex(v);
			pawalker.setPreVex(preVex);
			pawalker.setAnalyzer(this);
			// day end
			//cout<<"��pawalker����:"<<endl;
			// 		xulei_dataflow <<"��pawalker����:"<<endl;
			//time_t pa_begin, pa_end;
			//pa_begin=clock();	
			pawalker.start(ast,judgeType);
			// 		xulei_dataflow <<"��pawalker����:"<<endl;
			//pa_end=clock();
			//func<<"ָ�����ʱ�䣺"<<pa_end-pa_begin<<endl;
			//cout<<"��pawalker����:"<<endl;

			//by wangzhiqiang
			//time_t info_begin, info_end;
			//info_begin = clock();
			//infoTreeWalker->setDataFlowAnalyzer(this);
			//infoTreeWalker->walkAstOnVex(v, preVex);
			//info_end = clock();
			//infoRuntime<<"������Ϣ����ʱ�䣺"<<info_end-info_begin<<endl;
			//cout<<"��infoTreewalker����:"<<endl;
			//end



	  }catch (string s) {
			cout<<"  ERROR!!  "<<s<<endl;
			paprocess<<"  ERROR!!  "<<s<<endl;
#ifdef PA_TRACE
			paTrace<<"Exception occured!!!!!"<<endl;
#endif
	  }
	  //paTrace<<"After PA:start 1"<<endl;
	  //��������Ҫ����Ϣ���м�¼
	  // 	cout<<"hhh1"<<v<<" "<<v->getGlobalIndex()<<endl;
	  //     xulei_dataflow<<"hhh1"<<v<<" "<<v->getGlobalIndex()<<endl;
	  //funcStack����
	  //cout<<pawalker.funcStack.back()<<" "<<pawalker.funcStack.back()->callNum<<endl;

	  //pawalker.mypa.v_heapCOut|=pawalker.mypa.v_heap&fdd_ithvar(C,v->getGlobalIndex())&fdd_ithvar(P,Paths)&fdd_ithvar(K,pawalker.funcStack.back()->callNum);
	  //pawalker.mypa.pointtoCOut|=pawalker.mypa.pointto&fdd_ithvar(C,v->getGlobalIndex())&fdd_ithvar(P,Paths)&fdd_ithvar(K,pawalker.funcStack.back()->callNum);
	  //paTrace<<"After PA:start 2"<<endl;
#ifdef CCDebug
	  cout << "pawalker.mypa.v_heapCOut: " << pawalker.mypa.v_heapCOut << endl;
	  cout << "pawalker.mypa.pointtoCOut: " << pawalker.mypa.pointtoCOut << endl;
	  cout << "pawalker.mypa: " << pawalker.mypa.CovertChannelA << endl;

#endif
	  addInforFlow(pawalker, pawalker.mypa.CovertChannelA, v);
	  // 20100707 by kong
	  if (countListSize != ConstToRealList.size()) {
#ifdef CCDebug
			cout << "countListSize: " << countListSize << endl;
			cout << ConstToRealList.size() << ", " << v->getGlobalIndex() << endl;
#endif
			list<ConstToReal>::iterator iListID = ConstToRealList.end();

			//ConstToReal temp = *(--iListID);
			--iListID;
			(*iListID).cfgIndex = v->getGlobalIndex();
			//temp.constField1 = (*iListID).constField1;
			//temp.constVar1 = (*iListID).constVar1;
			//temp.realField1 = (*iListID).realField1;
			//temp.realVar1 = (*iListID).realVar1;
			//ConstToRealList.pop_back();

			countListSize = ConstToRealList.size();
	  }
	  //paTrace<<"After PA:start 3"<<endl;

	  //genSwithCovertChannelInfor(pawalker ,fdd_ithvar(C,v->getGlobalIndex()), dq,nodestack, v);
	  //genCovertChannelInfor( pawalker ,fdd_ithvar(C,v->getGlobalIndex()));
	  //genRecursionInfor(pawalker, fdd_ithvar(C,v->getGlobalIndex()));
	  //paTrace<<"After PA:start 4"<<endl;

	  //pawalker.mypa.CovertChannelAAllCout |=pawalker.mypa.CovertChannelAAll&fdd_ithvar(C,v->getGlobalIndex())&fdd_ithvar(P,Paths)&fdd_ithvar(K,pawalker.funcStack.back()->callNum);
	  //	pawalker.mypa.CovertChannelACFG|=pawalker.mypa.CovertChannelA&fdd_ithvar(C,v->getGlobalIndex());
	  // #ifdef PAcout
	  // 	cout<<"end of vexnode "<<fddset<<pawalker.mypa.pointto<<"\n--------------------------"<<endl;
	  // 
	  // 	paprocess<<" end "<<fddset<<pawalker.mypa.pointto<<"\npointtoArray  "<<pawalker.mypa.pointtoArray
	  // 		<<"\n refernecealise  "<<pawalker.mypa.referenceAlias
	  // 		<<"\n--------------------------"<<endl;
	  // 	paCovertChannel<<"@@vexnode: "<<v->getGlobalIndex()<<" "<<v->getLocalIndex() <<endl;
	  // 	paCovertChannel<<fddset<<" end "<<fddset<<"CovertChannelA : "<<pawalker.mypa.CovertChannelA
	  // 		<<"\n<<CovertChannelAAll : "<<(pawalker.mypa.CovertChannelAAllCout&fdd_ithvar(C,v->getGlobalIndex()))//pawalker.mypa.CovertChannelAAll
	  // 		<<"\n<<CovertChannelACFG : "<<pawalker.mypa.CovertChannelACFG
	  // 			<<"\n<<CovertChannelAAllCout : "<<pawalker.mypa.CovertChannelAAllCout
	  // 		<<"\n--------------------------"<<endl;
	  // 
	  // #endif	
	  end=clock();
	  // 	cout<<"~~~~~~~~~~~~~~~~~~~~~ vexnode"<<v->getGlobalIndex()<<" time: "<<(end-begin)/CLOCKS_PER_SEC<<"~~~~~~~~~~~~~~~~~~~~~~~~~"<<endl;

	  /*
	  paTrace<<"New pointto is:"<<endl<<"\t";
	  dumpBDD_ioformat(paTrace, fddset);
	  dumpBDD(paTrace, pawalker.mypa.pointto);
	  paTrace<<endl;
	  */

#ifdef PA_TRACE
	  paTrace<<"New pointto is:"<<endl<<"\t"<<fddset<<pawalker.mypa.pointto<<endl;
	  //paTrace<<"Old pointtoArray is:"<<endl<<"\t"<<fddset<<pawalker.mypa.pointtoArray<<endl;
	  //paTrace<<"New pointtoCOut is:"<<endl<<"\t"<<fddset<<pawalker.mypa.pointtoCOut<<endl;
#endif

	  return returnvalue;
}
// DAY FOR CXX END
// DAY FOR CXX END

/************************************************************************/
/* ��CFGPaths�Ķ���                                                     */
/************************************************************************/
//deque<string> CFGPaths::m_pathFileNames;

CFGPaths::CFGPaths(SymbolItem* f)
{
	  m_curPathCount = 0;
	  if(!f)return;
	  m_func = f;
	  CFG* cfg = (CFG*)(m_func->getCFG());		
	  walkCFG(cfg);								// ��������·��
	  //sortPaths(); ��ʹ��������Ϊԭʼ���ɵ�·���ǻ�������ģ�������п���Ӱ���֧���ı���˳��
}

int compPath(const void *pa, const void *pb)
{
	  Path* a = *((Path**)pa);
	  Path* b = *((Path**)pb);
	  int i;
	  for(i = 0; i < a->length() && i < b->length(); i++)
	  {
			if((*a)[i]->getLocalIndex() < (*b)[i]->getLocalIndex())
				  return -1;
			if((*a)[i]->getLocalIndex() > (*b)[i]->getLocalIndex())
				  return 1;
	  }
	  if(i < a->length())
			return 1;
	  if(i < b->length())
			return -1;
	  return 0;
}
bool shouldNodeBeCheck(PATreeWalker &pawalker,VexNode * pNode)
{
	if(pNode==NULL)
		return false;

	if(g_project_quit_on_sink)
	{
		if(isReachSink ==true && (!pNode->is_entry_functionNode))
		{
			return false;
		}
		if(isReachSink ==true && pNode->is_entry_functionNode)
		{
			isReachSink = false;
		}
	}
	
// 	if (pNode->getVexIsSinked())
// 	{
// 		return false;
// 	}

	if (pNode->is_specialNode_need_visited==true)  //������ʲô������ͷ���������
	{
		return true;
	}
	if(pNode->is_entry_functionNode==false)
	{
		//����ı�ָ�룬���ڷ���ں�����˵��һ����Ҫ����
		if( pNode->b_change_pointer == true)
			return true;

		bool bCheck = true;
		//�ж���Ⱦ����
		if( taint_function_check_on )
		{
			if( pawalker.mypa.taintSource ==bddfalse /*&& pawalker.mypa.taintInfo==bddfalse*/ &&pawalker.mypa.isTaintedSource==bddfalse)
			{
				if( pNode->b_taint_source == false )
					bCheck = false;
			}else{
				if( pNode->b_taint_change == false && pNode->b_taint_source == false && pNode->b_taint_sink == false)
					bCheck = false;
			}

			//�����Ⱦ����˵��������һ��Ҫ����
			if( bCheck == true)
				return true;
		}

		bCheck = true;
		//�ж�������Ϣ
		if( sensitive_function_check_on )
		{
			if( pawalker.mypa.sensitiveSource ==bddfalse && pawalker.mypa.sensitiveInfo==bddfalse &&pawalker.mypa.isSensitivedSource==bddfalse )
			{
				if( pNode->b_sensitive_source == false )
					bCheck = false;
			}else{
				if( pNode->b_sensitive_change == false && pNode->b_sensitive_source == false && pNode->b_sensitive_sink == false)
					bCheck = false;
			}

			//���������Ϣ˵��������һ��Ҫ����
			if( bCheck == true)
				return true;
		}

		//����ڴ�©��˵��������һ��Ҫ����
		if( memory_function_check_on && pNode->b_memory_sink )
			return true;

		//���Σ�պ���˵��������һ��Ҫ����
		if( danger_function_check_on && pNode->b_danger_sink )
			return true;
		//��Ҷ�˵���÷��������ڿ��Բ�������
		return false;

	}
	else
	{
		if( taint_function_check_on && pNode->b_taint_sink)
		{
			return true;
		}
		if( sensitive_function_check_on && pNode->b_sensitive_sink)
		{
			return true;
		}
		//����ڴ�©��˵��������һ��Ҫ����
		if( memory_function_check_on && pNode->b_memory_sink )
			return true;

		//���Σ�պ���˵��������һ��Ҫ����
		if( danger_function_check_on && pNode->b_danger_sink )
			return true;
		//��Ҷ�˵���÷��������ڿ��Բ�������
		return false;
	}
	


}
void CFGPaths::sortPaths()
{
	  deque<Path> tmpDequePaths;
	  paTrace<<"Before sort, paths are:"<<endl<<*this<<endl;
	  Path ** arrPaths = (Path **)malloc(sizeof(Path*)*m_paths.size());
	  deque<Path>::iterator itPaths;
	  int iSize;
	  for(itPaths = m_paths.begin(),iSize=0; itPaths != m_paths.end(); itPaths++,iSize++){
			tmpDequePaths.push_back(*itPaths);
			arrPaths[iSize] = &(tmpDequePaths.at(iSize));
	  }
	  qsort((void *)arrPaths, iSize, sizeof(Path*), compPath);
	  m_paths.clear();
	  for(int i=0; i<iSize; i++)
			m_paths.push_back(*(arrPaths[i]));
	  tmpDequePaths.clear();
	  free(arrPaths);
	  paTrace<<"After sort, paths are:"<<endl<<*this<<endl;
	  //listPaths.sort()
}

CFGPaths& CFGPaths::operator = (const CFGPaths& ps)
{
	  m_func = ps.m_func;
	  m_paths = ps.m_paths;
	  m_curPathCount = 0;
	  return *this;
}

//ZLY, 2010-9-1, BEGIN, ͳ��·��Ҫ�������н����Ҫ���ǵĽڵ���
//�������еĽ�㲻�����������֧������2������������1
int CFGPaths::CalculateCoverCount(CFG *cfg)
{
	  if(cfg == NULL)
			return 0;
	  VexNode* v = cfg->getHead();	// ��ʾ��ǰҪ���ʵ�CFG�ڵ�, ��ʼΪͷ�ڵ�	
	  map<VexNode*, bool> mapVexVisited;
	  queue<VexNode*> bfsQueue;
	  int iCount = 1;
	  bfsQueue.push(v);
	  mapVexVisited[v] = true;
	  //paTrace<<"Should cover: "<<v->getLocalIndex()<<endl;
	  while(bfsQueue.size() > 0){
			VexNode * curVex = bfsQueue.front();
			VexNode * nextV;
			bfsQueue.pop();
			if(curVex->getOutNum() == 1){
				  nextV = curVex->getFirstOutArc()->getTailVex();
				  if(mapVexVisited.find(nextV) == mapVexVisited.end()){
						if( !(nextV->getOutNum()==1 && nextV->getBranchCircle()==onNull)		// 1. ���˳�ŵ�ǰ�ڵ������±�����������ѭ��...
							  && !(nextV->getOutNum()==2 && nextV->getBranchCircle()==onBoth) ){
									if(nextV->getOutNum() == 2){
										  iCount += 2;
										  //paTrace<<"Should cover(2): "<<nextV->getLocalIndex()<<endl;
									}else{
										  iCount++;
										  //paTrace<<"Should cover: "<<nextV->getLocalIndex()<<endl;
									}
						}
						bfsQueue.push(nextV);
						mapVexVisited[nextV] = true;
				  }
			}else if(curVex->getOutNum() == 2){//��֧���
				  nextV = getTailVexNode(curVex, true);
				  if(mapVexVisited.find(nextV) == mapVexVisited.end()){
						if( !(nextV->getOutNum()==1 && nextV->getBranchCircle()==onNull)		// 1. ���˳�ŵ�ǰ�ڵ������±�����������ѭ��...
							  && !(nextV->getOutNum()==2 && nextV->getBranchCircle()==onBoth) ){
									if(nextV->getOutNum() == 2){
										  iCount += 2;
										  //paTrace<<"Should cover(2): "<<nextV->getLocalIndex()<<endl;
									}else{
										  iCount++;
										  //paTrace<<"Should cover: "<<nextV->getLocalIndex()<<endl;
									}
						}
						bfsQueue.push(nextV);
						mapVexVisited[nextV] = true;
				  }
				  nextV = getTailVexNode(curVex, false);
				  if(mapVexVisited.find(nextV) == mapVexVisited.end()){
						if( !(nextV->getOutNum()==1 && nextV->getBranchCircle()==onNull)		// 1. ���˳�ŵ�ǰ�ڵ������±�����������ѭ��...
							  && !(nextV->getOutNum()==2 && nextV->getBranchCircle()==onBoth) ){
									if(nextV->getOutNum() == 2){
										  iCount += 2;
										  //paTrace<<"Should cover(2): "<<nextV->getLocalIndex()<<endl;
									}else{
										  iCount++;
										  //paTrace<<"Should cover: "<<nextV->getLocalIndex()<<endl;
									}
						}
						bfsQueue.push(nextV);
						mapVexVisited[nextV] = true;
				  }
			}
	  }
	  return iCount;

}
//ZLY, 2010-9-1, END, ͳ��·��Ҫ�������н����Ҫ���ǵĽڵ���

void CFGPaths::walkCFG(CFG* cfg)
{
	  if(!cfg)						// ���CFG������, �ͷ���	
			return;
	  //dataflow<<__FILE__<<":"<<__LINE__<<":walkCFG step 1"<<endl;
	  markDeadCircle(cfg);			// ���CFG���Ƿ��������
	  BranchVexStack branch;			// ������¼��֧�ڵ��ջ, ������
	  Path path;						// ��ǰ����������һ��·��
	  VexNode* v = cfg->getHead();	// ��ʾ��ǰҪ���ʵ�CFG�ڵ�, ��ʼΪͷ�ڵ�	

	  //ZLY, 2010-9-1, BEGIN, ��¼·�����ǵĽڵ���
	  int iTotalVex = CalculateCoverCount(cfg);
	  int iCoverVexCount = 0;
	  map<VexNode*, int> mapVexCovered;
	  //ZLY, 2010-9-1, END, ��¼·�����ǵĽڵ���

	  //dataflow<<__FILE__<<":"<<__LINE__<<":walkCFG step 2"<<endl;

	  while((v && v->getLocalIndex()!=1) || !branch.empty() )			// ��ǰ�ڵ㲻��β�ڵ�, ����ջ�ǿ�, ѭ��
	  {		
			//dataflow<<"Cur vex is"<<v->getLocalIndex()<<", getOutNum is "<<v->getOutNum() <<", getBranchCircle is "<<v->getBranchCircle()<<endl;
			//if(v->getData()->getAST() != nullAST)
			//	dataflow<<v->getData()->getAST()->toRealStringTree()<<endl;
			//dataflow<<__FILE__<<":"<<__LINE__<<":walkCFG step 3"<<endl;
			if( (v->getOutNum()==1 && v->getBranchCircle()==onNull)		// 1. ���˳�ŵ�ǰ�ڵ������±�����������ѭ��...
				  || (v->getOutNum()==2 && v->getBranchCircle()==onBoth) )
			{

				  dataflow<<"DEAD CIRCLE FOUND"<<endl;
				  VexNode* breaker = branch.top();	// �������ѭ���ķ�֧�ڵ�
				  branch.pop();
				  if(!breaker)						// �����֧ջ�Ѿ���, ������ѭ���޷��˳�
				  {
						dataflow<<"DEAD CIRCLE"<<endl;
						return;
				  }
				  dataflow<<"Backtrace from dead circle, pop "
						<<breaker->getLocalIndex()<<" -> ";
				  path.backTrace(breaker);					// ·������...
				  bool prePath = breaker->getBranchPath();	// ������ѭ��ʱ, ��֧�ڵ����ߵķ�֧
				  if(prePath)
				  {
						breaker->setBranchCircle(onTrue);		// ���breaker��T��֧��������ѭ��
						breaker->setBranchPath(false);
						v = getTailVexNode(breaker, false);		// ����һ��֧����
				  }
				  else
				  {
						breaker->setBranchCircle(onFalse);		// ���breaker��F��֧��������ѭ��
						breaker->setBranchPath(true);
						v = getTailVexNode(breaker, true);		// ����һ��֧����
				  } 
				  path.markCoverVex(breaker, iCoverVexCount, mapVexCovered);
				  continue;
			}
			//dataflow<<__FILE__<<":"<<__LINE__<<":walkCFG step 4"<<endl;
			if(v->getOutNum()==2 && v->getBranchCircle()==onTrue)
			{								// ��֧�ڵ��T�߽������֧, ������branchPath, ����F�� 
				  path.pushVex(v, iCoverVexCount, mapVexCovered);
				  v->setBranchPath(false);
				  v = getTailVexNode(v, false);
				  continue;
			}
			//dataflow<<__FILE__<<":"<<__LINE__<<":walkCFG step 5"<<endl;
			if(v->getOutNum()==2 && v->getBranchCircle()==onFalse)
			{								// ��֧�ڵ��F�߽������֧, ������branchPath, ����T��
				  path.pushVex(v, iCoverVexCount, mapVexCovered);
				  v->setBranchPath(true);
				  v = getTailVexNode(v, true);
				  continue;
			}

			//dataflow<<__FILE__<<":"<<__LINE__<<":walkCFG step 6"<<endl;
			if(v->getOutNum() ==2 && branch.find(v) )	// 2. ����ڵ�v�Ƿ�֧�ڵ�����ջ��, ��������һ����
			{
				  //dataflow<<"Walk to it's other branch..."<<endl;
				  path.pushVex(v, iCoverVexCount, mapVexCovered);  
				  bool prePath = v->getBranchPath();
				  //2010-08-28,ZLY,BEGIN, ������һ����֧��Ҫ����BranchPath
				  v->setBranchPath(!prePath);
				  //2010-08-28,ZLY,END
				  v = getTailVexNode(v, !prePath);
				  continue;
			}
			//dataflow<<__FILE__<<":"<<__LINE__<<":walkCFG step 7"<<endl;

			if( v->getOutNum() == 2 )		// 3. �ڵ�v�Ƿ�֧�ڵ�, �Ҳ��ڷ�֧ջ��
			{ 
				  if(!path.findVex(v))	// �ڵ㲻��·��������, ����ѹջ. 
						branch.push(v);		// ����ڵ��ڶ�����, �����ýڵ�֮ǰ����ջ, ��ʱ��ͨ��ѭ���ص��ýڵ�, 
				  // ��˲���ѹջ, �����㷨��������ѭ��
				  path.pushVex(v, iCoverVexCount, mapVexCovered);

				  bool prePath = v->getBranchPath();
				  v->setBranchPath(!prePath);
				  v = getTailVexNode(v, !prePath);

				  continue;
			}

			//dataflow<<__FILE__<<":"<<__LINE__<<":walkCFG step 8"<<endl;
			if( v->getLocalIndex() != 1)		// 5. v���Ƿ�֧�ڵ�, �Ҳ���β�ڵ�, ��������һ�ڵ�...
			{
				  path.pushVex(v, iCoverVexCount, mapVexCovered);
				  v = v->getFirstOutArc()->getTailVex();
				  continue;
			}

			//dataflow<<__FILE__<<":"<<__LINE__<<":walkCFG step 9"<<endl;
			if(!branch.empty())					// 6. v��β�ڵ�, ��ջ����, ��ջ, ��������һ����
			{
				  path.pushVex(v, iCoverVexCount, mapVexCovered);
				  m_paths.push_back(path);

				  //ZLY, 2010-9-1, BEGIN,�Դ���·���������Ż�
				  switch(giOptimalPaths){
				  case PATH_OPT_ONLY_COVER:
						if(iCoverVexCount >= iTotalVex){
							  cout.rdbuf(pOld);
							  cout<<"    ������ʹ�ý�㸲�ǵ�·���Ż�, ������"<<m_paths.size()<<"��·��!"<<endl;
							  pOld = cout.rdbuf(DzhTest.rdbuf());	 
							  paTrace<<"    ������ʹ�ý�㸲�ǵ�·���Ż�, ������"<<m_paths.size()<<"��·��!"<<endl;
							  return;
						}
						break;
				  case PATH_OPT_COVER_AND_COUNT:
						if(m_paths.size() >= gi_PATH_COUNT_OPT_THRESHOLD){
							  //paTrace<<"iCoverVexCount = "<<iCoverVexCount<<", iTotalVex = "<<iTotalVex<<endl;
							  if(iCoverVexCount >= iTotalVex){
									cout.rdbuf(pOld);
									cout<<"    ������·������ֵ�Ľ�㸲��·���Ż�, ������"<<m_paths.size()<<"��·��!"<<endl;
									pOld = cout.rdbuf(DzhTest.rdbuf());	 
									paTrace<<"    ������·������ֵ�Ľ�㸲��·���Ż�, ������"<<m_paths.size()<<"��·��!"<<endl;
									return;
							  }
						}
						break;
				  case PATH_OPT_ONLY_COUNT:
						if(m_paths.size() >= gi_PATH_COUNT_OPT_THRESHOLD){
							  cout.rdbuf(pOld);
							  cout<<"    ������ʹ��·������ֵ��·���Ż�, ������"<<m_paths.size()<<"��·��!"<<endl;
							  pOld = cout.rdbuf(DzhTest.rdbuf());	 
							  paTrace<<"    ������ʹ��·������ֵ��·���Ż�, ������"<<m_paths.size()<<"��·��!"<<endl;
							  return;
						}
						break;
				  case PATH_OPT_NO_OPT:
				  default:
						if(m_paths.size() >= MAX_PATH_COUNT){
							  cout.rdbuf(pOld);
							  cout<<"    �����е�·����̫��, ������"<<m_paths.size()<<"��·��!"<<endl;
							  pOld = cout.rdbuf(DzhTest.rdbuf());	 
							  paTrace<<"    �����е�·����̫��, ������"<<m_paths.size()<<"��·��!"<<endl;
							  return;
						}
						break;
				  }
				  //ZLY, 2010-9-1, END,�Դ���·���������Ż�


				  //dataflow<<"Begin a new path..."<<endl;
				  v = branch.top(); branch.pop();
				  //paTrace<<"Pop branch:"<<v->getLocalIndex()<<endl;
				  path.backTrace(v);
				  path.markCoverVex(v, iCoverVexCount, mapVexCovered);


				  bool prePath = v->getBranchPath();
				  v->setBranchPath(!prePath);
				  v = getTailVexNode(v, !prePath);
				  continue;
			}
			//dataflow<<__FILE__<<":"<<__LINE__<<":walkCFG step 10"<<endl;
	  }
	  if(v)
			path.pushVex(v, iCoverVexCount, mapVexCovered);
	  //dataflow<<__FILE__<<":"<<__LINE__<<":walkCFG step 11"<<endl;
	  m_paths.push_back(path);
	  //dataflow<<__FILE__<<":"<<__LINE__<<":walkCFG step 12"<<endl;
}


ostream& operator<<(ostream& o, const CFGPaths& paths)
{
	  int i = 0; int sz = paths.size();
	  while(i<sz)
	  {
			const Path& path = paths.m_paths[i];
			o<<path<<endl;
			i++;
	  }
	  return o;
}

// ��ȡһ��·��
bool CFGPaths::getNextPath(Path& path, int& cnt)
{
	  if(m_curPathCount>=m_paths.size())return false;
	  cnt = m_curPathCount;
	  path = m_paths[m_curPathCount++];
	  return true;
}

// ·��������
int CFGPaths::size()const
{
	  return m_paths.size();
}

bool CFGPaths::empty()const
{
	  return m_paths.empty();
}

inline void CFGPaths::reset()
{
	  m_curPathCount = 0;
}

Path* CFGPaths::getPath(int cnt) { // ��õ�cnt��·������cnt=0��ʼ���·�� by kong 20100111
	  if (cnt >= m_paths.size() || cnt < 0)
			return NULL;
	  return &(m_paths[cnt]);
}


Path* CFGPaths::operator[](int cnt)
{
	  if(cnt<0 || cnt>=m_paths.size()) return 0;
	  return &(m_paths[cnt]);
}

void CFGPaths::markDeadCircle(CFG* cfg)
{
	  //dataflow<<__FILE__<<":"<<__LINE__<<":markDeadCircle step"<<endl;
	  if(!cfg)return;
	  map<VexNode*, set<VexNode*> > vexTraces;
	  VexNode* vHead = cfg->getHead();
	  VexNode* vEnd = cfg->getEnd();
	  VexNode* v=vHead;
	  deque<VexNode*> vexes;

	  //dataflow<<__FILE__<<":"<<__LINE__<<":markDeadCircle step"<<endl;
	  while(v)	
	  {
			//dataflow<<__FILE__<<":"<<__LINE__<<":markDeadCircle step"<<endl;
			//2010-08-30,ZLY,���ڵ��һ�η��ʣ�����ӿյ�set
			if(vexTraces.find(v) == vexTraces.end()){
				  //dataflow<<__FILE__<<":"<<__LINE__<<":markDeadCircle add new set"<<endl;
				  set<VexNode*> tmpSet;
				  //vexTraces[v] = tmpSet;
				  ////dataflow<<__FILE__<<":"<<__LINE__<<":markDeadCircle"<<endl;
				  //dataflow<<__FILE__<<":"<<__LINE__<<":markDeadCircle v:"<<v<<endl;
				  typedef map<VexNode*, set<VexNode*> >::value_type newValType;
				  vexTraces.insert(newValType(v, tmpSet));
			}
			//2010-08-30,ZLY,END
			//dataflow<<__FILE__<<":"<<__LINE__<<":markDeadCircle step"<<endl;
			vexTraces[v].insert(v);
			//dataflow<<__FILE__<<":"<<__LINE__<<":markDeadCircle step"<<endl;
			if(v!=vHead && v!=vEnd) vexes.push_front(v);
			//dataflow<<__FILE__<<":"<<__LINE__<<":markDeadCircle step"<<endl;
			v=v->getNextVexNodeInList();
			//dataflow<<__FILE__<<":"<<__LINE__<<":markDeadCircle step"<<endl;
	  }

	  //dataflow<<__FILE__<<":"<<__LINE__<<":markDeadCircle step"<<endl;
	  vexes.push_front(vEnd);
	  vexes.push_back(vHead);

	  bool change=true;
	  while(change)
	  {
			change = false;
			deque<VexNode*>::iterator it = vexes.begin();	
			while(it!=vexes.end())
			{
				  v = *it;
				  for(ArcBox* arc = v->getFirstOutArc(); arc; arc=arc->getNextEdgeWithSameHeadVex())
						// ÿ���ڵ���Ե����亢�ӽڵ���Ե���Ľڵ�
				  {
						VexNode* vNext = arc->getTailVex();
						int sizeBefore = vexTraces[v].size();
						set<VexNode*>& vSet = vexTraces[vNext];
						for(set<VexNode*>::iterator it = vSet.begin(); it!=vSet.end(); it++)
							  vexTraces[v].insert(*it);
						int sizeAfter = vexTraces[v].size();
						if(sizeAfter!=sizeBefore) change = true;
				  }
				  it++;
			}
	  }
	  //dataflow<<__FILE__<<":"<<__LINE__<<":markDeadCircle step"<<endl;

	  //ZLY 2010-8-19 BEGIN: report dead-loop
	  // ���˵������������ѭ�����������ѭ���У��������������Ľ��������� ��ѭ����־��
	  // ��ˣ���ʱ��ȡ�ķ����������ģ�
	  // ѭ�����ۼ�������ѭ����־�Ľ���е�����кţ���ѭ����ʹ�ø��кš�
	  // ���Ǵ���һ�����⣺
	  // ֻ�ܱ��������������У��������������ѭ�����ұ�����Ǹ�ѭ�������һ�������к�
	  string myFileName;
	  long   lineOfDeadLoop = -1;
	  //ZLY 2010-8-19 END: report dead-loop

	  for(map<VexNode*, set<VexNode*> >::iterator mit = vexTraces.begin(); mit!=vexTraces.end(); mit++)
	  {
			set<VexNode*>& vSet = mit->second;
			if(vSet.find(vEnd)!=vSet.end())
				  continue;
			VexNode* v = mit->first;

			//ZLY 2010-8-19 BEGIN: report dead-loop
			/* ZLY 2010-8-19 ����ԭ���Ĵ���
			* if(v->getOutNum()==1)v->setBranchCircle(onNull);
			* else if(v->getOutNum()==2)v->setBranchCircle(onBoth);
			*/

			if(v->getOutNum()==1) {
				  v->setBranchCircle(onNull);
			}
			else if(v->getOutNum()==2){
				  v->setBranchCircle(onBoth);
			}
			switch( v->getOutNum() ){
			case 1: case 2:
				  if( v->getData() != NULL ){
						RefMyAST myAst = static_cast<RefMyAST>(v->getData()->getAST());
						if( myAst != nullAST ){
							  if(myAst->getFileName() != ""){
									myFileName = myAst->getFileName();
									int  newLine = myAst->getLineNum();
									lineOfDeadLoop = ( lineOfDeadLoop < newLine ) ? newLine : lineOfDeadLoop;
							  }
						}
				  }
				  break;
			}
			//ZLY 2010-8-19 END: report dead-loop
	  }
	  //dataflow<<__FILE__<<":"<<__LINE__<<":markDeadCircle step"<<endl;

	  //ZLY 2010-8-19 BEGIN: report dead-loop
	  if( lineOfDeadLoop > 0 && myFileName != ""){
			extern ofstream buffer_warning;
			buffer_warning << myFileName << "\t" << lineOfDeadLoop << "\t" 
				  << "�������\t�������\t������ѭ��"  << endl;
	  }
	  //ZLY 2010-8-19 END: report dead-loop
	  //dataflow<<__FILE__<<":"<<__LINE__<<":markDeadCircle step"<<endl;
}



/************************************************************************/
/* BranchVexStack�еķ�������                                           */
/************************************************************************/
inline void CFGPaths::BranchVexStack::push(VexNode* v)
{ 
	  stk.push_front(v); 
}

inline void CFGPaths::BranchVexStack::pop()
{ 
	  if(!stk.empty()) stk.pop_front(); 
}

VexNode* CFGPaths::BranchVexStack::top()
{ 
	  if(!stk.empty()) 
	  {
			list<VexNode*>::iterator it = stk.begin();
			return *it;
	  }
	  return 0;
}

inline	bool CFGPaths::BranchVexStack::empty()
{ 
	  return stk.empty();
}

int CFGPaths::BranchVexStack::find(VexNode* vex)
{
	  list<VexNode*>::iterator it;
	  for(it = stk.begin(); it != stk.end(); it++)
	  {
			if(vex == *it)return 1;
	  }
	  return 0; 
}

void CFGPaths::BranchVexStack::display()
{
	  list<VexNode*>::reverse_iterator it;
	  for(it = stk.rbegin(); it!=stk.rend(); it++)
	  {
			VexNode* v = *it;
			dataflow<<v->getLocalIndex()<<" -> ";
	  }
}




//////////////////////////////////////////////////////////////////////////
// ��CFGPathsManager�ķ�������
//////////////////////////////////////////////////////////////////////////
CFGPathsManager* CFGPathsManager::m_instance = 0;

CFGPathsManager* CFGPathsManager::instance()
{
	  if(!m_instance)
			m_instance = new CFGPathsManager;
	  return m_instance;
}

CFGPaths& CFGPathsManager::pathsOfFunction(SymbolItem* func)
{
	  if(m_mapPaths[func].empty())
			m_mapPaths[func] = CFGPaths(func);
	  m_mapPaths[func].reset();
	  return m_mapPaths[func];
}



/************************************************************************/
/* ��Path�ķ�������                                                     */
/************************************************************************/
void Path::markCoverVex(VexNode* v, int& iVexCount, map<VexNode*, int>& mapVexCovered)
{	
	  if(v->getOutNum() == 2){
			if(mapVexCovered.find(v) == mapVexCovered.end()){
				  iVexCount++;
				  mapVexCovered[v] = 1;
				  //paTrace<<"new Cover "<<v->getLocalIndex()<<endl;
			}else if(mapVexCovered[v] == 1){
				  iVexCount++;
				  mapVexCovered[v] = 2;
				  //paTrace<<"new Cover(2) "<<v->getLocalIndex()<<endl;
			}
	  }else{
			if(mapVexCovered.find(v) == mapVexCovered.end()){
				  iVexCount++;
				  mapVexCovered[v] = 1;
				  //paTrace<<"new Cover "<<v->getLocalIndex()<<endl;
			}
	  }
}

void Path::pushVex(VexNode* v, int& iVexCount, map<VexNode*, int>& mapVexCovered)
{	
	  m_path.push_back(v);	
	  //2010-08-11,ZLY,BGING,��ʼ���������λ
	  m_AnalyzedFlag.push_back(NotAnalyzed);
	  //2010-08-11,ZLY,END
	  markCoverVex(v, iVexCount, mapVexCovered);
}

bool Path::findVex(VexNode* v)
{
	  return (find(m_path.begin(), m_path.end(), v) != m_path.end());
}

void Path::backTrace(VexNode* v)
{
	  deque<VexNode*>::iterator it = find(m_path.begin(), m_path.end(), v);
	  if(it!=m_path.end()){
			m_path.erase(++it, m_path.end());
			//2010-08-11,ZLY,BGING,��ʼ���������λ
			size_t m_path_size = m_path.size();
			size_t m_flag_size = m_AnalyzedFlag.size();
			if(m_flag_size > m_path_size){
				  for(; m_flag_size > m_path_size; m_flag_size--){
						m_AnalyzedFlag.pop_back();
				  }
			}
			//2010-08-11,ZLY,END
	  }
}

VexNode* Path::operator[](int cnt)
{
	  if(cnt<0 || cnt>=m_path.size())
			return 0;
	  return m_path[cnt];
}

int Path::firstDifferentVex(Path& p)const
{
	  if(p.length()<=0 || this->length()<=0) return 0;
	  int cnt = 0;

	  //WXQ, 2010-08-30
	  while(true)
	  {
			if(cnt>=this->length() || cnt>=p.length())
				  return cnt;
			if(m_path[cnt] != p[cnt])
				  break;
			cnt++;
	  }


	  VexNode* v = m_path[cnt-1];
	  if(v && v->getNodeType()==vDoWhile && getTailVexNode(v, false) == m_path[cnt])
	  {
			VexNode* difVex = getTailVexNode(v, true);
			int x = 0;
			while(m_path[x] != difVex )x++;
			return x;
	  }
	  return cnt;
}

bool Path::vexIsInCircle(int cnt)
{
	  calculateCircles();
	  deque<SubPath>::iterator it;
	  for(it=m_circles.begin(); it!=m_circles.end(); it++)
	  {
			int head = it->head; int tail = it->tail;
			if(cnt<=tail && cnt>=head) return true;
	  }
	  return false;
}

bool Path::vexIsCircleHead(VexNode* vex)
{
	  calculateCircles();
	  deque<SubPath>::iterator it;
	  for(it=m_circles.begin(); it!=m_circles.end(); it++)
			if(vex == m_path[it->head]) return true;
	  return false;
}

bool Path::vexIsCircleHead(int cnt)
{
	  calculateCircles();
	  deque<SubPath>::iterator it;
	  for(it=m_circles.begin(); it!=m_circles.end(); it++)
			if(cnt == it->head) return true;	
	  return false;	
}

void Path::calculateCircles()
{
	  if(m_circlesCalculated) return;
	  m_circles.clear();
	  for(int i = 0; i<m_path.size(); i++)
	  {	
			if(m_path[i]->getNodeType() == vCircle || m_path[i]->getNodeType()==vDoWhile)
				  for(int j = m_path.size()-1; j>i; j--)
				  {
						if(m_path[i]==m_path[j])
						{
							  SubPath circle = {i,j};
							  m_circles.push_back(circle);
							  break;
						}
				  }
	  }
	  m_circlesCalculated = true;
}

void Path::calculateCompounds()
{
	  if(m_compoundsCalculated) return;
	  calculateCircles();
	  stack<int> stk;
	  int i = 0;
	  m_compounds = m_circles;
	  while(i<m_path.size())
	  {
			VexNode* curVex = m_path[i];
			int outNum = curVex->getOutNum(); int inNum = curVex->getInNum();
			ArcBox* arc = curVex->getFirstInArc();
			while(arc)
			{
				  VexNode* preVex = arc->getHeadVex();
				  if(preVex->getRank() >= curVex->getRank()
						&& curVex->getData()->getNodeType()!=eNormalExitNode)
						inNum--;
				  arc = arc->getNextEdgeWithSameTailVex();
			}

			VexNodeType nodeType = curVex->getNodeType();
			if(inNum > 1 && !stk.empty() )
			{
				  SubPath s = {stk.top(), i};
				  m_compounds.push_back(s);	stk.pop();	
			}
			if(outNum == 1 || outNum == 0)
			{
				  i++; continue;
			}
			if( nodeType != vCircle && nodeType != vDoWhile )
				  stk.push(i);
			i++;
	  }
	  m_compoundsCalculated = true;
}

int Path::length()const
{
	  return m_path.size();
}		

bool Path::empty()const
{
	  return m_path.empty();
}
deque<int> Path::getSwitches(int curIndex)
{
	  calculateCompounds();
	  deque<int> r;
	  int i = 0; int sz = m_compounds.size();
	  while(i<sz)//cxx edit
	  {
			if(curIndex>m_compounds[i].head && curIndex<m_compounds[i].tail)
				  r.push_back(m_compounds[i].head);
			i++;
	  }
	  return r;
}
ostream& operator<<(ostream& o, const Path& p)
{
	  int cnt = 0;
	  while(cnt<p.length())
	  {
			VexNode* v = p.m_path[cnt];
			//o<<v->getLocalIndex()<<"/";
			if(v->getLocalIndex() == 0)
				  o<<"Start";
			else if(v->getLocalIndex() == 1)
				  o<<"End";
			else if(v->getData()->getNodeType() == eNormalExitNode)
				  o<<"Exit";
			else if(v->getData()->getNodeType() == eScopeOutNode)
				  o<<"}";
			else if(v->getData()->getAST() != nullAST)
				  o<<((RefMyAST)(v->getData()->getAST()))->getLineNum();
			else
				  o<<"_"<<v->getLocalIndex();

			//o<<"/"<<v->rank;//���rank����

			if(cnt+1<p.length())o<<"->";
			cnt++;
	  }
	  return o;
}


void Path::displayCircle()
{
	  calculateCircles();
	  int i = 0;
	  while(i<m_circles.size())
	  {
			SubPath c = m_circles[i++];
			dataflow<<"( "<<c.head<<" , "<<c.tail<<" )\t";
	  }
	  dataflow<<endl;
}

void Path::displayCompounds()
{
	  calculateCompounds();
	  int i = 0;
	  while(i<m_compounds.size())
	  {
			SubPath c = m_compounds[i++];
			dataflow<<"( "<<c.head<<" , "<<c.tail<<" )\t";
	  }
	  dataflow<<endl;
}
// TEST END



/************************************************************************/
/* ��CallStack�ķ�������                                                */
/************************************************************************/
void CallStack::push(SymbolItem* s)
{
	  string indent = "",funcs = "";
	  for(list<SymbolItem*>::iterator it = callStack.begin();it != callStack.end();it++)
	  {
			indent += "  ";
			funcs +=("/" + (*it)->getName());
	  }
	  openConsole();
	  cout<<indent<<">>"<<funcs+"/"+s->getName()<<endl;
	  closeConsole();
	  callStack.push_back(s);
	  list<int> newPath;
	  curPaths.push_back(newPath);
}

void CallStack::pop()
{
	  if(!callStack.empty()){
			string indent = "",funcs = "";
			for(list<SymbolItem*>::iterator it = callStack.begin();it != callStack.end();it++)
			{
				  if(it != callStack.begin())
						indent += "  ";
				  funcs +=("/" + (*it)->getName());
			}
			openConsole();
			cout<<indent<<"<<"<<funcs<<endl;
			closeConsole();

			callStack.pop_back();
			curPaths.pop_back();
	  }
}

bool CallStack::find(SymbolItem* s)
{
	  list<SymbolItem*>::iterator it;
	  for(it = callStack.begin(); it!=callStack.end(); it++)
			if(s == *it) return true;
	  return false;
}

list<int> * CallStack::getCurPathInfo()
{
	  if(curPaths.size() < 1)
			return NULL;
	  return &(curPaths.back());
}

string CallStack::getAllFunctionInStack()
{
	  ostringstream ossTmp;
	  list<SymbolItem*>::iterator it;
	  for(it = callStack.begin(); it!=callStack.end(); it++){
			ossTmp<<(*it)<<";";
			//ossTmp<<(*it)->getName()<<";";
	  }
	  return ossTmp.str();
}

string CallStack::getAllPathsInStack()
{
	  ostringstream ossTmp;
	  list< list<int> >::iterator itPaths;
	  list<int>::iterator itPath;
	  for(itPaths = curPaths.begin(); itPaths!=curPaths.end(); itPaths++){
			list<int>& onePath = *itPaths;
			int i = 0;
			for(itPath = onePath.begin(); itPath!=onePath.end(); itPath++){
				  if(i>0)
						ossTmp<<",";
				  i++;
				  ossTmp<<(*itPath);
			}
			ossTmp<<";";
	  }
	  return ossTmp.str();
}

ostream& operator<<(ostream& o, const CallStack& c)
{
	  o<<"Call Stack: ";
	  list<SymbolItem*>::const_iterator it;
	  for(it = c.callStack.begin(); it!=c.callStack.end(); it++)
			o<<(*it)->getName()<<"->";
	  o<<endl;
	  return o;
}

// bdd bone = bdd_fullsatone(b1); // ȷ��ֻȡһ��Ԫ��
// int x = fdd_scanvar(bone, 0);
// int y = fdd_scanvar(bone, 1);
// bone = fdd_ithvar(0, x) & fdd_ithvar(1, y);

// by kong 20100520
void DataFlowAnalyzer::addInforFlow(PATreeWalker& pawalker, bdd curBdd, VexNode* v) {
	  //bdd temp = pawalker.bddsatone(curBdd,false); 
	  bdd temp = bdd_satone(curBdd);
	  curBdd &= fdd_ithvar(C, v->getGlobalIndex());
	  curBdd &= fdd_ithvar(Z, 0);
#ifdef CCDebug	
	  cout << "curBdd: " << curBdd << endl;
#endif
	  pawalker.mypa.CovertChannelA = curBdd;
#ifdef CCDebug
	  cout << "curBddddddddd: " << pawalker.mypa.CovertChannelA << endl;
#endif
	  while (temp != bddfalse) {		
			curBdd -= temp;
			int intV = fdd_scanvar(temp, V2);
#ifdef CCDebug
			cout << "addInforFlow: " << fddset << temp << endl;
#endif
			if (0 != intV) {
#ifdef CCDebug
				  cout << "enter addInforFlow" << endl;
#endif
				  temp &= fdd_ithvar(C, v->getGlobalIndex());
				  temp &= fdd_ithvar(Z, 0);
				  //IFCollector->CCAll |= temp;
				  //IFCollector->CCAll &= fdd_ithvar(C, v->getLocalIndex());
				  //IFCollector->CCAPath |= temp;
				  //IFCollector->CCAPath &= fdd_ithvar(C, v->getLocalIndex());
			}
			else {
				  temp &= fdd_ithvar(C, v->getGlobalIndex());
				  temp &= fdd_ithvar(Z, 0);
				  //IFCollector->CCBranch |= temp;
			}
			temp = bdd_satone(curBdd);
	  }
#ifdef CCDebug
	  cout << "IFCollector->CCAPath: " << fddset << IFCollector->CCAPath << endl;
	  cout << "end addInforFlow: " << fddset << IFCollector->CCAll << endl;
	  cout << "CCBranch: " << IFCollector->CCBranch << endl;
#endif
}

// 20100707 by kong
// extern map<int, int> TmpToReal; 
int DataFlowAnalyzer::findRealID(int tempID) {
	  map<int, int>::iterator iMap = TmpToReal.begin();
	  int firstTempID = iMap->first;
	  int realID = 0;
	  for (; iMap != TmpToReal.end(); iMap++) {
			if (tempID == iMap->first) {
				  realID = iMap->second;
				  if (realID < firstTempID) {
						//cout << "�ҵ�ʵ�ʵ�ID��: " << realID << endl;
						return realID;
				  }
				  else {
						map<int, int>::iterator iMap2;
						for (iMap2 = TmpToReal.begin(); iMap2 != TmpToReal.end(); iMap2++) {
							  if (iMap2->first == realID) {
									//cout << "�ҵ�ʵ�ʵ�ID��2222: " << iMap->second << endl;
									return iMap->second;
							  }
						}
				  }
			}
	  }
	  cout << "�Ҳ���ʵ�ʵ�ID" << endl;
	  return 0;
}

analyzedNodeStack DataFlowAnalyzer::nodestack;
void analyzedNodeStack::push(VexNode* v)
{
	  nodeStack.push_back(v);
}
void analyzedNodeStack::pop()
{
	  if(!nodeStack.empty())
	  {
			nodeStack.pop_back();
	  }
}
void analyzedNodeStack::popMore(int n)
{
	  if(n <= 0)
			return;
	  for(int i=0;i<n;i++)
			pop();
}
void analyzedNodeStack::printStack()
{
	  vector<VexNode*>::iterator it=nodeStack.begin();
	  while(it!=nodeStack.end())
	  {
			if(it==nodeStack.begin())
			{
#if futeng_path_output
				  futeng_path<<(**it);
#endif
			}else{

#if futeng_path_output
				  futeng_path<<"->"<<(**it);
#endif
			}
			it++;
	  }
}
int analyzedNodeStack::findNodeInStack(VexNode* v)
{
	  int ret = 0;
	  vector<VexNode*>::reverse_iterator it = nodeStack.rbegin();
	  while(it != nodeStack.rend())
	  {
			if(0 == v->getLocalIndex()){//�ҵ��˵�ǰ�����ĺ�������ڽڵ㣬���ڼ�����ǰ����
				  break;
			}
			if(*it == v)
			{
				  ++ret;
			}
			++it;
	  }
	  return ret;
}
bool analyzedNodeStack::matchNodeAndPop(VexNode* v)
{
	  bool findVex=false;
	  vector<VexNode*>::reverse_iterator it=nodeStack.rbegin();
	  while(nodeStack.size()>0 && (*it)!=v)
	  {
			nodeStack.pop_back();
			it++;
	  }
	  if(nodeStack.size()>0)
	  {
			findVex=true;
	  }
	  return findVex;
}
void analyzedNodeStack::popCurrentFuncNode()
{
	  while(nodeStack.size()>0&&nodeStack.back()->getLocalIndex()!=0)
	  {
			nodeStack.pop_back();
	  }
	  nodeStack.pop_back();
}
const deque<int>& analyzedNodeStack::getSwitchesInStack(VexNode* v,int index,bool reAalculation)
{
	  static stack<VexNode*> circlestk;
	  static bool pushintostacknexttime=false;
	  int count=1;
	  int count2;
	  VexNodeType nodetype;
	  stack<int> stk;
	  if(reAalculation)
	  {
			getSwitchesInStack(v,circlestk,stk);
			return getSwitch;
	  }
	  //��ǰҪ��������ջ��Ԫ��
	  if(pushintostacknexttime)
	  {
			getSwitch.push_back(index-1);
			pushintostacknexttime=false;
	  }
	  nodetype=v->getNodeType();
	  if(nodetype==vCircle||nodetype==vDoWhile)
	  {
			if(circlestk.empty())
			{
				  circlestk.push(v);
				  pushintostacknexttime=true;
				  //return getswitch;
			}else if(v!=circlestk.top())
			{
				  circlestk.push(v);
				  pushintostacknexttime=true;
				  //return getswitch;
			}else if(v==circlestk.top())
			{
				  circlestk.pop();
				  getSwitch.pop_back();
				  //return getswitch;
			}
	  }
	  int outnum=v->getOutNum();
	  int innum=v->getInNum();
	  ArcBox* arc=v->getFirstInArc();
	  while(arc){
			VexNode* prenode=arc->getHeadVex();
			if(prenode->getRank()>=v->getRank()&&v->getData()->getNodeType()!=eNormalExitNode)
			{
				  innum--;
			}
			arc=arc->getNextEdgeWithSameTailVex();
	  }
	  if(innum>1&& !stk.empty())
	  {
			stk.pop();
			getSwitch.pop_back();
			//return getswitch;
	  }
	  if(outnum>1&&nodetype!=vCircle&&nodetype!=vDoWhile)
	  {
			stk.push(index);
			//getSwitch.push_back(index);
			pushintostacknexttime=true;
			//return getswitch;
	  }
	  //futeng_debug<<"dk:"<<endl;
	  //for(deque<int>::iterator it=getSwitch.begin();it!=getSwitch.end();it++)
	  //{
	  //	futeng_debug<<"��"<<*it;
	  //}
	  //futeng_debug<<endl;
	  //futeng_debug<<"stk:"<<endl;
	  //for(vector<VexNode*>::iterator it=nodeStack.begin();it!=nodeStack.end();it++)
	  //{
	  //	futeng_debug<<"��"<<(*it)->getLocalIndex();
	  //}
	  //futeng_debug<<endl;
	  return getSwitch;
}
void analyzedNodeStack::getSwitchesInStack(VexNode* v,stack<VexNode*>& circlestk,stack<int>& stk)
{
	  int count=1;
	  vector<VexNode*>::iterator begin=nodeStack.begin();
	  vector<VexNode*>::iterator end=nodeStack.end();
	  vector<VexNode*>::iterator rbegin;
	  stack<VexNode*> ss;
	  while(!circlestk.empty())
			circlestk.pop();
	  while(!stk.empty())
			stk.pop();
	  getSwitch.clear();
	  while(begin!=end)
	  {
			VexNodeType type=(*begin)->getNodeType();
			if(type==vCircle||type==vDoWhile)
			{
				  rbegin=begin+1;
				  while(rbegin!=end)
				  {
						if(*begin==*rbegin)
							  break;
						rbegin++;
				  }
				  if(rbegin==end)
				  {
						VexNode* node=(*begin);
						circlestk.push(node);
						getSwitch.push_back(count);
				  }
			}
			begin++;
			count++;
	  }
	  begin=nodeStack.begin();
	  count=1;
	  while(begin!=end)
	  {
			VexNode* curVex=*begin;
			int outNum=curVex->getOutNum();
			int inNum=curVex->getInNum();
			ArcBox* arc=curVex->getFirstInArc();
			VexNodeType type=(*begin)->getNodeType();
			while(arc)
			{
				  VexNode* preVex=arc->getHeadVex();
				  if(preVex->getRank()>=curVex->getRank()&& curVex->getData()->getNodeType()!=eNormalExitNode)
				  {
						inNum--;
				  }
				  arc=arc->getNextEdgeWithSameTailVex();
			}
			if(inNum>1&&!stk.empty())
			{
				  stk.pop();
				  getSwitch.pop_back();
			}
			if(outNum>1&&type!=vCircle&&type!=vDoWhile)
			{
				  stk.push(count);
				  getSwitch.push_back(count);
			}
			begin++;
			count++;
	  }
}
ostream& operator<<(ostream &o,const analyzedNodeStack& stk)
{
	  const vector<VexNode*>& vec=stk.getNodeStack();
	  vector<VexNode*>::const_iterator it=vec.begin();
	  while(it!=vec.end())
	  {
			VexNode* v = *it;

			o<<v->getLocalIndex();
			it++;
			if(vec.end()==it||0==(*it)->getLocalIndex())
			{
				  o<<";";
			}else{
				  o<<",";
			}

			/*
			o<<v->getLocalIndex()<<"/";
			if(v->getLocalIndex() == 0)
			o<<"Start";
			else if(v->getLocalIndex() == 1)
			o<<"End";
			else if(v->getData()->getNodeType() == eNormalExitNode)
			o<<"Exit";
			else if(v->getData()->getNodeType() == eScopeOutNode)
			o<<"}";
			else if(v->getData()->getAST() != NULL)
			o<<((RefMyAST)(v->getData()->getAST()))->getLineNum();
			else
			o<<"_"<<v->getLocalIndex();

			it++;
			if(vec.end()==it||0==(*it)->getLocalIndex())
			{
			o<<";";
			}else{
			o<<"->";
			}
			*/
	  }
	  return o;
}





