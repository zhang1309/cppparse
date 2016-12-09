#include "DataFlowAnalysisIter.hpp"
#include "../ExprTreeWalkerIter.hpp"
#include "../CFG/CFGStructure.h"
#include <iostream>
using namespace std;

ofstream dataflowIter("dataflowIter.txt");
list<Definition*> allDef;					// �������еĶ�ֵ
map<VexNode*, list<Definition*> > gen;		// ÿ������ע���ڵ��ϵ�gen����
map<VexNode*, list<Definition*> > kill;		// ÿ������ע���ڵ��ϵ�kill����
//map<SymbolItem*, list<Definition*> > sblDefSet;
map<QualifiedSymbolItem, list<Definition*> > sblDefSet;	// Ϊ������Ŀ���������ж�ֵ
extern list<SymbolItem*> g_FunctionList;

void printDefInfor(CFG* cfg);
void calculateDefinitions(CFG* cfg);	// �������㶨ֵ����
void printSet(InOutSet* s);
void printGenOrKill(list<Definition*>& lst);
void printDef(Definition* def);

void walkTree()
{
	list<SymbolItem*>::iterator iter;
	ExprTreeWalkerIter walker;
	for(iter = g_FunctionList.begin(); iter != g_FunctionList.end(); iter++) // ����ÿ��������Ŀ
	{
		CFG *p = (CFG*)((*iter)->getCFG());		// ��ȡ������Ŀ��CFG
		if(!p) continue;
		
		gen.clear(); sblDefSet.clear(); kill.clear();
		VexNode* vex = p->getHead();			// ��ȡ��������ͷ�ڵ�

		// 1. �ȱ���������ͼ��ÿ���ڵ��Ӧ��ASTһ��, 
		// �������������ͼ�ڵ��gen, kill����, �Լ�������Ŀ�����ж�ֵsblDefSet
		while(vex)			// 
		{
			CFGNode* node = vex->data;
			if(!node){ vex = vex->nextVexNode; continue; }
			RefAST ast =(RefAST)node->getAST();	// ��ȡ�������ڵ��ϵ�AST
			if(!ast){vex = vex->nextVexNode; continue;}
			walker.init(vex);
			walker.start(ast);	// ����AST, ����gen, kill, sblDefSet, allDef
			vex = vex->nextVexNode;
		}

		// 2. �������������ͼp�ϵĶ�ֵ
		calculateDefinitions(p);
		printDefInfor(p);	// ��ӡ�����Ϣ
	}
}

void calculateDefinitions(CFG* cfg)
{
	if(!cfg)return;
	VexNode* vex = cfg->getHead();
	while(vex)	//��ʼ��
	{
		vex->in=new InOutSet();
		vex->out=new InOutSet();  
		*(vex->out) = gen[vex];		// ��ʼ��in, out
	
		// ����kill��
		list<Definition*>& kills = kill[vex];
		list<Definition*>::iterator it;
		for(it=gen[vex].begin(); it!=gen[vex].end(); it++)
		{
			Definition* def = *it;		//�ڵ�vex�ϵĶ���
			kills += sblDefSet[def->defSbl];		//def����ı��������ж���
			kills.remove(def);		// ��def����ע���Ķ��弯
		}
		vex=vex->nextVexNode;
	}

	bool change = true;    

	while(change)  // ����in out����.
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
	
	//��ʾ���еĶ���
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
		//��ʾgen����
		dataflowIter<<"VexNode "<<vex->getLocalIndex()<<" Gen set is: "<<endl;
		printGenOrKill(gen[vex]);

		//��ʾkill����
		dataflowIter<<"VexNode "<<vex->getLocalIndex()<<" Kill set is: "<<endl;
		printGenOrKill(kill[vex]);

		//��ʾin����
		dataflowIter<<"VexNode "<<vex->getLocalIndex()<<" In Set is: "<<endl;
		printSet(vex->in);
		
		//��ʾout����
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