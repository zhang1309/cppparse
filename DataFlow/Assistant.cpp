#include "Assistant.h"
#include "../GlobalVariableExprWalker.hpp"
#include "../ExprValueTreeWalker.hpp"
#include "../PA/PAanalysis.h"
#include "../STDCTokenTypes.hpp"
#include <list>
#include <algorithm>

extern ProcessBar processbar_Cur;

int octAtoi(const char* str)
{
	int r = 0;
	int i = 1;
	while(str[i]!='\0')
	{
		if(str[i]<'0' || str[i]>'7')
			return r;
		r = 8*r + (str[i++]-'0');
	}
	return r;
}

int hexAtoi(const char* str)
{
	int r = 0;
	int i = 2;
	while(str[i]!='\0')
	{
		if(str[i]>='a' && str[i]<='f')
			r = r*16+ (10 + str[i++]-'a'); 
		else if(str[i]>='A' && str[i]<='F')
			r= r*16+(10 + str[i++]-'A'); 
		else if(str[i]<='9' && str[i]>='0')
			r = 16*r+(str[i++] - '0');
		else return r;
	}
	return r;
}

bool isChildScope(ScopeSymbolTable*c, ScopeSymbolTable* p)
{
	if(!p || !c)return false;
	ScopeSymbolTable* tmp = c->getParentScopeEntry();
	while(tmp)
	{
		if(tmp == p)
			return true;
		tmp = tmp->getParentScopeEntry();
	}
	return false;
}

bool isBrotherScope(ScopeSymbolTable*b1, ScopeSymbolTable* b2)
{
	if(!b1 || !b2)return false;
	if(b1->getBrotherScopeEntry()==b2 || b2 ->getBrotherScopeEntry()==b1)
		return true;
	return false;
}

VexNode* getTailVexNode(VexNode* v, bool b)
{
	ArcBox* arc= v->getFirstOutArc();
	while(arc)
	{
		if(b && arc->edgeTypeToString()=='T')
			return arc->getTailVex();
		if(!b && arc->edgeTypeToString()=='F')
			return arc->getTailVex();
		arc = arc->getNextEdgeWithSameHeadVex();
	}
	return 0;
}

void calculateGlobalVariableValues(VexValueSet* enValues)
{
	extern SymRoot* root;

	/* 2016-08-18,ZLY,从root中获取全局变量，因为全局变量都存在文件作用域了，没有放全局作用域，参考genGlobelPointto函数，该函数计算所有全局指针指向
	ScopeSymbolTable* glbTbl = root->getGlobalScope();		// 获得全局作用域
	list<SymbolItem*>& glbSbls = glbTbl->getVariableList();	// 获得全局变量列表
	list<SymbolItem*>::iterator git;
	*/
	const list<SymbolItem* >& glbSbls = root->getSIInFileScope();
	list<SymbolItem*>::const_iterator git;
	if(glbSbls.size() > 0)
	{
		openConsole();
		cout<<"计算全局变量初始值，共"<<glbSbls.size()<<"个全局变量..."<<endl;
		closeConsole();
		processbar_Cur.setCurrentAnalysisInfo(string("计算全局变量初始值"), glbSbls.size());
	}
	int iCounter = 0;
	for(git=glbSbls.begin(); git!=glbSbls.end(); git++)		// 遍历全局变量
	{
		
		SymbolItem* sbl = *git;
		RefAST ast = (sbl->getASTNode());					
		if(!ast)					// 如果全局变量没有对应的AST, 类对象声明暂时找不到其声明时的AST
		{		
			if(MingledSymbolItem(sbl).isIntegerType())//判断是否是整形
			{
				IntegerValueSet vs = IntegerValueSet(IntegerValue());
				vs.setLine(sbl->getFileLineNum());
				(*enValues)[MingledSymbolItem(sbl)] = vs;
			}
			continue;
		}
		if(MingledSymbolItem(sbl).isIntegerType())
		{
			GlobalVariableExprWalker walker;				// 全局变量初始化ast遍历器
			walker.init(enValues);							// 初始化遍历器
			walker.declarationTree((RefMyAST)ast);	
		}
		processbar_Cur.setCurrentAnalysisPos(++iCounter);
	}
}

bool isMemberOfType(SymbolItem* sbl, Type* type)
{
	if(!type || !sbl) return false;
	ItemKind kind = type->getItemKind();
	if(kind!=TYPE_CLASS && kind!=TYPE_STRUCT)
		return false;
	list<SymbolItem*> members = type->getAttributelist();
	list<SymbolItem*>::iterator it = find(members.begin(), members.end(), sbl);
	if(it!=members.end()) return true;
	list<Type*> baseTypes = type->getBaseClassList();
	list<Type*>::iterator tit;
	for(tit=baseTypes.begin(); tit!=baseTypes.end(); tit++)
	{
		Type* bType = *tit;
		if(isMemberOfType(sbl,bType)) return true;
	}
	return false;
}

IntegerValueSet getIntValuesFromAst(RefMyAST ast, VexNode* v, VexNode* preVex, PATreeWalker& pawalker)
{
	ItemKind item;
	if(!ast || !v) return IntegerValueSet();
	DataFlowAnalyzer analyzer(0,pawalker,pawalker.myAnalyzer->p_parser);
	ExprValueTreeWalker* walker = analyzer.getExprTreeWalker();
	walker->setVexes(v,preVex);
	walker->setFuncInfoCollect(false);
	if(STDCTokenTypes::Expression == ast->getType())	
	{
		return walker->expressionTree(ast,MingledSymbolItem(),item);
	}
	else
	{
		return walker->assignmentExpressionTree(ast,MingledSymbolItem(),item);
	}
}



