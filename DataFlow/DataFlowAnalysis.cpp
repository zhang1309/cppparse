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

#define Branch_Predict 1 //  //开启分支条件预测--判断真假分支此开关由图形界面控制，在这里保持为1
#define xulei_dataflow_output 0
#define Traverse_Advanced_Output 0//这个不用时，一定要关掉
#define  Traverse_Advance 1      //开启预遍历置1，关闭置0 

#define Node_Trace 1   //smy   //打印输出节点
//调试模式下的开关
#define Debug_Mode  1 //调试模式的开关
#define MaxLine 0  //调试输出信息的上界
#define MinLine 0    //调试输出信息的下界
#define SpecialLine 0//输出单独一行的信息

#define Memory_Debug 0; //调试时输出内存模块的相关bdd信息
#define Taint_Debug 0; //调试时输出污染传播模块的相关bdd信息
#define leak_resource 0;//资源泄漏的调试开关，输出提条路径结束时的Bdd信息

#define debugSpecialFunction 1  //调试特定函数的开关
bool PrintSpecialFunctionFlag = false;  //调试大范围代码时，打印特殊函数指针堆空间BDD集合的标志

#define functionNeedTodebug "" //要调试的函数的名字

//调试模式下的开关--end
int g_cueernt_visited_line = 0;//记录当前访问节点的行号
bool isReachSink = false;  //是否到达sink点
extern bool g_project_use_condition_check; //是否开启分支判断  add 2016-11-16
extern int  g_project_loop_check_count;    //循环扫描遍数         add 2016-11-16
extern bool g_project_quit_on_sink;       ////Sink回退开关"     add 2016-11-18
//#define ANALYZE_OPT_TRACE

//add analysize pattern shot

 long long  FunctionCallCount=1;          //跨过程次数
 long long  FunctionCallPattternShotCount =0;  //跨过程模式匹配次数

 long long  BranchVexNodeCount=1;             //分直接点次数
 long long  BranchVexNodePatternShotCount =0; //分支节点模式匹配次数
//add analysize pattern shot edd



//ZLY, BEGIN, 2010-9-1,对路径数过多进行优化
#define PATH_OPT_NO_OPT				0	//不开启优化
#define PATH_OPT_ONLY_COVER			1	//不管路径数是否超过阈值，仅覆盖所有控制流结点即可（分支结点要覆盖true分支与false分支）
#define PATH_OPT_ONLY_COUNT			2	//当路径数超过阈值时，直接仅分析最多的路径数
#define PATH_OPT_COVER_AND_COUNT	3	//当路径数超过阈值时，仅覆盖所有控制流结点即可（分支结点要覆盖true分支与false分支）
int giOptimalPaths = PATH_OPT_COVER_AND_COUNT;//是否进行优化
int gi_PATH_COUNT_OPT_THRESHOLD = 200; //路径数阈值
#define MAX_PATH_COUNT				50000 //后端处理的最大路径数
#define CIRCLE_VISITED_MAX_NUM 1           //
//ZLY, END, 2010-9-1,对路径数过多进行优化

#define FUTENG_DEBUG
//#define CCDebug // by kong
#define TEMPVERSION
#define ASSISTANT

using namespace std;

//pattern map begin
map<VexNode*,map<int,bool>* > gLoopVexPatterns;  //记录循环节点的匹配模式
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

PATreeWalker * TraversePaTreeWalker =NULL ;  //因为预遍历中要用到PATreeWalker中的方法，故一个指针，将PATreeWalker在与便利中使用。


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
list<int> NodeList;        //节点链表，存放当前分析路径上的节点

//2010-9-16,ZLY,BEGIN,保存调用栈中所有函数的分析路径上的节点
list<int>* gp_CurPath;//当前函数的路径指针
//2010-9-16,ZLY,END,保存调用栈中所有函数的分析路径上的节点

SymbolItem* currFuncItem=0;         //全局变量，记录当前分析的函数条目


//王运 2009-6-17
extern SymRoot* root;//!0327 added by dengfan 
extern unordered_map<int ,SymbolItem*> VariableMap;
//extern list<SymbolItem*> globalFunctions;//这个变量用来存储具有AST的函数条目 wangyun 2008-11-18
extern FlowGraph flowGraph;
bool wangyunOk=false;

extern FuncMatrix funcMatrix;
map<SymbolItem*, CFGPaths> dataFlowPaths; // 20100106 by kong
extern list<PathBddMgr* > pathBddMgrList; // by kong 
extern InforFlowCollector* IFCollector;
extern map<int, int> TmpToReal; // 20100707 by kong 
extern list<ConstToReal> ConstToRealList; // 20100707 by kong
int countListSize = 0; // 20100707 by kong

extern ProcessBar processbar;    //进度条控制对象
int back_process_begin=0;
int back_process_end=0;

ofstream func("function.log");
//by wangzhiqiang
ofstream infoRuntime("infoTreeRuntime.log");
//end
ofstream futeng_path("futeng_path.log",ofstream::trunc);
#define futeng_path_output 0
int giCompatibleCount = 0;//用于分析效率分析时统计本次分析的兼容函数个数
int giHistoryPathVexCount = 0;//用于分析效率分析时统计本次分析不用分析的路径节点数

extern ProcessBar processbar_Cur;

bool gb_OnlyOneEntry = false;//用于标记是否只有一个入口函数，用于显示更明细的进度信息
long gl_TotalEntryNodes = 0;//当gb_OnlyOneEntry为true时，记录入口函数中的所有结点数
long gl_CurrentAnalyzedNodes = 0;

void closeConsole();
void openConsole();

int getAllNodesNum(CFGPaths& paths)  //得到所有待分析节点个数
{
	  cout<<"进度条："<<endl;
	  //获得所有需要分析的节点个数
	  int all_nodes_num=0;    //要分析的节点总个数
	  int pathindex=0;       //路径下标
	  Path *curPathPtr=0;    //当前路径
	  Path *prePathPtr=0;    //前一条路径
	  while(curPathPtr = paths[pathindex])  //当前路径
	  {
			int vexindex=0;
			if(prePathPtr) vexindex = curPathPtr->firstDifferentVex(*prePathPtr);
			if(vexindex>0) vexindex--;   //得到分叉节点

			while((*curPathPtr)[vexindex])
			{
				  //cout<<(*curPathPtr)[vexindex]->getLocalIndex()<<endl;
				  ++vexindex;
				  ++all_nodes_num;
			}

			prePathPtr = curPathPtr;

			++pathindex;   //下一条路径
	  }

	  return all_nodes_num;
}


//xulei 20100414
RefAST ReConstructAST(RefAST ast, CPPParser* p_parser)      //重构AST，把e转换为!e
{
	  if(!p_parser)
			return ast;
	  RefAST temp_Expression=RefAST(ANTLR_USE_NAMESPACE(antlr)nullAST);	//创建Expression节点	

	  temp_Expression=p_parser->getASTFactory()->create(STDCTokenTypes::Expression,"Expression");

	  RefAST temp_unaryoperator=RefAST(ANTLR_USE_NAMESPACE(antlr)nullAST);	//创建unaryoperator节点	
	  temp_unaryoperator=p_parser->getASTFactory()->create(STDCTokenTypes::UnaryOperator,"UnaryOperator");

	  RefAST temp_not=RefAST(ANTLR_USE_NAMESPACE(antlr)nullAST);	 //创建not节点			
	  temp_not=p_parser->getASTFactory()->create(STDCTokenTypes::NOT,"!");

	  ANTLR_USE_NAMESPACE(antlr)RefAST(p_parser->getASTFactory()->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(temp_not)->add(ast)));
	  ANTLR_USE_NAMESPACE(antlr)RefAST(p_parser->getASTFactory()->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(temp_unaryoperator)->add(temp_not)));
	  ANTLR_USE_NAMESPACE(antlr)RefAST(p_parser->getASTFactory()->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(temp_Expression)->add(temp_unaryoperator)));

	  return temp_Expression;         //条件取非    
}

//以下两个函数相互调用，处理AST的等价转换，可以有效减少树的复杂度。
void visitAST(RefAST ast, CPPParser* p_parser);

void EquivalentConversion(RefAST ast, CPPParser* p_parser)  //对AST进行一次等价变换，处理!!e <=> e的情况
{
	  if (!p_parser)
	  {
			return;
	  }
	  RefAST root=ast->getFirstChild();

	  if(root && root->getType() == ASTInsTreeWalkerTokenTypes::UnaryOperator)    //根节点为UnaryOperator节点
	  {   
			RefAST child=root->getFirstChild();   //孩子节点

			if (child && child->getType()==ASTInsTreeWalkerTokenTypes::NOT)   //如果孩子节点为NOT节点
			{
				  if (child->getFirstChild() && child->getFirstChild()->getType()==ASTInsTreeWalkerTokenTypes::Expression)  //如果孩子节点的下一个节点为Expression节点
				  {
						child=child->getFirstChild();
				  }

				  RefAST grandchild=child->getFirstChild();   //孙子节点
				  if (grandchild && grandchild->getType()==ASTInsTreeWalkerTokenTypes::UnaryOperator)
				  {
						RefAST grandgrandchild=grandchild->getFirstChild();   //孙子的儿子节点
						if (grandgrandchild && grandgrandchild->getType()==ASTInsTreeWalkerTokenTypes::NOT)
						{
							  //完成等价转换
							  ANTLR_USE_NAMESPACE(antlr)RefAST(p_parser->getASTFactory()->make((new ANTLR_USE_NAMESPACE(antlr)ASTArray(2))->add(ast)->add(grandgrandchild->getFirstChild())));
							  //由于对结构进行了改变，所以需要重新遍历新的AST
							  visitAST(ast, p_parser);
						}
				  }
			}
	  }

}

