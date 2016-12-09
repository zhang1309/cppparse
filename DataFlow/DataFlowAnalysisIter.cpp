#include "DataFlowAnalysisIter.hpp"
#include "../ExprTreeWalkerIter.hpp"
#include "../CFG/CFGStructure.h"
#include <iostream>
using namespace std;

ofstream dataflowIter("dataflowIter.txt");
list<Definition*> allDef;					// 保存所有的定值
map<VexNode*, list<Definition*> > gen;		// 每个控制注流节点上的gen集合
map<VexNode*, list<Definition*> > kill;		// 每个控制注流节点上的kill集合
//map<SymbolItem*, list<Definition*> > sblDefSet;
map<QualifiedSymbolItem, list<Definition*> > sblDefSet;	// 为符号条目保存其所有定值
extern list<SymbolItem*> g_FunctionList;

void printDefInfor(CFG* cfg);
void calculateDefinitions(CFG* cfg);	// 迭代计算定值集合
void printSet(InOutSet* s);
void printGenOrKill(list<Definition*>& lst);
void printDef(Definition* def);

void walkTree()
{
	list<SymbolItem*>::iterator iter;
	ExprTreeWalkerIter walker;
	for(iter = g_FunctionList.begin(); iter != g_FunctionList.end(); iter++) // 访问每个函数条目
	{
		CFG *p = (CFG*)((*iter)->getCFG());		// 获取函数条目的CFG
		if(!p) continue;
		
		gen.clear(); sblDefSet.clear(); kill.clear();
		VexNode* vex = p->getHead();			// 获取控制流的头节点

		// 1. 先遍历控制流图中每个节点对应的AST一次, 
		// 计算各个控制流图节点的gen, kill集合, 以及符号条目的所有定值sblDefSet
		while(vex)			// 
		{
			CFGNode* node = vex->data;
			if(!node){ vex = vex->nextVexNode; continue; }
			RefAST ast =(RefAST)node->getAST();	// 获取控制流节点上的AST
			if(!ast){vex = vex->nextVexNode; continue;}
			walker.init(vex);
			walker.start(ast);	// 遍历AST, 更新gen, kill, sblDefSet, allDef
			vex = vex->nextVexNode;
		}

		// 2. 迭代计算控制流图p上的定值
		calculateDefinitions(p);
		printDefInfor(p);	// 打印相关信息
	}
}

void calculateDefinitions(CFG* cfg)
{
	if(!cfg)return;
	VexNode* vex = cfg->getHead();
	while(vex)	//初始化
	{
		vex->in=new InOutSet();
		vex->out=new InOutSet();  
		*(vex->out) = gen[vex];		// 初始化in, out
	
		// 计算kill集
		list<Definition*>& kills = kill[vex];
		list<Definition*>::iterator it;
		for(it=gen[vex].begin(); it!=gen[vex].end(); it++)
		{
			Definition* def = *it;		//节点vex上的定义
			kills += sblDefSet[def->defSbl];		//def定义的变量的所有定义
			kills.remove(def);		// 被def定义注销的定义集
		}
		vex=vex->nextVexNode;
	}

	bool change = true;    

	while(change)  // 计算in out集合.
	{
		change = false;
		vex = cfg->getHead();
		while(vex)
		{
			InOutSet* in = vex->in;
			InOutSet* out = vex->out;
			ArcBox* arc = vex->firstin;
			while(arc)   // in = out + out + ...
			{
				*in+=*(arc->getHeadVex()->out);
				arc = arc->getNextEdgeWithSameTailVex();
			}

			InOutSet oldOut;
			oldOut = *out;
			*out = gen[vex]+(*in-kill[vex]);   // out = gen + in - kill
			if(oldOut!=*out)change=true;
			vex=vex->nextVexNode;
		}
	}
}

void printDefInfor(CFG* cfg)
{
	if(!cfg)return;
	
	//显示所有的定义
	dataflowIter<<"///////// All definitions: "<<endl;
	for(map<QualifiedSymbolItem, list<Definition*> >::iterator it = sblDefSet.begin(); 
		it!=sblDefSet.end(); it++)
	{
		list<Definition*>& lst = it->second;
		printGenOrKill(lst);
	}

	VexNode* vex = 0;
	vex = cfg->getHead();	
	while(vex)
	{
		if(vex->getLocalIndex()==0){vex=vex->nextVexNode; continue;}
		//显示gen集合
		dataflowIter<<"VexNode "<<vex->getLocalIndex()<<" Gen set is: "<<endl;
		printGenOrKill(gen[vex]);

		//显示kill集合
		dataflowIter<<"VexNode "<<vex->getLocalIndex()<<" Kill set is: "<<endl;
		printGenOrKill(kill[vex]);

		//显示in集合
		dataflowIter<<"VexNode "<<vex->getLocalIndex()<<" In Set is: "<<endl;
		printSet(vex->in);
		
		//显示out集合
		dataflowIter<<"VexNode "<<vex->getLocalIndex()<<" Out Set is: "<<endl;
		printSet(vex->out);

		vex=vex->nextVexNode;
	}
}

void printSet(InOutSet* s)
{
	map<QualifiedSymbolItem, list<Definition*> >::iterator it;
	int i = 0; 
	for(it=s->defMap.begin(); it!=s->defMap.end(); it++)
	{
		list<Definition*>& defs = it->second;
		list<Definition*>::iterator itt;
		for(itt=defs.begin(); itt!=defs.end(); itt++)
		{
			Definition* def = *itt; i++;
			printDef(def); dataflowIter<<'\t';
		}
	}
	if(!i)dataflowIter<<"NULL";
	dataflowIter<<endl;
}

void printGenOrKill(list<Definition*>& lst)
{
	if(lst.empty())
	{
		dataflowIter<<"NULL"<<endl;
		return;
	}
	list<Definition*>::iterator it;
	for(it=lst.begin(); it!=lst.end(); it++)
	{
		Definition* def = *it;
		printDef(def);
		dataflowIter<<'\t';
	}
	dataflowIter<<endl;
}

void clearDefinition()
{
	list<Definition*>::iterator it;
	int i = allDef.size();
	for(it=allDef.begin(); it!=allDef.end(); it++)
	{
		Definition* def = *it;
		if(def) delete def;
	}
	dataflowIter<<i<<" definitions have been deleted"<<endl;
}


void printDef(Definition* def)
{
	QualifiedSymbolItem sbls = def->defSbl;
	dataflowIter<<'(';
	Iter it = sbls.begin();
	while(it!=sbls.end())
	{
		SymbolItem* sbl = *it;
		dataflowIter<<sbl->getName();
		it++;
		if(it!=sbls.end())dataflowIter<<"::";
	}
	dataflowIter<<" , ";
	dataflowIter<<def->defVex->getLocalIndex()<<')';
}