void visitAST(RefAST ast, CPPParser* p_parser)    //深度优先，递归遍历ast的每个节点
{
	  if (ast)    //1 先处理父节点
	  {
			EquivalentConversion(ast, p_parser);
	  }

	  RefAST child=ast->getFirstChild();         //得到第一个孩子节点
	  if (child)           //如果孩子节点不空，遍历其孩子节点
	  {
			visitAST(child, p_parser);       //2 首先遍历第一个孩子节点
			RefAST brother=child->getNextSibling();  //得到右兄弟节点
			while(brother)     //3 遍历剩余的孩子节点
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
	//2016-08-18,ZLY,与入口处一致，先计算全局变量值，再计算全局指向信息
	  pawalker.resetCurrentAnalysis();
	  calculateGlobalVariableValues(&globalValues);
	  genGlobelPointto(pawalker);
	  //清除记录的模式
	  gFunctionPAPattern.clear();
	  gFunctionSensitivePattern.clear();
	  gFunctionTaintPattern.clear();
	  gLoopVexPatterns.clear();
	  gFunctionPAPatternVHeapChange.clear();
	  //2016-09-11  smy-add 清空newLocation存储的函数错误位置信息，以免对其他入口函数造成影响
	  pawalker.newLocation.clear();
	  pawalker.stringMap.clear();
	  //smy-end
}
/*
作用：处理入口函数的参数中类型为指针的参数
参数：  pawalker:指针分析walker，主要向pawalker.mypa中的相应bdd存入信息
bddIt：参数对应的bdd
type：参数的类型的符号条目

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
			if(TYPE_POINTER==k)//下一级仍然是个指针
			{
				  pawalker.mypa.pointto|=paraBdd&fdd_ithvar(V2,++(pawalker.curvarnum))&fdd_ithvar(F2,0);//创建一个虚拟变量作为当前指针的下一级指向
				  paraBdd=fdd_ithvar(V1,pawalker.curvarnum)&fdd_ithvar(F1,0);
				  pawalker.heap_type[pawalker.curvarnum] = "Entry_Function_ParaBDD";               
				  pawalker.mypa.v_heap |= fdd_ithvar(V2,pawalker.curvarnum) & fdd_ithvar(F2,0);     //创建虚拟变量时，将其加入堆空间
 				  type=t;
			}else if(TYPE_STRUCT==k){//该变量其实是一个指向结构体类型的指针，需要处理结构体内的成员
				  ScopeSymbolTable* sc=static_cast<ScopeSymbolTable*>( t->getPointtoSymTable());
				  if(sc==NULL)
				  {
						return;
				  }
				  //创建一个结构体变量--   大于等于778297sssssss
				  pawalker.mypa.v_struct|=fdd_ithvar(V1,++(pawalker.curvarnum));
				  pawalker.mypa.pointto|=paraBdd&fdd_ithvar(V2,pawalker.curvarnum)&fdd_ithvar(F2,0);
				  pawalker.heap_type[pawalker.curvarnum] = "Entry_Function_ParaBDD";
				  pawalker.mypa.v_heap |= fdd_ithvar(V2,pawalker.curvarnum) & fdd_ithvar(F2,0);
				  //处理结构体内的成员
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
				  //把结构体成员递归处理
				  processFunctionPara(pawalker,explistPara,recursiveDepth+1);
				  isPointer=false;
			}else if(TYPE_FUNC==k){//是函数指针
				  isPointer=false;
			}else{
				  pawalker.mypa.v_heap|=fdd_ithvar(V2,++(pawalker.curvarnum))&fdd_ithvar(F2,0);
				  //heapsize??
				  int iHeapCode = pawalker.curvarnum;  //add 2016-10-09
				  pawalker.heap_type[iHeapCode] = "Entry_Function_ParaBDD";   //smy 设置入口函数参数BDD的特殊类型 
				  pawalker.mypa.pointto|=paraBdd&fdd_ithvar(V2,(pawalker.curvarnum))&fdd_ithvar(F2,0);
				  isPointer=false;
			}			

	  }
}
/*
作用：处理入口函数的参数，主要是结构体、指针、数组类型，创建其虚拟指向信息，加入相应的bdd中
参数：  pawalker:指针分析walker，主要向pawalker.mypa中的相应bdd存入信息
explist:形参的bdd列表

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
			//1、通过bdd找到符号的ID
			v1=fdd_scanvar(*bddIt,V1);
			f1=fdd_scanvar(*bddIt,F1);//如果F1域有值，说明是在结构体内
			if(f1>0) 
			{
				  symbolID=f1;
			}else{
				  symbolID=v1;
			}
			//2、通过VariableMap找到形参对应的SymbolItem
			unordered_map<int,SymbolItem*>::iterator sblIt=VariableMap.find(symbolID);
			if(VariableMap.end()==sblIt)//找不到则跳过去
			{
				  continue;
			}
			SymbolItem* sbl=sblIt->second;
			//3、判断参数的类型并处理
			Type *type=sbl->getTypeItem();//找到形参的类型的符号条目
			ItemKind kind=type->getItemKind();
			if(TYPE_STRUCT==kind)//是结构体
			{
				  if(f1>0)//处理结构体内嵌结构体成员的情况
				  {
						pawalker.mypa.f_struct|=fdd_ithvar(F1,f1);//记录下来
						//变量名折叠
						pawalker.mypa.fieldOpe|=(*bddIt)&fdd_ithvar(V2,++(pawalker.curvarnum));
						symbolID=pawalker.curvarnum;
				  }
				  //处理结构体内的成员
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
				  //把结构体成员递归处理
				  processFunctionPara(pawalker,explistPara,recursiveDepth+1);
			}
			else if(TYPE_POINTER==kind)//如果类型是指针
			{
				  typeIsPointer(type,*bddIt,pawalker,recursiveDepth);
			}else if(TYPE_ARRAY==kind)//如果类型是数组
			{
				  pawalker.mypa.pointto|=(*bddIt)&fdd_ithvar(V2,++(pawalker.curvarnum))&fdd_ithvar(F2,0);
				  int n=fdd_scanvar(*bddIt,F1);
				  if(n>0)
				  {
						pawalker.mypa.v_array|=fdd_ithvar(V1,v1);//用v_array比较，是因为之前代码在类型判定后数组全都存到了v_array中
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
* 函数名称：switchTaintedSensitive
* 功    能：交换敏感信息和污染源的配置信息
* 返 回 值：
* 备    注：敏感信息和污染源共用一套代码，由于一期的时候配置信息是放在sensitiveMap中，
所以在进行污染信息检查时，为了复用敏感信息的代码，需要把taintedTraceMap
中的内容复制到sensitiveMap中，该模块检测完以后再换回来
* 上次修改：
* 创 建 人：futeng
* 日    期：2016.5.5 15:5
*******************************************************************/
void switchTaintedSensitive()
{
	  map<string,set<int> > temp;
	  temp = sensitiveMap;
	  sensitiveMap = tainedTraceMap;
	  tainedTraceMap = temp;
}

/*******************************************************************
* 函数名称：printTraverseAdvancedResult
* 功    能：打印预遍历结果信息
* 返 回 值：
* 备    注
* 上次修改：
* 创 建 人：shenmengyuan
* 日    期：2016-10-19
*******************************************************************/
void printTraverseAdvancedResult(PATreeWalker &pawalker)
{
	FunctionManager* pFuncManager = FunctionManager::functionManagerInstance();
	list<SymbolItem*>& sortedFunctionList = pFuncManager->getSortedFunctionList();   //获取所有入口函数
	openConsole();
	cout<<"---------------预遍历结果--------------"<<endl;
	closeConsole();

	//打印输出结果
	for(list<SymbolItem*>::iterator it = sortedFunctionList.begin();it!=sortedFunctionList.end();it++)
	{
		
		openConsole();
	
			if((*it)->getName()=="")
				continue;
		FunctionFlagSet * tempFlagSet = TraverseInfo::findModuleFunctionInformation[(*it)->getName()];
		if(tempFlagSet==NULL)
			return;
		cout<<"+++++++++++++++++++++++++++++当前函数（"<<(*it)->getName()<<"）的预遍历结果++++++++++++++++++++++++++++++++++++"<<endl;
		cout<<"---------各个标识变量信息----------"<<endl;
		cout<<"b_change_pointer:"<<tempFlagSet->b_change_pointer<<endl;
		cout<<"b_danger_sink:"<<tempFlagSet->b_danger_sink<<endl;
		cout<<"b_memory_sink:"<<tempFlagSet->b_memory_sink<<endl<<endl;

		cout<<"b_taint_change:"<<tempFlagSet->b_taint_change<<endl;
		cout<<"b_taint_sink:"<<tempFlagSet->b_taint_sink<<endl;
		cout<<"b_taint_source:"<<tempFlagSet->b_taint_source<<endl<<endl;


		cout<<"b_sensitive_change:"<<tempFlagSet->b_sensitive_change<<endl;
		cout<<"b_sensitive_source:"<<tempFlagSet->b_sensitive_source<<endl;
		cout<<"b_sensitive_sink:"<<tempFlagSet->b_sensitive_sink<<endl;
		cout<<"++++++++++++++++++++++++++++结束++++++++++++++++++++++++++++++++++++"<<endl;
		
		closeConsole();
		CFG* currentCFG = (CFG*)(*it)->getCFG();  //得到函数的CFG
		if(currentCFG != NULL)
		{


			VexNode* pNode = currentCFG->getHead();//得到cfg的入口节点
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
						cout<<"语句行号："<<line<<" " <<static_cast<RefMyAST>(ast)->getText()<<
							"\t是否需要访问：:"<<shouldNodeBeCheck(pawalker,pNode)<<endl;
#else
						cout<<"语句行号："<<line<<" " <<static_cast<RefMyAST>(ast)->getText()<<endl;
						cout<<"---------各个标识变量信息----------"<<endl;
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
// 						cout<<"bCheck:"<<pNode->getIsNeedCheck()<<endl;    //特殊节点专用标记
// 						cout<<"needVisited:"<<pNode->getNeedVisit()<<endl;  //入口函数专用标记

						
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

	cout<<"语句行号："<<line<<" " <<static_cast<RefMyAST>(ast)->getText()<<endl;
	cout<<"---------各个标识变量信息----------"<<endl;
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
	  // 1. 计算并保存全局变量值
	  VexValueSet globalValues;	// 用来保存全局变量的整型值信息, 作为每个函数分析时的入口流值
	  calculateGlobalVariableValues(&globalValues);
	  //IFCollector = new InforFlowCollector();
	  //cxx
	  PATreeWalker pawalker(1);
	  genGlobelPointto(pawalker);
	  //cxx end

	  //打印所有的函数
	  /*sensitive_error<<"函数总数："<<g_FunctionList.size()<<endl;
	  for(list<SymbolItem*>::iterator it_func = g_FunctionList.begin(); it_func != g_FunctionList.end(); ++it_func)
	  {
	  sensitive_error<<(*it_func)->getName()<<endl;
	  }
	  */



	  // 20100508
	  //2010-10-9,ZLY,不管是否有main,均生成调用图，并分析所有入口函数
	  FunctionManager* pFuncManager = FunctionManager::functionManagerInstance();
	  list<SymbolItem*>& entrySbl = pFuncManager->getAllEntryFunc();
	  int total_func_numbers = entrySbl.size(); 
	  int curr_func_number=0;		

	  if(1 == total_func_numbers)
			gb_OnlyOneEntry = true;
	  bool bFirstFunc = true;

	  ModuleManager* pModuleManager = ModuleManager::moduleManagerInstance();//模块控制器

	  bool bSwitched = false;
	  TraverseTreeWalker traverseWalker;
	  int curvarnumBackUp = pawalker.curvarnum;
	  pModuleManager->openSM();//执行自动机模块
	  int iTotalModules = pModuleManager->getTotalModuleCount();
	  int iModuleCount = 0;
	 // while(pModuleManager->nextModule() != ModuleManager::MODULE_FINISHED)//每一个检测模块
	//  {
// 			if(ModuleManager::MODULE_TAINTED == ModuleManager::runningType)  //这部分暂时不作处理
// 			{
// 				  switchTaintedSensitive();//见函数声明处说明
// 				  bSwitched = true;
// 			}


			SymbolItem* funcItem;
			pawalker.setAnalzingModuleType(ModuleManager::runningType);//设置该次分析的模块
			curr_func_number = 0;
// 			openConsole();
// 			cout<<"正在分析第("<<pModuleManager->getAnalyzingModuleIndex()<<"/"<<pModuleManager->getTotalModuleCount()<<")个模块："<<pModuleManager->getModuleName()<<endl;
// 			closeConsole();
			/*ft 进行预遍历*/
			time_t traverseBegin,traverseEnd;
			time(&traverseBegin);
			openConsole();
			cout<<"预遍历开始"<<endl;
			closeConsole();
			TraversePaTreeWalker  = &pawalker;  //将全局唯一的pawalker赋值给全局对象，在预遍历中使用
			traverseAdvanceEntry(traverseWalker,ModuleManager::ALL_MODULE);  //
			time(&traverseEnd);
			openConsole();
			cout<<"预遍历结束，耗时:"<<(traverseEnd - traverseBegin)<<endl;
			closeConsole();
			runTimeLog<<"预遍历，耗时:：" << (traverseEnd - traverseBegin) << " 秒.\n";
			runTimeLog.flush();
#if Traverse_Advanced_Output
			printTraverseAdvancedResult(pawalker);
#endif
		//	exit(0);
			/*end*/
			list<SymbolItem*>& sortedFunctionList = pFuncManager->getSortedFunctionList();   //获取所有排序的函数
			pawalker.generateFuncTempParaForAll(sortedFunctionList);
			for (list<SymbolItem*>::iterator it = entrySbl.begin(); it != entrySbl.end(); it++) //每一个入口函数
			{
				  funcItem =*it;//当前要分析的入口函数
				  if(bFirstFunc){
						bFirstFunc = false;
				  }else{
						resetCurrentAnalysis(pawalker, globalValues);
						pawalker.curvarnum = curvarnumBackUp;
						if (funcItem)
							  (((FuncSymbolItem*)(funcItem))->function_int_map).clear();
				  }
				  ++curr_func_number;


				  if(!(funcItem->getASTNode()))						// 函数没有对应的AST, 很可能为声明条目
				  {			
						continue; 
				  }
				  add2FuncStack(pawalker,funcItem);

				  cout.rdbuf(pOld);
				  cout<<"正在分析第("<<curr_func_number<<"/"<<total_func_numbers<<")个入口函数："<<funcItem->getName()<<endl;
				  pOld = cout.rdbuf(DzhTest.rdbuf());	 
				  processbar_Cur.setCurrentAnalysisInfo(string("函数:") + funcItem->getName(), 100, 50);

				  GlobalTool::clearAllPatterns();
				  ScopeSymbolTable* sc=static_cast<ScopeSymbolTable*>( funcItem->getPointtoSymTable());
				  list<SymbolItem*> parameterList=sc->getFuncParaList();
				  list<SymbolItem*>::iterator paraIt = parameterList.begin();
				  vbdd explist;
				  while(paraIt!=parameterList.end())
				  {
						//首先判断是指针、数组、或者其他类型
						bdd tmp=fdd_ithvar(V1,(*paraIt)->getSymbolID())&fdd_ithvar(F1,0);
						explist.push_back(tmp);
						pawalker.mypa.v_funcEntryPara |=tmp;
						paraIt++;
				  }
				  processFunctionPara(pawalker,explist,1);  //为入口函数的指针类型变量分配BDD

				  DataFlowAnalyzer analyzer(funcItem,pawalker,p_parser);	// 为函数条目构造数据流分析器
				  analyzer.setEntranceValues(globalValues);		// 设置待分析函数的入口流值
				  analyzer.analyze(false,1);	
				  processbar_Cur.setCurrentAnalysisInfo(string("函数 ") + funcItem->getName(), 100, 100);
				  processbar.setBackOne(curr_func_number , total_func_numbers);
				  openConsole();
				  cout<<endl<<"当前函数调用次数:"<<FunctionCallCount<<"次"<<endl;
				  cout<<"函数调用模式匹配次数:"<<FunctionCallPattternShotCount<<"次"<<endl;
				  cout<<"当前扫描的分直接点数:"<<BranchVexNodeCount<<"次"<<endl;
				  cout<<"当前分支节点模式匹配数:"<<BranchVexNodePatternShotCount<<"次"<<endl;

				  cout<<endl<<"函数调用模式匹配命中率："<< (FunctionCallPattternShotCount*1.0/FunctionCallCount)<<endl;
				  cout<<"分支结点模式匹配命中率："<<(BranchVexNodePatternShotCount*1.0/BranchVexNodeCount)<<endl;
				  closeConsole();
			}

// 			if(bSwitched)
// 			{
// 				  switchTaintedSensitive();
// 				  bSwitched = false;
// 			}
			//processbar.setBcck_OneFunction(curr_func_number,total_func_numbers);
			
			// 利用数据流分析器对函数条目进行分析
			//break;
			pModuleManager->closeSM();//自动机模块只执行一次
			++ iModuleCount;
	//  }

	  processbar.setBackEnd();

	  paTrace<<"本次分析的函数兼容数为:"<<giCompatibleCount<<endl;
	  paTrace<<"本次分析的使用历史结点数为:"<<giHistoryPathVexCount<<endl;

	  //2010-08-12,ZLY,BEGIN,将资源泄漏检查放到每条路径结束后执行
	  //outputFinalError(pawalker);      //20100508, xulei
	  //2010-08-12,ZLY,END

	  //list<SymbolItem*>::iterator begin=globalFunctions.begin(); // 对应g_functionList
	  //list<SymbolItem*>::iterator end=globalFunctions.end();

	  cout.rdbuf(pOld);
	  cout<<endl<<"后端分析结束。"<<endl;
	  processbar_Cur.setCurrentAnalysisInfo(string("没有当前分析信息"),1);
	  //后端处理结束
	  /*processbar.setBackEnd();    
	  cout<<"正在构造信息流图..."<<endl;
	  cout.rdbuf(DzhTest.rdbuf());	

	  int iFunctionCount = 0;
	  int iTotalFunctions = globalFunctions.size();

	  //将每个函数的信息流添加到信息流图中。
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
	  // 		cout<<"------------------------------进入信息流生成函数--------------------------------"<<endl;

	  flow.ScanAFunc(pawalker, branchDepthMgr);
	  // 		cout << "ScanAFunc" << endl;
	  // 		flow.PutNodeNumber();
	  //将得到的信息流添加到图上。
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

	  //将边增加到各个结点的入边或者出边中
	  FlowArc* arc=new FlowArc(node1,node2,(*begin).GetFuncName(), (*begin).getFileName(), (*begin).getLineNum()); 
	  //查看该边是否已经在图中出现
	  bool ifExist3=flowGraph.IFArcExist(arc);
	  if(ifExist3==false)
	  {
	  //修改每个变量的入边集合和出边集合
	  node1->AddInArc(arc);
	  node2->AddOutArc(arc);
	  flowGraph.AddFlowArc(arc);
	  }
	  // 			cout << "flowGraph.deleteNull()" << endl;
	  //flowGraph.deleteNull();
	  //cout<<"出节点："<<(*begin).GetOutVar()<<"，入节点："<<(*begin).GetInVar()<<"，函数名: "<<(*begin).GetFuncName()<<endl;
	  }
	  //flow.printFlows();
	  iFunctionCount++;
	  processbar.setCCEnd_stage(0.5*((float)iFunctionCount/iTotalFunctions));
	  }
	  cout.rdbuf(pOld);
	  cout<<"构造信息流图结束"<<endl;*/
	  cout.rdbuf(DzhTest.rdbuf());	
}


/************************************************************************/
/* 类DataFlowAnalyzer方法定义                                           */
/************************************************************************/
DataFlowAnalyzer::DataFlowAnalyzer(SymbolItem* f,PATreeWalker& p,CPPParser* p_p):pawalker(p),p_parser(p_p)
{	
	  bTainedSinkFlag4VexNode = false;	//单个控制流结点是否触发了tained sink的标记
	  bTainedSinkFlag4Func = false;		//当前函数是否触发了tained sink的标记

	  m_treeWalker = new ExprValueTreeWalker;		// 构造AST遍历器
	  //by wangzhiqiang
	  //infoTreeWalker= new InfoTreeWalker(&p);

	  m_treeWalker->setDataFlowAnalyzer(this);	// 设置遍历器
	  m_treeWalker->setFuncInfoCollect(false);
	  m_func = f;									// 待分析函数
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

//2010-08-23,ZLY,不能这样做，因为这样可能导致跳过上条路径保存的分支结点现场，使得自动机的分析不正确
//但这样做对于指针指向信息来说应该是正确的
//应该通过对路径的合理排序达到提高分析效率的目的
/*
//2010-08-22,ZLY,BEGIN,向前查找相同结点数最多的一条路径，使用该路径的历史信息
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
//当前路径分析从返回的iFirstDifVex-1开始
//返回的iFirstDifVex-2是要取历史的分析结果结点
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
//2010-08-22,ZLY,END,向前查找相同结点数最多的一条路径，使用该路径的历史信息
*/
void DataFlowAnalyzer::analyze(int isInterprocedural,int deep)
{
	  if(!needAnalysis())	//当前函数条目已在栈中
	  {
			return;
	  }
	  pushCallStack();// 将当前函数条目压入函数调用栈
	  if(m_func)
	  {
			if(isInterprocedural)
			{
				  int callNum=m_callStack.getStackSize();
				  for(int i=0;i<callNum;i++)
						function_call<<"\t";
				  function_call<<"被调用函数："<<m_func->getName()<<"("<<m_func->getFileLineNum()<<")"<<endl;
			}
			else
			{
				  function_call<<"入口函数："<<m_func->getName()<<"("<<m_func->getFileLineNum()<<")"<<endl;
				  /*	
				  openConsole();
				  cout<<"入口函数："<<m_func->getName()<<"("<<m_func->getFileLineNum()<<")"<<endl;
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
			function_call<<"函数"<<m_func->getName()<<" cfg为空"<<endl;
			popCallStack();
			return;
	  }
#if debugSpecialFunction
	  if (m_func->getName()==functionNeedTodebug)
	  {
		  PrintSpecialFunctionFlag =true;  //这是为了调试大范围代码时，快速打印精准信息的标志
	  }
#endif
	  analyzeNode(cfg->getHead(),NULL,NULL,isInterprocedural);
	  getParentOutInfor_ALL(NULL,pawalker);   //将函数所有路径的信息汇总
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
	  //对于每个污染变量，扫描所有自动机，看那些自动机使用了tained状态
	  //首先判断污染变量是否在ProgramState的Symbol_list中，如不在则加入其中
	  //其次要保证污染变量的状态是tained
	  //ProgramState
	  /*
	  把王志强写的infoTreeWalker.g从项目中删除了，这里的tainedInfo本来是他的.g中定义的变量，这里添加一个局部变量
	  使链接通过。
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
				  cout<<"自动机标记，没有找到符号条目"<<endl;
				  continue;
			}

			//1
			//判断该条目是否已经有状态，没有就加进去 Symbol_list存放所有有状态的符号条目
			list<SymbolItem*>::iterator it2;
			for(it2=ProgramState.Symbol_list.begin();it2!=ProgramState.Symbol_list.end();it2++)
			{
				  if(*it2==sbl)
				  {
						break;//找到
				  }
			}
			if(it2==ProgramState.Symbol_list.end())
			{
				  ProgramState.Symbol_list.push_back(sbl);//如果找不到，就把该条目添加进去
			}
			//2
			list<AttachToSymbolItem*>* lstAttach = (list<AttachToSymbolItem*>*)(sbl->getSM_State());
			if(lstAttach != NULL)
			{
				  //Sm_list存放的是所有自动机
				  for(list<_SM*>::iterator it3=ProgramState.Sm_list.begin(); it3!=ProgramState.Sm_list.end(); it3++)
				  {
						//首先找到自动机中的tained状态
						State* tainedState = FindTainedState((*it3));
						if(tainedState==NULL)
							  continue;

						bool bHasState = false;
						for(list<AttachToSymbolItem*>::iterator it4=lstAttach->begin(); it4!=lstAttach->end(); it4++)
						{
							  if((*it4)->Sm_list->find(*it3) != (*it4)->Sm_list->end())
							  {//变量在该自动机中有状态
									bHasState = true;
									//确保其状态为tained
									(*(*it4)->Sm_list)[(*it3)] = tainedState;
									break;
							  }
						}
						if(bHasState==false)
						{//加入自动机状态到第一个AttachToSymbolItem中
							  (*(lstAttach->begin()))->Sm_list->insert(attach_value((*it3), tainedState));
						}
				  }
			}else//该变量无状态
			{//创建AttachToSymbolItem，记录状态
				  for(list<_SM*>::iterator it3=ProgramState.Sm_list.begin(); it3!=ProgramState.Sm_list.end(); it3++)
				  {
						//首先找到自动机中的tained状态
						State* tainedState = FindTainedState((*it3));
						if(tainedState==NULL)
							  continue;
						if(lstAttach == NULL)
						{
							  lstAttach = new list<AttachToSymbolItem*>();
							  sbl->setSM_State(lstAttach);
							  /* 此处临时生成一个只有一层的MingleSymbolItem，实际需要构造完整的MingleSymbolItem
							  * 实际处理可能是这种逻辑：先根据sbl构造MingleSymbolItem，然后按照MingleSymbolItem去匹配list<AttachSymbolImte>中的每一个
							  */
							  AttachToSymbolItem * tmp = new AttachToSymbolItem();
							  tmp->Mingled_Item.addSubSblItem(sbl);
							  tmp->Sm_list->insert(attach_value(*it3,tainedState));
							  lstAttach->push_back(tmp);
							  sm_debug<<"状态转移！"<<endl;
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
* 函数名称：
* 功    能：从头结点开始，依次分析一个函数的CFG结点
* 参    数：
* 返 回 值：
* 备    注：
* 上次修改：
* 创 建 人：
* 日    期：2016.8.30 10:36
*******************************************************************/
void DataFlowAnalyzer::analyzeNode(VexNode *curNode,VexNode *preNode,ArcBox *arc,bool isInterprocedural)
{
	/************************************************************************/
	/* int fun()
	{
	  int a = malloc();
	  if(....)//如果在这里模式匹配成功，直接返回，指针指向信息会被丢失
	  free(a);
	
	
	}*/
	/************************************************************************/
	  if(curNode != NULL && false == isContinueRun(pawalker,curNode)) //如果当前节点不为空，而且是分直接点，如果模式匹配成功，则不再向下走2016-11-2
	  {      
		    BranchVexNodePatternShotCount++;                              //模式匹配的实质：上次走到这个分支结点的mypa.pointto里的指向信息和这次走到这个节点的mypa.pointto里的指向信息完全相同（只要mypa.pointto的Root值相同），//则认为是模式匹配，不再向下走
		   if( !shouldNodeBeCheck(pawalker,curNode))                   //直接返回会丢失指针指向信息，必须保证后面的节点完全不访问，才可直接返回
		   {
#if Node_Trace
			   openConsole();
			   cout<<endl<<"节点模式匹配：当前节点["<<*curNode<<"]模式匹配且后续路径无需访问,路径终止。";
#if leak_resource 
			   cout<<"line(此时):"<<line<<endl;
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
	  //static int pathCnt=1;//路径计数器
	  //static int pathSize=1;//用来判断路径是否走完
	  //static int vexIndex=0;//节点索引,从1开始计数
	  bool bFinalPath=false;
	  bool isFirstNodeOnPath=false;   //当前节点是否为当前路径的第一个节点
	  ArcBox* arcFalse=NULL;
	  ArcBox* arcTrue=NULL;
	  map<string, PaIntValue> backup_map;//遇到判断节点的时候用来备份的map
	  ValueParameter integerOperationType;
	  int line;
	  while(curNode!=NULL)
	  {	
		  line = getFileLine(curNode);
#if Traverse_Advanced_Output 
		    
				printCurrentNodeTraverseAdvancedResult(pawalker,curNode); //打印上一次的预遍历结果
#endif
			analyzeNodeIndex++;  //结点在单一路径中的编号
			nodestack.push(curNode);
			iPushNodeCount++;//记录本层调用压栈节点数

		//	openConsole();
		//	cout<<"line:"<<line<<" : "<<"pointto:"<<fddset<<pawalker.mypa.pointto<<endl;
		//	closeConsole();
#if 0
			20160530 将这两条语句移到visitVex()中
				  //pawalker.findSensitiveSink=false;
				  bTainedSinkFlag4VexNode = false;
			pawalker.analyzingNodeType=curNode->getNodeType();
#endif
#if 0 
			20160530 注释掉老版本的整形分析
				  m_treeWalker->setVexIsCircleHead(false);
			m_treeWalker->walkAstOnVex(curNode,preNode);
#endif
#if Traverse_Advance
			//预遍历模块标记该节点之后都不用走  
			if( !shouldNodeBeCheck(pawalker,curNode))
			{
#ifdef ANALYZE_OPT_TRACE
				openConsole();
				cout<<"advance tarverse mark"<<endl; 
				closeConsole();
#endif
				
#if  futeng_path_output
					futeng_path<<"一条路径分析完毕，信息收集..."<<endl;
#endif

#if Node_Trace
					openConsole();
					cout<<endl<<"预遍历：当前节点["<<*curNode<<"]及其后续节点无需访问，路径中止。";
#if leak_resource 
					cout<<"line(此时):"<<line<<endl;
					cout<<"v_heap:"<<fddset<<pawalker.mypa.v_heap<<endl;
					cout<<"pointto:"<<fddset<<pawalker.mypa.pointto<<endl;

#endif
					closeConsole();
#endif
					//2010-08-12,ZLY,BEGIN,最后一条路径进行信息汇总，入口函数的路径可达时进行资源泄漏检查
					if(isInterprocedural == false){  //isInterprocedural:在最外层被置为false
						bool bFinalPath=false;
						if(1==theRestPathSize)
						{
							bFinalPath=true;
						}
						outputFinalErrorByVHeap(pawalker);
					}

					/*一条路径分析完毕，记录下其指向信息和堆空间的信息，以该函数的id作为标记*/
					recordOnePathInfo(pawalker);

					if(1 == theRestPathSize && isInterprocedural == false)
					{
#if futeng_path_output
						futeng_path<<"\t\t入口函数最后一条路径结束，进行资源回收..."<<endl;
#endif
						releaseLeakCheckResource(pawalker); //资源回收
					}
				
					--theRestPathSize;
				nodestack.popMore(iPushNodeCount);//将压入栈中的“直线”节点串弹出
				return;
			}   
#endif

#if Node_Trace  
			openConsole();                     //打印输出
			if(1==analyzeNodeIndex)//首节点
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


			if(2==curNode->getOutNum())//如果是分支节点，跳出循环
			{
				BranchVexNodeCount++;
				//++theRestPathSize;//遇到分支结点，那么需要遍历的路径加一
				break;
			}
			visitVex(curNode,preNode,pawalker,beingAnalysisPathIndex,
				curNode->getFirstOutArc(),
				true,theRestPathSize,isInterprocedural,integerOperationType);
			openConsole();


			cout<<"line(此行访问后):"<<line<<endl;
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
			//下面这种sink机制现在不用，在与便利处改为回退到入口函数的sink机制
			if( 1 && getTainedSinkFlag4VexNode())//发现一个sink点，且发现错误，该路径不再继续向后走，直接返回
			{
#ifdef ANALYZE_OPT_TRACE
				  openConsole();
				  cout<<"tained sink triggered, quit current path...."<<endl;
				  closeConsole();
#endif
				  --theRestPathSize;
				  recordOnePathInfo(pawalker);
				  nodestack.popMore(iPushNodeCount);
				  recoverSinkFlag4VexNode();//sink结点只在报错的一条路径上将语句截断，不影响一个函数内的其他路径
				  return;
			}
#endif

			/*
			//自动机
			//标记污染源BEGIN
			MarkTainedVar(curNode);
			//标记污染源END
			handleVexState(curNode,isFirstNodeOnPath,pawalker,m_treeWalker,false);
			travelVex(curNode,false,pawalker,m_treeWalker,false);
			//自动机end
			*/
			if(curNode->getLocalIndex()==1)//尾结点（头结点是0，尾结点是1）
			{
#if Node_Trace
				   openConsole();
				   cout<<"->"<<*curNode<<"[尾节点]"<<endl;
#if leak_resource 
				   cout<<"line(此时):"<<line<<endl;
				   cout<<"v_heap:"<<fddset<<pawalker.mypa.v_heap<<endl;
				   cout<<"pointto:"<<fddset<<pawalker.mypa.pointto<<endl;

#endif
				   closeConsole();
#endif
// 				  futeng_path<<"->"<<*curNode<<endl;
				  theRestPathSize--;
				  /*
				  if(theRestPathSize==0)//最后一条路径的最后一个节点分析完，将函数弹出栈
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

	  /*开始处理分支节点*/
	  VexNodeType nodeType = curNode->getNodeType();
	  unsigned char backUpCircleNum = 0;
	  if(vCircle == nodeType || vDoWhile == nodeType){
			backUpCircleNum = nodestack.findNodeInStack(curNode);//当前circle节点被分析的次数
	  }
	  if(++pathBranchCount > pathBranchTravelCount)
	  {
			openConsole();
			cout<<"function branch count greater than threshold "<<pathBranchTravelCount<<", quit current path...."<<endl;
			closeConsole();
			nodestack.popMore(iPushNodeCount);
			return;
	  }
	  //分支
#if 0
	  20160601注释，这样的模式比对，逻辑有问题
			/*判断iPushNodeCount>1 是因为如果分析的第一个结点是分支结点，分重复模式匹配两次（与函数入口处的模式匹配冲突）
			，第二次肯定就会兼容了*/
			if((vIf == nodeType || vSwitch ==nodeType) //判断分支节点或者第一次进入循环节点之前发现匹配的模式，那么就直接返回了
				  || ((vDoWhile == nodeType || vCircle == nodeType) && 1 == backUpCircleNum)
				  ){
						if(iPushNodeCount > 1 && false == isContinueRun(pawalker,curNode))//该模式已经有了，结束
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
			if(arcFalse->getEdgeType()!=eFalse)//首先处理真分支
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
			/*如果节点是circle节点，且已经循环了最大次数，那么从false分支跳出;而分支节点无条件的试着走真假分支*/
			if(((vDoWhile == nodeType || vCircle == nodeType) && g_project_loop_check_count >= backUpCircleNum)
				|| (vIf == nodeType || vSwitch == nodeType)
				){
						/*如果是分支节点或者第一次进入循环节点，那么需要在分析之前“备份信息”，后续的若干次循环不需要备份，走false分支时恢复*/
						if(vIf == nodeType || vSwitch == nodeType ||
							  ((vCircle== nodeType || vDoWhile == nodeType) && 1 == backUpCircleNum)
							  ){
								    ++theRestPathSize;//遇到分支结点，那么需要遍历的路径加一
									backUpHeap = pawalker.mypa.v_heap;
									backUpPointto = pawalker.mypa.pointto;
									sensitiveBackUp = pawalker.mypa.isSensitivedSource;
									taintBackUp = pawalker.mypa.isTaintedSource;
									//pointCOut=pawalker.mypa.pointtoCOut;
									//heapCOut=pawalker.mypa.v_heapCOut;
									backup_map = ((FuncSymbolItem*)DataFlowAnalyzer::getCallStack().top())->function_int_map;
									//xxy_backUp备份
						}
						//backUpBranchVex=curNode;
						/*真分支*/
						integerOperationType.treeValue = TV_TRUE; 
						visitVex(curNode,preNode,pawalker,beingAnalysisPathIndex,
							curNode->getFirstOutArc(),
							true,theRestPathSize,isInterprocedural,integerOperationType);
						//if( 1 && pawalker.findSensitiveSink)//发现一个sink点，且发现错误，该路径不再继续向后走，直接返回
						if( 1 && getTainedSinkFlag4VexNode())//发现一个sink点，且发现错误，该路径不再继续向后走，直接返回
						{
							  openConsole();
							  cout<<"tained sink triggered, quit current path...."<<endl;
							  closeConsole();
							  --theRestPathSize;
							  recordOnePathInfo(pawalker);
							  nodestack.popMore(iPushNodeCount);
							  recoverSinkFlag4VexNode();//sink结点只在报错的一条路径上将语句截断，不影响一个函数内的其他路径
							  return;
						}
#if Branch_Predict 
						if((!g_project_use_condition_check) || (TV_FALSE != integerOperationType.treeValue)){//预测的true分支可行或不确定，继续分析当前路径的后续节点
#else
						if(true||TV_FALSE != integerOperationType.treeValue){//预测的true分支可行或不确定，继续分析当前路径的后续节点
#endif
							  analyzeNode(arcTrue->getTailVex(),curNode,NULL,isInterprocedural); 
#if Branch_Predict
							   if((g_project_use_condition_check)&&TV_TRUE == integerOperationType.treeValue)//如果可以确定条件为真，那么不需要走false分支
#else
							   if(false&&TV_TRUE == integerOperationType.treeValue)//如果可以确定条件为真，那么不需要走false分支
#endif
							  {
								    --theRestPathSize;
									nodestack.popMore(iPushNodeCount);
									return;
							  }
						}
			}
			/*假分支*/

			/*if分支试探着走false分支，而circle节点只在两种情况下走假分支：（1）直接跳过循环体的路径（2）循环了规定次数，那么从false分支
			跳出*/
			if(((vDoWhile == nodeType || vCircle == nodeType) && (g_project_loop_check_count  < backUpCircleNum|| 1 == backUpCircleNum))
				  || (vIf == nodeType || vSwitch == nodeType)
				  ){
						/*if分支节点走假分支时需要恢复信息；循环节点在直接走false分支时需要恢复信息*/
						if(vIf == nodeType || vSwitch == nodeType 
							  || ((vCircle == nodeType || vDoWhile == nodeType) && 1 == backUpCircleNum)
							  ){
									//恢复分支节点信息，更新bdd中有关路径编号的信息
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

									//xxy_backUp恢复
						}
						if((vCircle == nodeType || vDoWhile == nodeType) && 1 != backUpCircleNum)//强制从循环的false分支跳出
						{
							  integerOperationType.treeValue = TV_MUST_RETURN_TRUE; 
						}else{
							  integerOperationType.treeValue = TV_FALSE; 
						}
						visitVex(curNode,preNode,pawalker,beingAnalysisPathIndex,
							  curNode->getFirstOutArc(),
							  true,theRestPathSize,isInterprocedural,integerOperationType);

						//if( 1 && pawalker.findSensitiveSink)//发现一个sink点，且发现错误，该路径不再继续向后走，直接返回
						if( 1 && getTainedSinkFlag4VexNode())//发现一个sink点，且发现错误，该路径不再继续向后走，直接返回
						{
							  openConsole();
							  cout<<"tained sink triggered, quit current path...."<<endl;
							  closeConsole();
							  --theRestPathSize;
							  nodestack.popMore(iPushNodeCount);
							  recoverSinkFlag4VexNode();//sink结点只在报错的一条路径上将语句截断，不影响一个函数内的其他路径
							  return;
						}
#if Branch_Predict
						if((!g_project_use_condition_check)||TV_FALSE != integerOperationType.treeValue){//预测的路径可行或不确定，继续分析当前路径的后续节点
#else
							if(true||TV_FALSE != integerOperationType.treeValue){//预测的路径可行或不确定，继续分析当前路径的后续节点
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
	  ConditionTreeWalker con_treewalker(&(pawalker.mypa), pPreOutValue); //第一个参数存着指针的信息，第二个参数存着整型的信息
	  CFGNode* p=curNode->getData();
	  RefAST ast=p->getAST();
	  ConstraintResultType type=con_treewalker.start(ast);
	  switch(type)
	  {
	  case conflict: 
			//returnvalue=false;        //终止当前路径分析，从下一条路径开始分析
			returnType=FalsePath;
			break;
	  case satiable:
			//returnvalue=true;  
			returnType=TruePath;//继续分析
			break;
	  default: // case uncertain
			//产生新的信息流
			//returnvalue=true;         //继续分析
			returnType=BothPath;
	  }
	  //if(vCircle==curNode->getNodeType())
	  //{
	  //	if(FalsePath==returnType)
	  //	{
	  //		//如果当前循环条件确定为假，则直接走false分支，否则都要走
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
//		if(!switchHeadDeque.empty())//因为前端有bug,所以暂时加上这个限制条件
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
//深度优先遍历每个函数的结点(搭框架之前的备份)
//void DataFlowAnalyzer::analyze(int isInterprocedural,int deep)
//{
//	if(!needAnalysis())	//当前函数条目已在栈中
//	{
//		return;
//	}
//	pushCallStack();// 将当前函数条目压入函数调用栈
//	
//	if(m_func)
//	{
//		if(isInterprocedural)
//			depth<<"被调用函数："<<m_func->getName()<<endl;
//		else
//			depth<<"入口函数："<<m_func->getName()<<endl;
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
//	static int pathCnt=1;//用来判断当前有几条路径要分析
//	//bool bFinalPath=false;
//	while(curNode!=NULL&&curNode->getOutNum()<=1)
//	{
//		depth<<*curNode<<endl;
//		if(curNode->getLocalIndex()==1)
//		{
//			pathCnt--;
//			if(0==pathCnt)
//			{
//				depth<<"pathCnt==0 遍历结束！"<<endl;
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
//			m_treeWalker->setbDecidePathToTravel(true);//设置开关，第一次遍历这个节点，尽量确定要走的路线
//			//m_treeWalker->expressionTree((RefMyAST)(curNode->getData()->getAST()),MingledSymbolItem());
//			m_treeWalker->walkAstOnVex(curNode,preNode);
//			m_treeWalker->setbDecidePathToTravel(false);
//			m_treeWalker->setVexIsCircleHead(true);//猜值开关
//			m_treeWalker->walkAstOnVex(curNode,preNode);
//		}
//
//		if(m_treeWalker->pathToTravel==pathTrue)
//		{
//			depth<<"分支："<<*curNode<<"只需遍历true分支"<<endl;
//			m_treeWalker->walkAstOnVex(curNode,preNode);
//			analyzeNode(arcTrue->getTailVex(),curNode,NULL);
//		}else if(m_treeWalker->pathToTravel==pathFalse){
//			depth<<"分支："<<*curNode<<"只需遍历false分支"<<endl;
//			m_treeWalker->walkAstOnVex(curNode,preNode);
//			analyzeNode(arcFalse->getTailVex(),curNode,NULL);
//		}else{
//			depth<<"分支："<<*curNode<<"两个分支都需遍历"<<endl;
//			pathCnt++;
//			m_treeWalker->walkAstOnVex(curNode,preNode);
//			analyzeNode(arcTrue->getTailVex(),curNode,NULL);
//			analyzeNode(arcFalse->getTailVex(),curNode,NULL);
//		}
//		//pathCnt++;
//		//depth<<"分支："<<*curNode<<endl;
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
//					depth<<"分支："<<*curNode<<endl;
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
//		depth<<"分支："<<*curNode<<endl;
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
//	static int pathCnt=-1;//路径编号计数器，别名分析中用在路径域
//	static int analyzeBranch=false;//该标志为true时，分析分支节点而不递归，防止出现死循环
//	int curPath;
//	VexNode *pre=preVode;
//	//存储当前单一路径的信息，稍后合并到IFCollector
//	bdd ccaPath=bddfalse;
//	bdd ccBranch=bddfalse;
//	curPath=++pathCnt;//当前路径编号
//	while(curNode!=NULL)
//	{
//		if(curNode->getOutNum()<2||(2==curNode->getOutNum()&&analyzeBranch))
//		{
//			if(2==curNode->getOutNum()&&arc->getEdgeType()==eTrue&&findNodeInNodeStack(curNode))//节点已经在栈中，说明是一个循环的尾结点，直接跳过，进入false分支
//			{
//				//??完全放弃该结点的分析？？
//				analyzeBranch=false;
//				pre=curNode;
//				ArcBox *myarc=curNode->getFirstOutArc();
//				while(myarc->getEdgeType()!=eFalse)
//					myarc=myarc->getNextEdgeWithSameHeadVex();
//				if(myarc->getEdgeType()!=eFalse)
//					depth<<"寻找错误路径出错"<<endl;
//				curNode=myarc->getTailVex();
//				continue;
//			}
//			curNode->addAnalysisNum();//该结点分析次数加一
//			if(analyzeBranch)
//			{
//				analyzeBranch=false;
//			}
//
//			depth<<*curNode<<endl;
//			
//			//收集当前单个结点信息
//			bool isTrue=false;
//			if(2==curNode->getOutNum())
//				isTrue=isTrueEdge(curNode,arc->getTailVex());
//			handleVexState(curNode,false,pawalker,m_treeWalker,isTrue);	
//			travelVex(curNode,false,pawalker,m_treeWalker,isTrue);//暂未处理	if( true == isFirstNodeOnPath)
//				//判断是否是循环,用栈
//			m_treeWalker->walkAstOnVex(curNode, preVode);
//			//收集当前单个结点信息 end
//			if(1==curNode->getLocalIndex())//end 结点
//			{
//				depth<<*curNode<<endl;
//				//一条路径走完，需要收集所需的信息
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
//			ccaPath=IFCollector->CCAPath;//备份当前路径分支节点前的信息流信息
//			ccBranch=IFCollector->CCBranch;
//			analyzeBranch=true;
//			ArcBox *arc=curNode->getFirstOutArc();
//			analyzeNode(curNode,pre,arc);
//			analyzeBranch=true;
//			/*
//			//还原分支节点处信息
//			//??把在P域中但不再temp_pointto中的数据移除
//			bdd temp_pointto=pawalker.mypa.pointtoCOut&fdd_ithvar(C, pre->getGlobalIndex())&fdd_ithvar(P,curPath+1); //xulei modify, 20100412.  vexCnt=>vexCnt-1
//			//？？将分支结点前一个结点指向信息复制到当前要分析的路径
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
//			//还原分支节点处信息end*/
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
// 该方法用来对函数条目进行数据流分析
void DataFlowAnalyzer::analyze(int isInterprocedural)	
{	

	  string sCurFuncName = "<NULL>";
	  //dzh
	  //streambuf* pOld = cout.rdbuf(DzhTest.rdbuf());
	  //end
	  if(!needAnalysis())						// 如果当前函数条目不需要分析则返回
	  {
			return;
	  }
	  pushCallStack();						// 将当前函数条目压入函数调用栈

	  //2010-9-16,ZLY,BEGIN,分析路径列表中放入当前分析路径对象
	  gp_CurPath = m_callStack.getCurPathInfo();
	  //2010-9-16,ZLY,BEGIN,分析路径列表中放入当前分析路径对象

	  currFuncItem=m_callStack.top();

	  if (currFuncItem)
	  {
			cout.rdbuf(pOld);
			if(isInterprocedural){
				  int iCallNum = m_callStack.getStackSize();
				  for(int i=0; i<iCallNum; i++)
						cout<<"  ";
				  cout<<"正在分析被调用函数"<<currFuncItem->getName()<<"..."<<endl;	
			}
			/*
			else
			cout<<endl<<"正在分析入口函数"<<currFuncItem->getName()<<"..."<<endl;	
			*/
			if(isInterprocedural)
				  func<<"正在分析被调用函数"<<currFuncItem->getName()<<"..."<<endl;	
			else
				  func<<"正在分析入口函数"<<currFuncItem->getName()<<"..."<<endl;	
			cout.rdbuf(DzhTest.rdbuf());	 
#ifdef PA_TRACE_LITTLE
			sCurFuncName = currFuncItem->getName();
			if(isInterprocedural)
				  paTrace<<"正在分析被调用函数"<<sCurFuncName<<"..."<<endl;
			/*
			else
			paTrace<<"正在分析入口函数"<<sCurFuncName<<"..."<<endl;	
			*/
#endif
	  }

	  time_t begin, end;
	  time(&begin);

	  //dataflow<<getCallStack();				// 打印调用栈, 暂保留 			
	  CFGPathsManager* pathsMgr = CFGPathsManager::instance();	// 获得路径管理器实例
	  CFGPaths& paths = pathsMgr->pathsOfFunction(m_func);		// 从路径管理器中获取待分析函数的路径
	  dataFlowPaths[m_func] = paths;  // 20100106 by kong
	  //dataflow << dataFlowPaths[m_func] << "kongdelan " << endl; // 20100106 by kong
	  //dataflow<<paths;							// 打印所有路径
#if futeng_path_output
	  futeng_path<<"所有路径：\n"<<paths<<endl;
#endif
	  if(gb_OnlyOneEntry && isInterprocedural == false){
			int i;
			gl_TotalEntryNodes = 0;
			for(i=0; i<paths.size(); i++)
				  gl_TotalEntryNodes += paths[i]->length();
			gl_CurrentAnalyzedNodes = 0;
	  }
	  //	Path prePath, curPath;						// 分别用来保存前一条路径和当前路径的节点队列
	  //cout << "current function is: " << paths.getFunc()->getName() << endl;

	  Path *prePathPtr=0, *curPathPtr=0;
	  int pathCnt=0;
	  bdd temp=bddfalse;//cxx
	  //	while(paths.getNextPath(curPath, pathCnt))	// 从所有路径中不停的获取下一条路径进行分析

	  int allNodesNum = getAllNodesNum(paths);    //得到所有节点个数，该函数用于进度条控制，没有别的用途
	  int oneNode=0;    //当前第几个节点，用于控制条控制

	  //IFCollector->CCAFunc = bddfalse;//IFCollector全局变量
	  bool bFinalPath = false;
	  processbar_Cur.setCurrentAnalysisInfo(string("函数 ") + sCurFuncName, paths.size(), pathCnt);

	  pawalker.setAnalyzer(this);
	  while(curPathPtr=paths[pathCnt])
	  {
			cout.rdbuf(pOld);
			if(isInterprocedural){
				  int iCallNum = m_callStack.getStackSize();
				  for(int i=0; i<iCallNum; i++)
						cout<<"  ";
				  cout<<"  正在分析函数 "<<sCurFuncName <<" 的第("<<pathCnt+1<<"/"<<paths.size()<<")条路径..."<<endl;	
			}
			else
				  cout<<"  正在分析函数 "<<sCurFuncName <<" 的第("<<pathCnt+1<<"/"<<paths.size()<<")条路径..."<<endl;	
			cout.rdbuf(DzhTest.rdbuf());	 

			//2010-08-12,ZLY,BEGIN,判断是否是最后一条路径
			if(pathCnt == paths.size() - 1)
				  bFinalPath = true;
			//2010-08-12,ZLY,END

#ifdef PA_TRACE
			paTrace<<"Begin a new path:"<<endl;
#endif
#ifdef PA_TRACE_LITTLE
			paTrace<<"\t2010-08-11,ZLY, 分析第"<<pathCnt+1<<"条路径:"<<endl;
			paTrace<<"\t\t"<<(*curPathPtr)<<endl;
#endif
			//2010-08-11,ZLY,BGING,重置所有结点的分析标记位为false
			curPathPtr->resetAllNodeFlag();
			//2010-08-11,ZLY,END
			NodeList.clear();           //每次从一条新路径开始分析，节点队列清空

			//2010-9-16,ZLY,BEGIN,处理当前函数路径信息
			gp_CurPath->clear();
			//2010-9-16,ZLY,BEGIN,处理当前函数路径信息


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
			//2010-08-23,ZLY,这样做有问题，具体见函数GetMaxSameSubPath处的说明
			/*
			//2010-08-22,ZLY,BEGIN,查找前面路径的最大相同路径
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
			for(int i = 0; i<vexCnt; i++) {						// 打印出前一路径公共部分的数据流信息
			dataflow<<" *VexNode "<<(*curPathPtr)[i]->getLocalIndex()<<": "
			<<(*curPathPtr)[i]->inValues<<endl;
			NodeList.push_back((*curPathPtr)[i]->getLocalIndex());     //开始部分的公共节点先加入到节点队列中
			IFCollector->CCAPath |= IFCollector->CCAFunc & fdd_ithvar(C, (*curPathPtr)[i]->getGlobalIndex());
			}
			*/
			//cout << "IFCollector->CCAFunc: " << IFCollector->CCAPath << endl;

			VexNode* preVex, *curVex;
			VexNode* tmpVex = NULL;
			//cxx
			//xulei 修改原因，因为是从分叉节点开始分析，所以需要得到分叉节点的上一个节点的Out信息，而不是分叉节点的Out信息。
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
						paTrace<<"2010-08-13,ZLY,程序错误：未找到当前路径上不存在结点下标"<<vexCnt-1<<""<<endl;
				  }else{
						paTrace<<"\t\t2010-08-13,ZLY, 取第历史路径上结点"<<tmpVex->getLocalIndex()<<"的历史分析结果..."<<endl;
				  }
#endif
				  Path * targetPath;
				  //2010-08-11,ZLY,找到之前路径中最近一个被分析的目标结点，取出其指向信息
				  int iTargetPathIndex = pathCnt - 1;
				  // 			int iTargetPathIndex = pathCnt-1;
				  while(iTargetPathIndex >= 0){
						targetPath = paths[iTargetPathIndex];
						if(targetPath == NULL){
							  paTrace<<"2010-08-11,ZLY,程序错误：未找到历史分析结果（路径为空）!"<<endl;
							  iTargetPathIndex = -1;
							  break;
						}
						tmpVex = (*targetPath)[vexCnt-1];
						if(targetPath->getAnalyzedFlag(vexCnt-1) == Analyzed)
							  break;
						if(targetPath->getAnalyzedFlag(vexCnt-1) == NotReachable){
							  if(tmpVex == NULL){
									paTrace<<"2010-08-11,ZLY,程序错误：第"<<iTargetPathIndex+1<<"条路径上不存在结点下标:"<<vexCnt-1<<endl;
							  }else{
#ifdef PA_TRACE_LITTLE
									paTrace<<"\t\t2010-08-11,ZLY, 发现第"<<iTargetPathIndex+1<<"条路径结点"<<tmpVex->getLocalIndex()<<"不可达未分析!"<<endl;
#endif
							  }
							  iTargetPathIndex = -1;
							  break;
						}
						if(tmpVex == NULL){
							  paTrace<<"2010-08-11,ZLY,程序错误：第"<<iTargetPathIndex+1<<"条路径上不存在结点下标:"<<vexCnt-1<<endl;
						}else{
#ifdef PA_TRACE_LITTLE
							  paTrace<<"\t\t2010-08-11,ZLY, 发现第"<<iTargetPathIndex+1<<"条路径结点"<<tmpVex->getLocalIndex()<<"未分析过!"<<endl;
#endif
						}
						iTargetPathIndex--;
				  }

				  if(iTargetPathIndex < 0){
						/*
						VexNode* tmpCurVex = (*curPathPtr)[vexCnt-1];
						if(tmpCurVex != NULL)
						paTrace<<"2010-08-11,ZLY,程序错误：未找到结点"<<tmpCurVex->getGlobalIndex()<<"的历史分析结果!"<<endl;
						else
						paTrace<<"2010-08-11,ZLY,程序错误：当前路径(第"<<pathCnt+1<<"条)上不存在下标为"<<vexCnt-1<<"的结点!"<<endl;
						prePathPtr = curPathPtr;
						pathCnt++;
						continue;
						*/
						//2010-08-13,ZLY,BEGIN,不能从头开始分析，要取函数入口结点(0)的历史分析结果，
						//                     否则路径开始是状态不是函数入口处的状态
						/*
						paTrace<<"\t\t2010-08-11,ZLY,未取得历史分析结果，本条路径从开始处分析!"<<endl;
						vexCnt = 0;
						//2010-08-10,ZLY,下面使用temp而不是temp_pointto会不会有问题???
						temp=pawalker.mypa.pointtoCOut&fdd_ithvar(C, (*curPathPtr)[vexCnt]->getGlobalIndex())&fdd_ithvar(P,pathCnt); 
						//temp_CovertChannelAAll=pawalker.mypa.CovertChannelAAllCout&fdd_ithvar(C, (*curPathPtr)[vexCnt]->getGlobalIndex())&fdd_ithvar(P,pathCnt);
						temp_vheap=pawalker.mypa.v_heapCOut&fdd_ithvar(C, (*curPathPtr)[vexCnt]->getGlobalIndex())&fdd_ithvar(P,pathCnt);
						*/
						paTrace<<"\t\t2010-08-11,ZLY,未取得历史分析结果，本条路径从开始结点后开始分析!"<<endl;
						vexCnt = 1;
						iTargetPathIndex = 0;
						targetPath = paths[iTargetPathIndex];//取出第1条路径
						if(targetPath == NULL){
							  paTrace<<"2010-08-13,ZLY,程序错误，未找到第1条路径（路径为空）! 放弃当前路径分析!"<<endl;
							  prePathPtr = curPathPtr;
							  pathCnt++;
							  continue;
						}
						if(targetPath->getAnalyzedFlag(vexCnt-1) != Analyzed){
							  paTrace<<"2010-08-13,ZLY,程序错误，发现第1条路径结点0未分析过! 放弃当前路径分析!"<<endl;
							  prePathPtr = curPathPtr;
							  pathCnt++;
							  continue;
						}

						//2010-08-13,ZLY,END
				  }
				  //2010-08-13,ZLY,BEGIN,若前面发现没找到历史分析结果，则会取第一条路径结点(0)的历史分析结果
				  //                     因此必定能取历史分析结果
				  //else
				  //2010-08-13,ZLY,END
				  {
#ifdef PA_TRACE
						paTrace<<"\t\tiTargetPathIndex="<<iTargetPathIndex<<endl;
						paTrace<<"\t\tvexCnt="<<vexCnt<<endl;
#endif
						for(int i = 0; i<vexCnt; i++) {						// 打印出前一路径公共部分的数据流信息
							  dataflow<<" *VexNode "<<(*curPathPtr)[i]->getLocalIndex()<<": "
									<<(*curPathPtr)[i]->inValues<<endl;
							  NodeList.push_back((*curPathPtr)[i]->getLocalIndex());     //开始部分的公共节点先加入到节点队列中

							  //2010-9-16,ZLY,BEGIN,处理当前函数路径信息
							  gp_CurPath->push_back((*curPathPtr)[i]->getLocalIndex());
							  //2010-9-16,ZLY,BEGIN,处理当前函数路径信息

							  //IFCollector->CCAPath |= IFCollector->CCAFunc & fdd_ithvar(C, (*curPathPtr)[i]->getGlobalIndex());
						}

						tmpVex = (*(paths[iTargetPathIndex]))[vexCnt-1];
						//				if(tmpVex == NULL){
						//					paTrace<<"2010-08-11,ZLY,程序错误(2)：第"<<iTargetPathIndex+1<<"条路径上不存在结点下标:"<<vexCnt-1<<endl;
						//				}else{
						//#ifdef PA_TRACE_LITTLE
						//					paTrace<<"\t\t2010-08-11,ZLY, 取第"<<iTargetPathIndex+1<<"条路径结点"<<tmpVex->getLocalIndex()<<"的历史分析结果..."<<endl;
						//#endif
						//				}
						giHistoryPathVexCount += vexCnt;
						//temp_pointto=pawalker.mypa.pointtoCOut&fdd_ithvar(C, (*curPathPtr)[vexCnt-1]->getGlobalIndex())&fdd_ithvar(P,iTargetPathIndex+1); //xulei modify, 20100412.  vexCnt=>vexCnt-1
						//temp_CovertChannelAAll=pawalker.mypa.CovertChannelAAllCout&fdd_ithvar(C, (*curPathPtr)[vexCnt-1]->getGlobalIndex())&fdd_ithvar(P,iTargetPathIndex);
						//temp_vheap=pawalker.mypa.v_heapCOut&fdd_ithvar(C, (*curPathPtr)[vexCnt-1]->getGlobalIndex())&fdd_ithvar(P,iTargetPathIndex+1);
#ifdef PA_TRACE
						paTrace<<"\t\tfuteng取出结果  temp_pointto is："<<endl;
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
			//	//2010-08-13,ZLY,每两条路径至少前两个结点是相同的，因此不可能出现vexCnt等于0的情况
			//	if(prePathPtr){//当前不是第一条路径
			//		paTrace<<"2010-08-13,ZLY,程序错误，发现路径异常没有相同部分! 放弃当前路径分析!"<<endl;
			//		prePathPtr = curPathPtr;
			//		pathCnt++;
			//		continue;
			//	}
			//	//2010-08-13.ZLY
			//	//2010-08-10,ZLY,下面使用temp而不是temp_pointto会不会有问题???
			//    temp=pawalker.mypa.pointtoCOut&fdd_ithvar(C, (*curPathPtr)[vexCnt]->getGlobalIndex())&fdd_ithvar(P,pathCnt); 
			//          //temp_CovertChannelAAll=pawalker.mypa.CovertChannelAAllCout&fdd_ithvar(C, (*curPathPtr)[vexCnt]->getGlobalIndex())&fdd_ithvar(P,pathCnt);
			//    temp_vheap=pawalker.mypa.v_heapCOut&fdd_ithvar(C, (*curPathPtr)[vexCnt]->getGlobalIndex())&fdd_ithvar(P,pathCnt);
			//}

			if (temp_pointto!=bddfalse)
			{
				  //temp_pointto=bdd_exist(temp_pointto,fdd_ithset(P));//??把在P域中但不再temp_pointto中的数据移除
				  //pawalker.mypa.pointtoCOut|=temp_pointto&fdd_ithvar(P,pathCnt+1);//？？将分支结点前一个结点指向信息复制到当前要分析的路径

				  //temp_CovertChannelAAll=bdd_exist(temp_CovertChannelAAll,fdd_ithset(P));
				  //pawalker.mypa.CovertChannelAAllCout|=temp_CovertChannelAAll&fdd_ithvar(P,pathCnt+1);

				  //temp_vheap=bdd_exist(temp_vheap,fdd_ithset(P));
				  //pawalker.mypa.v_heapCOut|=temp_vheap&fdd_ithvar(P,pathCnt+1);
			}


#ifdef PA_TRACE
			paTrace<<"\tpointto is:"<<endl<<"\t\t"<<fddset<<pawalker.mypa.pointto<<endl;
			paTrace<<"\tv_heapCOut is:"<<endl<<"\t\t"<<fddset<<pawalker.mypa.v_heapCOut<<endl;
#endif

			bool is_feasible_path=true; //是否是可达路径
			//cxx end

			if(gb_OnlyOneEntry && isInterprocedural == false){
				  if(vexCnt){
						gl_CurrentAnalyzedNodes += vexCnt;
						processbar.setBcck_OneFunction(gl_CurrentAnalyzedNodes, gl_TotalEntryNodes);
				  }
			}

			while(curVex = (*curPathPtr)[vexCnt])						// 从当前路径与前一路径的非公共部分开始, 分析每个节点
			{
				  /*
				  cout.rdbuf(pOld);
				  cout<<".";	
				  cout.rdbuf(DzhTest.rdbuf());	 
				  */

				  NodeList.push_back(curVex->getLocalIndex());

				  //2010-9-16,ZLY,BEGIN,处理当前函数路径信息
				  gp_CurPath->push_back(curVex->getLocalIndex());
				  //2010-9-16,ZLY,BEGIN,处理当前函数路径信息

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
						//2010-08-11,ZLY,不论是否可达，均处理分支结点的状态保存与恢复
						handleVexState(curVex,isFirstNodeOnPath,pawalker,m_treeWalker,isTrue);
						//2010-08-11,ZLY,END
						if (is_feasible_path)   //如果当前路径是可达路径，则分析节点
							  travelVex(curVex,isFirstNodeOnPath,pawalker,m_treeWalker,isTrue);
						smNode_end = clock();
						func << "SM结点：" << smNode_end - smNode_begin << endl;
						if( true == isFirstNodeOnPath) 
							  isFirstNodeOnPath = false;
						//dzh end
				  }
				  //paTrace<<"After SM"<<endl;

				  // cxx
				  if (!isInterprocedural)   //不是被调函数内的节点，需要做进度条控制
				  {
						++oneNode;
						processbar.setBackOne(oneNode, allNodesNum);
				  }

				  //paTrace<<"Before get int value"<<endl;
				  m_treeWalker->setVexIsCircleHead(curPathPtr->vexIsCircleHead(curVex));
				  m_treeWalker->walkAstOnVex(curVex, preVex);	
				  //			infoWalker->walkAstOnVex(curVex, preVex);// 遍历节点对应的AST, 分析结果挂到节点上，做整型值的数据流分析
				  //paTrace<<"After get int value"<<endl;


				  time_t node_begin, node_end;
				  node_begin=clock();
				  //if(!visitVex(curVex,vexCnt,preVex,pawalker,pathCnt+1,(*curPathPtr).getSwitches(vexCnt),(*curPathPtr), true))
				  //if(!visitVex(curVex,vexCnt,preVex,pawalker,pathCnt+1,(*curPathPtr).getSwitches(vexCnt),(*curPathPtr), is_feasible_path, bFinalPath, isInterprocedural))
				  //{//返回值是false，表示当前路径是不可达路径，从下一条路径开始分析
				  //	if(is_feasible_path == true){
				  //		paTrace<<"\t\t发现一条不可达路径，从结点 ";
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
				  //		paTrace<<" 开始"<<endl;
				  //	}
				  //	xulei_dataflow<<"找到一条不可达路径"<<endl;
				  //	//2010-08-10,ZLY, 不分析路径的处理存在BUG，会导致分支结点进入栈，但是程序状态未保存
				  //	is_feasible_path=false;
				  //	curPathPtr->setAnalyzedFlag(vexCnt, NotReachable);
				  //	//2010-08-10,ZLY, END
				  //    //break;
				  //}else{
				  //	//2010-08-11,ZLY,BGING,设置当前结点的分析标记位为Analyzed
				  //	curPathPtr->setAnalyzedFlag(vexCnt, Analyzed);
				  //	//2010-08-11,ZLY,END
				  //}
				  node_end=clock();
				  func<<"结点号:"<<curVex->getLocalIndex()<<",总时间："<<node_end-node_begin<<endl;
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
			//	prePath=curPath;		// 当前路径成为前一条路径, 进入下一轮循环
			prePathPtr = curPathPtr;
			pathCnt++;
			processbar_Cur.setCurrentAnalysisInfo(string("函数 ") + sCurFuncName, paths.size(), pathCnt);
	  } //.while(paths.getNextPath(curPath, pathCnt))
	  popCallStack();				// 分析结束, 当前函数弹栈


	  /*2010-08-27,不能再取栈顶，也不需要取，因为每次分析一个函数都会压栈并置currFuncItem,
	  如果弹栈后再取会导致栈为空时仍然取top，而在CallStack类中并没有判断使用的list是否为空
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
	  func<<"时间："<<diff<<endl;

#ifdef PA_TRACE_LITTLE
	  paTrace<<"分析函数"<<sCurFuncName<<"结束"<<endl;	
#endif
	  cout.rdbuf(pOld);
	  if(isInterprocedural){
			int iCallNum = m_callStack.getStackSize() + 1;
			for(int i=0; i<iCallNum; i++)
				  cout<<"  ";
			cout<<"分析被调用函数函数"<<sCurFuncName<<"结束"<<endl;	
	  }
	  else
			cout<<"分析入口函数函数"<<sCurFuncName<<"结束"<<endl;	
	  pOld = cout.rdbuf(DzhTest.rdbuf());	 

	  //	cout.rdbuf(pOld);
	  //信息流已经合并 王运 2009-09-18
	  //cout<<"111111111111111111111"<<pawalker.mypa.pointtoCOut<<endl;
}  

//2010-08-11,ZLY,BEGIN:处理分支结点上的状态保存与恢复
//这个函数体中的代码原来位于travelVex的开始部分
void DataFlowAnalyzer::handleVexState(VexNode* v,bool isFirstNodeOnPath,PATreeWalker& pawalker,ExprValueTreeWalker* expTreewalker,bool IsTrueEdge)
{
	  //	cout<<"dzh...................1"<<endl;
	  //如果是分支节点，同时是该分支节点已经遍历过，即该分支节点的In信息已经存储
	  //	if (isFirstNodeOnPath && v->pInfo != NULL)
	  if(2==v->getAnalysisNum()&&2==v->getOutNum()&&v->pInfo!=NULL)
	  {
			AllStateInfo* pState = static_cast<AllStateInfo*>(v->pInfo);
			map<SymbolItem*,list<AttachToSymbolItem*>*> symbolState = pState->symbolStates;
			map<_SM*,list<State*> > virtualState = pState->virtualStates;		
			//清空全局状态集合
			ProgramState.Symbol_list.clear();
			ProgramState.Virtual_Set.clear();
			//拷贝分支节点信息到符号条目上(该类信息是绑定到符号条目上的信息)
			map<SymbolItem*,list<AttachToSymbolItem*>*>::iterator iter1 = symbolState.begin();
			while(iter1 != symbolState.end())
			{
				  ProgramState.Symbol_list.push_back((*iter1).first);

				  //释放挂接在符号条目上的状态信息
				  list<AttachToSymbolItem*>* tmp = (list<AttachToSymbolItem*>*)((*iter1).first->getSM_State());
				  list<AttachToSymbolItem*>::iterator iter_att = tmp->begin();
				  while(iter_att != tmp->end())
				  {
						//	delete (*iter_att);
						++iter_att;
				  }
				  //将保存在分支节点的状态信息挂接到符号条目上
				  ((*iter1).first)->setSM_State((*iter1).second);
				  ++iter1;
			}

			//拷贝分支信息到维持全局程序状态的对象
			//因为不绑定到变量的那些状态无法挂接到符号条目上，因此用一个全局集合来维持

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
			//当前节点是分支节点，同时还没遍历过
			//如果该节点有2条出边，那么该节点为分支节点，保存程序信息。
			//非分支节点信息暂时不考虑，对于当前程序分析，非分支节点的In,Out信息没有使用
			//如果对每个节点的In,Out信息都进行存储的话，开销太大了，而这只存储有用的In,Out信息
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

				  //更新当前绑定到程序变量的状态信息，
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

				  //更新当前程序未绑定到变量状态的信息
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

	  //对每个节点(即每个语句)，用所设定的安全规则对其检查。
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
list<_SM*>					Sm_list;	    //维系当前自动机集合。
list<SymbolItem*>			Symbol_list;	//维系当前程序点处条目集合(该集合中的条目已经发生状态的变化)											经发生状态的变化)
map<_SM*,list<SM_State*> >	Virtual_Set;	//不绑定到变量，只是维系程序状												态，每个自动机都有这样子一个集合。
}
*/
// (分析开始前)设置待分析函数CFG的入口流值
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

// (分析结束后)获取被分析函数的CFG的出口流值
VexValueSet DataFlowAnalyzer::getExitValues()
{
	  CFG* cfg = 0;
	  if(!m_func || !(cfg = (CFG*)(m_func->getCFG())) ) return VexValueSet();
	  VexNode* ex = cfg->getEnd();
	  return ex->outValues;
}

// 当前分析的函数条目压入函数调用栈
void DataFlowAnalyzer::pushCallStack()
{
	  m_callStack.push(m_func);
}

// 从函数调用栈中弹出栈顶条目
void DataFlowAnalyzer::popCallStack()
{
	  m_callStack.pop();
}

// 获取函数调用栈	
/*
inline CallStack DataFlowAnalyzer::getCallStack()
{
return m_callStack;
}
*/

// 是否需要对函数条目进行分析
bool DataFlowAnalyzer::needAnalysis()
{
	 /* cout<<"进入这里！！"<<endl;*/

	  return (!(m_callStack.find(m_func)));
}
/*一条路径分析完毕，记录该路径的指向信息和堆空间分配信息*/
bool DataFlowAnalyzer::recordOnePathInfo(PATreeWalker& pawalker)
{
	SymbolItem* analyzingSbl = DataFlowAnalyzer::m_callStack.top();
	if(NULL != analyzingSbl){
		int id = analyzingSbl->getSymbolID();
		if(bddfalse != pawalker.mypa.pointto){
			pawalker.mypa.pointtoCOut |= pawalker.mypa.pointto & fdd_ithvar(M1,id);    //该路径最后一个节点的ID与对应的指向信息和堆空间分配信息
		}
		if(bddfalse != pawalker.mypa.v_heap){
			pawalker.mypa.v_heapCOut |= pawalker.mypa.v_heap & fdd_ithvar(M1,id);
		}
	}
	return true;
}

// DAY FOR CXX BEGINvisitVex(VexNode* v,PATreeWalker& pawalker ,int Paths,deque<int> dq,Path& curPath )
bool DataFlowAnalyzer::visitVex(VexNode* v, /*int curVex_id*/ VexNode* preVex , PATreeWalker& pawalker ,int Paths,/*const deque<int>& dq,*/ArcBox *arc, bool is_reachable, int pathSize, bool isInterprocedural,
	  ValueParameter& judgeType//整形分析模块需要做的操作
	  )
{ 
	/*
	//自动机luxiao
	//标记污染源BEGIN

	MarkTainedVar(v);

	//标记污染源END
	handleVexState(v,false,pawalker,NULL,false);
	travelVex(v,false,pawalker,NULL,false);
	//自动机end
	 */
	  bTainedSinkFlag4VexNode = false;
	  pawalker.analyzingNodeType=v->getNodeType();
	  bool returnvalue=true;         //add by xulei, 指示当前路径是否是可达或者不可达，返回值为false表示不可达
	  pawalker.mypa.VexMethod |= fdd_ithvar(M1, m_func->getSymbolID())&fdd_ithvar(C,v->getGlobalIndex());

		if(v->getLocalIndex()==1)//在函数结尾出进行 信息合并操作
		{
// 		  openConsole();
// 		  cout<<"一条路径分析完毕，信息收集..."<<endl;
// 		  closeConsole();
#if  futeng_path_output
			futeng_path<<"一条路径分析完毕，信息收集..."<<endl;
#endif
			//2010-08-12,ZLY,BEGIN,最后一条路径进行信息汇总，入口函数的路径可达时进行资源泄漏检查
			if(is_reachable == true && isInterprocedural == false){  //isInterprocedural:在最外层被置为false
				  bool bFinalPath=false;
				  if(1==pathSize)
				  {
						bFinalPath=true;
				  }
				  /* #20160615
				  outputFinalError(pawalker, preVex, Paths, bFinalPath);//检查当前路径资源泄漏
				  */
				  outputFinalErrorByVHeap(pawalker);
			}

			/*一条路径分析完毕，记录下其指向信息和堆空间的信息，以该函数的id作为标记*/
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
// 			if(1 == pathSize){//最后一条路径的最后一个结点
// 				  //function_call<<"出栈："<<m_func->getName()<<endl;
// 				  //getParentOutInfor_ALL(v,pawalker);   //将函数所有路径的信息汇总
// 			}

			if(1 == pathSize && isInterprocedural == false){
// 				openConsole();
// 				cout<<"\t\t入口函数最后一条路径结束，进行资源回收..."<<endl;
// 				closeConsole();
#if futeng_path_output
				  futeng_path<<"\t\t入口函数最后一条路径结束，进行资源回收..."<<endl;
#endif
				  releaseLeakCheckResource(pawalker); //资源回收
			}
	  }
	  else if (v->getLocalIndex()!=0)  //非尾节点，非头节点
	  {
#if 0
			//futeng modify 注释于20160612，该函数没有实际作用，反而增加了pointto里边多余的信息
			getParentOutInfor(v,preVex,pawalker,Paths);   //得到上一节点的出信息,ft未更改
#endif
	  }
	  if (!is_reachable) //该节点不可达
	  {
			//2010-08-12,ZLY,BEGIN,如当前结点不可达，则将其出信息置为空
			pawalker.mypa.pointto = bddfalse;
			pawalker.mypa.v_heap = bddfalse;
			//2010-08-12,ZLY,END

			//pawalker.mypa.pointtoCOut|=pawalker.mypa.pointto&fdd_ithvar(C,v->getGlobalIndex())&fdd_ithvar(P,Paths)&fdd_ithvar(K,pawalker.funcStack.back()->callNum);
			//pawalker.mypa.CovertChannelAAllCout |=pawalker.mypa.CovertChannelAAll&fdd_ithvar(C,v->getGlobalIndex())&fdd_ithvar(P,Paths)&fdd_ithvar(K,pawalker.funcStack.back()->callNum);
			//pawalker.mypa.v_heapCOut|=pawalker.mypa.v_heap&fdd_ithvar(C,v->getGlobalIndex())&fdd_ithvar(P,Paths)&fdd_ithvar(K,pawalker.funcStack.back()->callNum);

			//2010-08-11,ZLY,发现自己不可达，直接返回false，同一路径后续结点均不可达
			return false;
			//return returnvalue;
			//2010-08-11,ZLY,END
	  }	
	  CFGNode* cfgnode=v->getData();
	  //xulei, 201004月 add.
	  //2010-08-12,ZLY,BEING,出作用域结点要进行析构函数的分析，所以此处不能不分析，在PATreeWalker中单独处理了出作用域结点的情况
	  //2010-08-10,ZLY,过滤掉出作用域结点
	  if (cfgnode)   
			//if (cfgnode && cfgnode->getNodeType() != eScopeOutNode)   
			//2010-08-10,ZLY:END,过滤掉出作用域结点
			//2010-08-12,ZLY,END
	  {
			// 		cout<<"heap2: "<<pawalker.mypa.v_heap<<endl;
			//cfgnode->getNodeType()返回1表示是分支节点，需要在分支节点挂上AST信息
			/*
			enum CFGNodeType { eNullNode = 0, ePredicateNode = 1, eStatementNode = 2, eReturnNode = 3,eDeclaNode = 4,
			eJudgeExceptionNode = 5,eNormalExitNode = 6, eExceptionExitNode = 7, eScopeOutNode = 8,eGotoNode = 9};
			*/
#if 0
			if (cfgnode->getNodeType()==ePredicateNode)   //ePredicateNode
			{
				  RefAST ePredicateNode_ast=cfgnode->getAST();    //条件判断语句的AST
#ifdef PA_TRACE
				  paTrace<<"判断路径是否可达..."<<endl;
				  paTrace<<"当前指向信息pointto为："<<endl;
				  paTrace<<fddset<<pawalker.mypa.pointto<<endl;
				  paTrace<<"ast is:"<<endl;
				  paTrace<<ePredicateNode_ast->toRealStringTree()<<endl;
#endif
#if xulei_dataflow_output
				  xulei_dataflow<<"分支节点"<<v->getLocalIndex()<<endl<<ePredicateNode_ast->toStringTree()<<endl;
#endif
				  //xulei_dataflow<<"下一个节点："<<curPath[curVex_id+1]->getLocalIndex()<<endl;
				  CFGEdgeType edge_type = eTrue;  /*enum CFGEdgeType { eNull = 0, eTrue = 1, eFalse = 2,eException = 3};*/
				  if(NULL != arc){
					edge_type=arc->getEdgeType();
				  }
#if xulei_dataflow_output
				  xulei_dataflow<<"得到出边类型了"<<endl;
#endif
				 /* cout<<"得到出边类型了"<<endl;*/
				  if (edge_type==eFalse)   //F分支，需要将AST做一个转换，得到!AST
				  {
						ePredicateNode_ast=ReConstructAST(ePredicateNode_ast, p_parser);      //重构AST，把e转换为!e
#ifdef PA_TRACE
						paTrace<<"p_parser is :"<<p_parser<<endl;
						paTrace<<"false分支ast转换结果："<<endl<<ePredicateNode_ast->toRealStringTree()<<endl;
#endif	
#if xulei_dataflow_output
						xulei_dataflow<<"遇到F分支，重构AST："<<ePredicateNode_ast->toStringTree()<<endl;
#endif
				  }

				  /*cout<<"如果是F分支，则已经重构了"<<endl;*/

				  //对AST进行一次等价变换，处理!!e <=> e的情况
				//  visitAST(ePredicateNode_ast, p_parser);  //temp comment
#if xulei_dataflow_output
				  xulei_dataflow<<"等价转换AST："<<ePredicateNode_ast->toStringTree()<<endl<<endl;
#endif

				 /* cout<<"已经做了等价转换了"<<endl;*/

				  //VexValueSet* p_Value_set=&(preVex->outValues);   //分析完前一个节点的整型信息集(Out信息)


				  //ConditionTreeWalker con_treewalker(&(pawalker.mypa), p_Value_set); //第一个参数存着指针的信息，第二个参数存着整型的信息
				  //// 			cout<<"Walk ast:"<<endl;
				  //// 			xulei_dataflow <<"Walk ast:"<<endl;
				  //// 			cout<<ePredicateNode_ast->toRealStringTree()<<endl;
				  //// 			xulei_dataflow<<ePredicateNode_ast->toRealStringTree()<<endl;
				  //ConstraintResultType type=con_treewalker.start(ePredicateNode_ast);
				  //switch(type)
				  //{
				  //case conflict: 
				  //	returnvalue=false;        //终止当前路径分析，从下一条路径开始分析
				  //	//returnvalue=true;       
				  //	break;
				  //case satiable:
				  //	returnvalue=true;         //继续分析
				  //	break;
				  //default: // case uncertain
				  //	//产生新的信息流
				  //	returnvalue=true;         //继续分析
				  //}
#ifdef PA_TRACE
				  paTrace<<"路径可达判定结果："<<returnvalue<<endl;
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
	  //2010-08-11,ZLY,发现自己不可达，直接返回false，不进行指针分析
	  //if (!ast)
	  if (!ast || returnvalue == false)
			//2010-08-11,ZLY,END
	  {
#ifdef PA_TRACE
			paTrace<<"ast is NULL!"<<endl;
			paTrace<<"Old pointto is:"<<endl<<"\t"<<fddset<<pawalker.mypa.pointto<<endl;
#endif

			//2010-08-12,ZLY,BEGIN,如当前结点不可达，则将其出信息置为空
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
	  //  	cout<<"分析语句："<<ast->toStringTree()<<endl;
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
			//cout<<"进pawalker分析:"<<endl;
			// 		xulei_dataflow <<"进pawalker分析:"<<endl;
			//time_t pa_begin, pa_end;
			//pa_begin=clock();	
			pawalker.start(ast,judgeType);
			// 		xulei_dataflow <<"出pawalker分析:"<<endl;
			//pa_end=clock();
			//func<<"指针别名时间："<<pa_end-pa_begin<<endl;
			//cout<<"出pawalker分析:"<<endl;

			//by wangzhiqiang
			//time_t info_begin, info_end;
			//info_begin = clock();
			//infoTreeWalker->setDataFlowAnalyzer(this);
			//infoTreeWalker->walkAstOnVex(v, preVex);
			//info_end = clock();
			//infoRuntime<<"敏感信息分析时间："<<info_end-info_begin<<endl;
			//cout<<"出infoTreewalker分析:"<<endl;
			//end



	  }catch (string s) {
			cout<<"  ERROR!!  "<<s<<endl;
			paprocess<<"  ERROR!!  "<<s<<endl;
#ifdef PA_TRACE
			paTrace<<"Exception occured!!!!!"<<endl;
#endif
	  }
	  //paTrace<<"After PA:start 1"<<endl;
	  //分析完需要对信息进行记录
	  // 	cout<<"hhh1"<<v<<" "<<v->getGlobalIndex()<<endl;
	  //     xulei_dataflow<<"hhh1"<<v<<" "<<v->getGlobalIndex()<<endl;
	  //funcStack出错，
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
/* 类CFGPaths的定义                                                     */
/************************************************************************/
//deque<string> CFGPaths::m_pathFileNames;

CFGPaths::CFGPaths(SymbolItem* f)
{
	  m_curPathCount = 0;
	  if(!f)return;
	  m_func = f;
	  CFG* cfg = (CFG*)(m_func->getCFG());		
	  walkCFG(cfg);								// 遍历搜索路径
	  //sortPaths(); 不使用排序，因为原始生成的路径是基本有序的，排序后有可能影响分支结点的遍历顺序
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

	if (pNode->is_specialNode_need_visited==true)  //无论是什么函数，头结点必须访问
	{
		return true;
	}
	if(pNode->is_entry_functionNode==false)
	{
		//如果改变指针，对于非入口函数来说，一定需要分析
		if( pNode->b_change_pointer == true)
			return true;

		bool bCheck = true;
		//判断污染传播
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

			//如果污染传播说分析，就一定要分析
			if( bCheck == true)
				return true;
		}

		bCheck = true;
		//判断敏感信息
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

			//如果敏感信息说分析，就一定要分析
			if( bCheck == true)
				return true;
		}

		//如果内存漏洞说分析，就一定要分析
		if( memory_function_check_on && pNode->b_memory_sink )
			return true;

		//如果危险函数说分析，就一定要分析
		if( danger_function_check_on && pNode->b_danger_sink )
			return true;
		//大家都说不用分析，终于可以不分析了
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
		//如果内存漏洞说分析，就一定要分析
		if( memory_function_check_on && pNode->b_memory_sink )
			return true;

		//如果危险函数说分析，就一定要分析
		if( danger_function_check_on && pNode->b_danger_sink )
			return true;
		//大家都说不用分析，终于可以不分析了
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

//ZLY, 2010-9-1, BEGIN, 统计路径要覆盖所有结点需要覆盖的节点数
//在死环中的结点不计入计数，分支结点计入2，其他结点计入1
int CFGPaths::CalculateCoverCount(CFG *cfg)
{
	  if(cfg == NULL)
			return 0;
	  VexNode* v = cfg->getHead();	// 表示当前要访问的CFG节点, 初始为头节点	
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
						if( !(nextV->getOutNum()==1 && nextV->getBranchCircle()==onNull)		// 1. 如果顺着当前节点再往下遍历将产生死循环...
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
			}else if(curVex->getOutNum() == 2){//分支结点
				  nextV = getTailVexNode(curVex, true);
				  if(mapVexVisited.find(nextV) == mapVexVisited.end()){
						if( !(nextV->getOutNum()==1 && nextV->getBranchCircle()==onNull)		// 1. 如果顺着当前节点再往下遍历将产生死循环...
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
						if( !(nextV->getOutNum()==1 && nextV->getBranchCircle()==onNull)		// 1. 如果顺着当前节点再往下遍历将产生死循环...
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
//ZLY, 2010-9-1, END, 统计路径要覆盖所有结点需要覆盖的节点数

void CFGPaths::walkCFG(CFG* cfg)
{
	  if(!cfg)						// 如果CFG不存在, 就返回	
			return;
	  //dataflow<<__FILE__<<":"<<__LINE__<<":walkCFG step 1"<<endl;
	  markDeadCircle(cfg);			// 标记CFG中是否存在死环
	  BranchVexStack branch;			// 用来记录分支节点的栈, 辅助用
	  Path path;						// 当前正在搜索的一条路径
	  VexNode* v = cfg->getHead();	// 表示当前要访问的CFG节点, 初始为头节点	

	  //ZLY, 2010-9-1, BEGIN, 记录路径覆盖的节点数
	  int iTotalVex = CalculateCoverCount(cfg);
	  int iCoverVexCount = 0;
	  map<VexNode*, int> mapVexCovered;
	  //ZLY, 2010-9-1, END, 记录路径覆盖的节点数

	  //dataflow<<__FILE__<<":"<<__LINE__<<":walkCFG step 2"<<endl;

	  while((v && v->getLocalIndex()!=1) || !branch.empty() )			// 当前节点不是尾节点, 或者栈非空, 循环
	  {		
			//dataflow<<"Cur vex is"<<v->getLocalIndex()<<", getOutNum is "<<v->getOutNum() <<", getBranchCircle is "<<v->getBranchCircle()<<endl;
			//if(v->getData()->getAST() != nullAST)
			//	dataflow<<v->getData()->getAST()->toRealStringTree()<<endl;
			//dataflow<<__FILE__<<":"<<__LINE__<<":walkCFG step 3"<<endl;
			if( (v->getOutNum()==1 && v->getBranchCircle()==onNull)		// 1. 如果顺着当前节点再往下遍历将产生死循环...
				  || (v->getOutNum()==2 && v->getBranchCircle()==onBoth) )
			{

				  dataflow<<"DEAD CIRCLE FOUND"<<endl;
				  VexNode* breaker = branch.top();	// 进入该死循环的分支节点
				  branch.pop();
				  if(!breaker)						// 如果分支栈已经空, 表明死循环无法退出
				  {
						dataflow<<"DEAD CIRCLE"<<endl;
						return;
				  }
				  dataflow<<"Backtrace from dead circle, pop "
						<<breaker->getLocalIndex()<<" -> ";
				  path.backTrace(breaker);					// 路径回溯...
				  bool prePath = breaker->getBranchPath();	// 进入死循环时, 分支节点所走的分支
				  if(prePath)
				  {
						breaker->setBranchCircle(onTrue);		// 标记breaker的T分支将进入死循环
						breaker->setBranchPath(false);
						v = getTailVexNode(breaker, false);		// 从另一分支搜索
				  }
				  else
				  {
						breaker->setBranchCircle(onFalse);		// 标记breaker的F分支将进入死循环
						breaker->setBranchPath(true);
						v = getTailVexNode(breaker, true);		// 从另一分支搜索
				  } 
				  path.markCoverVex(breaker, iCoverVexCount, mapVexCovered);
				  continue;
			}
			//dataflow<<__FILE__<<":"<<__LINE__<<":walkCFG step 4"<<endl;
			if(v->getOutNum()==2 && v->getBranchCircle()==onTrue)
			{								// 分支节点的T边将进入分支, 设置其branchPath, 搜索F边 
				  path.pushVex(v, iCoverVexCount, mapVexCovered);
				  v->setBranchPath(false);
				  v = getTailVexNode(v, false);
				  continue;
			}
			//dataflow<<__FILE__<<":"<<__LINE__<<":walkCFG step 5"<<endl;
			if(v->getOutNum()==2 && v->getBranchCircle()==onFalse)
			{								// 分支节点的F边将进入分支, 设置其branchPath, 搜索T边
				  path.pushVex(v, iCoverVexCount, mapVexCovered);
				  v->setBranchPath(true);
				  v = getTailVexNode(v, true);
				  continue;
			}

			//dataflow<<__FILE__<<":"<<__LINE__<<":walkCFG step 6"<<endl;
			if(v->getOutNum() ==2 && branch.find(v) )	// 2. 如果节点v是分支节点且在栈中, 遍历其另一条边
			{
				  //dataflow<<"Walk to it's other branch..."<<endl;
				  path.pushVex(v, iCoverVexCount, mapVexCovered);  
				  bool prePath = v->getBranchPath();
				  //2010-08-28,ZLY,BEGIN, 走另外一个分支，要设置BranchPath
				  v->setBranchPath(!prePath);
				  //2010-08-28,ZLY,END
				  v = getTailVexNode(v, !prePath);
				  continue;
			}
			//dataflow<<__FILE__<<":"<<__LINE__<<":walkCFG step 7"<<endl;

			if( v->getOutNum() == 2 )		// 3. 节点v是分支节点, 且不在分支栈中
			{ 
				  if(!path.findVex(v))	// 节点不在路径队列中, 将其压栈. 
						branch.push(v);		// 如果节点在队列中, 表明该节点之前被弹栈, 此时又通过循环回到该节点, 
				  // 因此不能压栈, 否则算法将进入死循环
				  path.pushVex(v, iCoverVexCount, mapVexCovered);

				  bool prePath = v->getBranchPath();
				  v->setBranchPath(!prePath);
				  v = getTailVexNode(v, !prePath);

				  continue;
			}

			//dataflow<<__FILE__<<":"<<__LINE__<<":walkCFG step 8"<<endl;
			if( v->getLocalIndex() != 1)		// 5. v不是分支节点, 且不是尾节点, 访问其下一节点...
			{
				  path.pushVex(v, iCoverVexCount, mapVexCovered);
				  v = v->getFirstOutArc()->getTailVex();
				  continue;
			}

			//dataflow<<__FILE__<<":"<<__LINE__<<":walkCFG step 9"<<endl;
			if(!branch.empty())					// 6. v是尾节点, 但栈不空, 弹栈, 访问其另一条边
			{
				  path.pushVex(v, iCoverVexCount, mapVexCovered);
				  m_paths.push_back(path);

				  //ZLY, 2010-9-1, BEGIN,对大量路径数进行优化
				  switch(giOptimalPaths){
				  case PATH_OPT_ONLY_COVER:
						if(iCoverVexCount >= iTotalVex){
							  cout.rdbuf(pOld);
							  cout<<"    开启仅使用结点覆盖的路径优化, 仅分析"<<m_paths.size()<<"条路径!"<<endl;
							  pOld = cout.rdbuf(DzhTest.rdbuf());	 
							  paTrace<<"    开启仅使用结点覆盖的路径优化, 仅分析"<<m_paths.size()<<"条路径!"<<endl;
							  return;
						}
						break;
				  case PATH_OPT_COVER_AND_COUNT:
						if(m_paths.size() >= gi_PATH_COUNT_OPT_THRESHOLD){
							  //paTrace<<"iCoverVexCount = "<<iCoverVexCount<<", iTotalVex = "<<iTotalVex<<endl;
							  if(iCoverVexCount >= iTotalVex){
									cout.rdbuf(pOld);
									cout<<"    开启带路径数阈值的结点覆盖路径优化, 仅分析"<<m_paths.size()<<"条路径!"<<endl;
									pOld = cout.rdbuf(DzhTest.rdbuf());	 
									paTrace<<"    开启带路径数阈值的结点覆盖路径优化, 仅分析"<<m_paths.size()<<"条路径!"<<endl;
									return;
							  }
						}
						break;
				  case PATH_OPT_ONLY_COUNT:
						if(m_paths.size() >= gi_PATH_COUNT_OPT_THRESHOLD){
							  cout.rdbuf(pOld);
							  cout<<"    开启仅使用路径数阈值的路径优化, 仅分析"<<m_paths.size()<<"条路径!"<<endl;
							  pOld = cout.rdbuf(DzhTest.rdbuf());	 
							  paTrace<<"    开启仅使用路径数阈值的路径优化, 仅分析"<<m_paths.size()<<"条路径!"<<endl;
							  return;
						}
						break;
				  case PATH_OPT_NO_OPT:
				  default:
						if(m_paths.size() >= MAX_PATH_COUNT){
							  cout.rdbuf(pOld);
							  cout<<"    函数中的路径数太多, 仅分析"<<m_paths.size()<<"条路径!"<<endl;
							  pOld = cout.rdbuf(DzhTest.rdbuf());	 
							  paTrace<<"    函数中的路径数太多, 仅分析"<<m_paths.size()<<"条路径!"<<endl;
							  return;
						}
						break;
				  }
				  //ZLY, 2010-9-1, END,对大量路径数进行优化


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

// 获取一条路径
bool CFGPaths::getNextPath(Path& path, int& cnt)
{
	  if(m_curPathCount>=m_paths.size())return false;
	  cnt = m_curPathCount;
	  path = m_paths[m_curPathCount++];
	  return true;
}

// 路径的数量
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

Path* CFGPaths::getPath(int cnt) { // 获得第cnt条路径，从cnt=0开始编号路径 by kong 20100111
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
			//2010-08-30,ZLY,若节点第一次访问，则添加空的set
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
						// 每个节点可以到达其孩子节点可以到达的节点
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
	  // 情况说明：若存在死循环，则下面的循环中，将整个控制流的结点均设置了 死循环标志，
	  // 因此，临时采取的方法是这样的：
	  // 循环中累计设置死循环标志的结点中的最大行号，出循环后使用该行号。
	  // 但是存在一个问题：
	  // 只能报告整个函数体中，代码行最晚的死循环，且报告的是该循环体最后一条语句的行号
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
			/* ZLY 2010-8-19 这是原来的代码
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
				  << "语义错误\t语义错误\t存在死循环"  << endl;
	  }
	  //ZLY 2010-8-19 END: report dead-loop
	  //dataflow<<__FILE__<<":"<<__LINE__<<":markDeadCircle step"<<endl;
}



/************************************************************************/
/* BranchVexStack中的方法定义                                           */
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
// 类CFGPathsManager的方法定义
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
/* 类Path的方法定义                                                     */
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
	  //2010-08-11,ZLY,BGING,初始化分析标记位
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
			//2010-08-11,ZLY,BGING,初始化分析标记位
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

			//o<<"/"<<v->rank;//输出rank参数

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
/* 类CallStack的方法定义                                                */
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

// bdd bone = bdd_fullsatone(b1); // 确保只取一个元素
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
						//cout << "找到实际的ID了: " << realID << endl;
						return realID;
				  }
				  else {
						map<int, int>::iterator iMap2;
						for (iMap2 = TmpToReal.begin(); iMap2 != TmpToReal.end(); iMap2++) {
							  if (iMap2->first == realID) {
									//cout << "找到实际的ID了2222: " << iMap->second << endl;
									return iMap->second;
							  }
						}
				  }
			}
	  }
	  cout << "找不到实际的ID" << endl;
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
			if(0 == v->getLocalIndex()){//找到了当前分析的函数的入口节点，不在继续向前查找
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
	  //当前要分析的是栈顶元素
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
	  //	futeng_debug<<"、"<<*it;
	  //}
	  //futeng_debug<<endl;
	  //futeng_debug<<"stk:"<<endl;
	  //for(vector<VexNode*>::iterator it=nodeStack.begin();it!=nodeStack.end();it++)
	  //{
	  //	futeng_debug<<"、"<<(*it)->getLocalIndex();
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





