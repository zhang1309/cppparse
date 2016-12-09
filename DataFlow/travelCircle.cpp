#include "travelCircle.h"
#include "../GolbalSupport.h"
#include "../SymTable/SymbolTable.h"
#include "../SymTable/ScopeSymbolTable.h"
#include "../buddy/bdd.h"
#include<unordered_map>
//#define FUTENG_DEBUG
extern map<VexNode*,map<int,bool>* > gLoopVexPatterns;
extern ofstream futeng_path;
extern ofstream futeng_debug;
void closeConsole();
void openConsole();

//#define ANALYZE_OPT_TRACE

/*路径选择相关*/
/*******futeng modify
@作用：判断是否在遍历一次循环内的结点
@参数：
	funcSymbolItem:该循环体所在的函数的符号表条目
	curVex:当前被分析的循环体的首节点
	isFirstTraveled:是不是第一次进入循环
@返回值:如果需要重新分析返回true，否则返回false
*/
bool travelCircleAgain(SymbolItem *funcSymbolItem,PATreeWalker& pawalker,VexNode* curVex,bool isFirstTraveled)
{
	bool returnValue=false;
	//如果curVex不是分支节点，返回false
	if(curVex->getOutNum()<2)
	{
		return returnValue;
	}
	//得到在该函数体内定义的所有的变量
	int funcID=funcSymbolItem->getSymbolID();
	bdd variableBdd=pawalker.mypa.variableInFunc&fdd_ithvar(M1,funcID);
	if(bddfalse==variableBdd)//还没有把该函数的变量存到bdd中
	{
		ScopeSymbolTable* scopeSymbol=static_cast<ScopeSymbolTable*>(funcSymbolItem->getPointtoSymTable());
		scopeSymbol=scopeSymbol->getFirstSonScopeEntry();
		const list<SymbolItem*> &variableList=scopeSymbol->getVariableList();
		for(list<SymbolItem*>::const_iterator it=variableList.begin();it!=variableList.end();it++)
		{
			pawalker.mypa.variableInFunc|=fdd_ithvar(V1,(*it)->getSymbolID())&fdd_ithvar(F1,0)&fdd_ithvar(M1,funcID);
		}
	}
	variableBdd=pawalker.mypa.variableInFunc&fdd_ithvar(M1,funcID);
	variableBdd=bdd_exist(variableBdd,fdd_ithset(M1));
	vector<SymbolItem*> funcTable;
	funcTable.push_back(funcSymbolItem);
	list<bdd> thisTable;
	vector<bdd> tempvbdd;
	vector<FunParaTemp*> tempvFunPara;
	genPASCall(pawalker,variableBdd,tempvbdd,tempvFunPara);
	bdd inCircleBdd=bddfalse;
	return returnValue;
}
void genPASCall(PATreeWalker& pawalker,bdd variableInFunc,vector<bdd>& tempvbdd,vector<FunParaTemp*>& tempvFuncPara)
{
	//首先处理全局变量
	bdd globalbdd=pawalker.mypa.v_globel;
	globalbdd|=variableInFunc;
	int globalID=fdd_scanvar(globalbdd,V1);
	bdd globalOne;
	bdd itemOne;
	while(globalID>0)
	{
		globalOne=fdd_ithvar(V1,globalID);
		unordered_map<int,SymbolItem*>::iterator variableMapIt=VariableMap.find(globalID);
		if(variableMapIt==VariableMap.end())
		{
			futeng_path<<"genPASCall error:发现一个不存在的全局变量！"<<endl;
			globalbdd-=globalOne;
			globalID=fdd_scanvar(globalbdd,V1);
			continue;
		}
		SymbolItem* globalSymbol=variableMapIt->second;
#ifdef FUTENG_DEBUG
		futeng_debug<<"global: "<<globalSymbol->getName()<<";ID: "<<globalSymbol->getSymbolID()<<endl;
#endif
		
		//扩充变量，主要是给指针变量的每一级取一个别名
		pawalker.genFunParaSpace(globalSymbol->itemParaTemp);
		pawalker.genFunParaTemp(globalSymbol->getTypeItem(),0,globalSymbol->itemParaTemp,globalSymbol->getSymbolID());
		FunParaTemp* ftemp=globalSymbol->itemParaTemp;
#ifdef FUTENG_DEBUG
		FunParaTemp* ftemp1=ftemp;
		while(ftemp1->num>=0)
		{
			futeng_debug<<"\tsourceItem:"<<ftemp1->sourceItem<<endl;
			futeng_debug<<"\tnum:"<<ftemp1->num<<endl;
			futeng_debug<<"\titem:"<<ftemp1->item<<endl<<endl;
			ftemp1++;
		}
#endif
		bdd bddItem=globalOne&fdd_ithvar(F1,0);//(V1)=>(V1,F1)增加了一个F1域
		while(ftemp->num>=0)
		{
			int lv=fdd_scanvar(bddItem,V1);
			bdd bddtemp1=fdd_ithvar(V1,lv)&pawalker.mypa.v_struct;
			bdd bddtemp2=fdd_ithvar(V1,lv)&pawalker.mypa.v_point;
			if (bddtemp1!=bddfalse&&bddtemp2==bddfalse)
			{
				/*关于全局变量时结构体的情况先忽略，代码有些繁琐
				if(ftemp->num==0)
					genIntraRepreStructBdd(fdd_ithvar(V1,ftemp->item)&fdd_ithvar(F1,0),callbdd,calleebdd,bdditem,bddCall2Return,false);
				else
					genIntraRepreStructBdd(bdditem,callbdd,calleebdd,bdditem,bddCall2Return,false);*/
			}
			pawalker.add2TempFunTable(tempvbdd,tempvFuncPara,bddItem,ftemp);//将变量对应的bdd加入到tempvbdd中，指向关系加入到tempvFuncPara中
			ftemp++;
			bdd bddItem1=pawalker.mypa.pointto&bddItem;
			if(bddfalse==bddItem1)//指针被没有指向任何东西
			{
				break;
			}
			pawalker.mypa.funcin|=bddItem1;
			bddItem1=bdd_exist(bddItem1,fdd_ithset(V1)&fdd_ithset(F1));
			bddItem=my_bdd_replace(bddItem1,pawalker.mypa.right2left);
		}

		
		globalbdd-=globalOne;
		globalID=fdd_scanvar(globalbdd,V1);
	}//end while(globalID>0)
#ifdef FUTENG_DEBUG
	vector<bdd>::iterator bddIt=tempvbdd.begin();
	int i=0;
	while(bddIt!=tempvbdd.end())
	{
		futeng_debug<<"\ttempvbdd["<<i<<"]:"<<fddset<<(*bddIt)<<endl;
		FunParaTemp* funp=tempvFuncPara[i];
		while(funp->num>=0)
		{
			futeng_debug<<"\t\tsourceItem:"<<funp->sourceItem<<endl;
			futeng_debug<<"\t\tnum:"<<funp->num<<endl;
			futeng_debug<<"\t\titem:"<<funp->item<<endl<<endl;
			funp++;
		}
		i++;
		bddIt++;
	}
#endif
}
void genPASCallFollow(vbdd& tempvbdd,vector<FunParaTemp*>& tempvFunPara,
	vbdd& reprebdd,vector<int>& repreElem,vbdd& tempvbdduse,PATreeWalker& pawalker)
{
	FunParaTemp* tempPara;
	list<FunParaTemp> reorderPara;
	vector<FunParaTemp*>::iterator paraIt=tempvFunPara.begin();
	if(paraIt!=tempvFunPara.end())
	{
		tempPara=*paraIt;
		reorderPara.clear();
		pawalker.reorderFuncPara(reorderPara,tempPara);
	}
}
/*
bdd pointerPatternMatching(PATreeWalker &pawalker,bdd &heapOut,VexNode *curNode,pathToTravel path)
{
	int globalIndex=curNode->getGlobalIndex();
	bdd patternBdd=bddfalse;
	//得到节点已经记录的模式
	patternBdd=pawalker.mypa.circleInPointerResult&fdd_ithvar(C,globalIndex);
	if(bddfalse==patternBdd)//模式为空
	{
		return bddfalse;
	}
	//遍历每一个模式，查看是否匹配当前的pawalker.mypa.pointto
	int calledNum=curNode->getCalledNum();
	for(int i=1;i<=calledNum;i++)
	{
		bdd onePattern=patternBdd&fdd_ithvar(K,i);//取出每一次被调用时的模式
		onePattern=bdd_exist(onePattern,fdd_ithset(K)&fdd_ithset(C));
		if(bddfalse==onePattern-pawalker.mypa.pointto)//如果当前模式是以记录模式的子集则模式匹配
		{
			bdd bddMatched=bddfalse;
			if(TruePath==path)
			{
				bddMatched=pawalker.mypa.circleOutPointerResult&fdd_ithvar(C,globalIndex)&fdd_ithvar(K,i)&fdd_ithvar(D,1);
				heapOut=pawalker.mypa.circleOutHeapResult&fdd_ithvar(C,globalIndex)&fdd_ithvar(K,i)&fdd_ithvar(D,1);
			}else if(FalsePath==path)
			{
				bddMatched=pawalker.mypa.circleOutPointerResult&fdd_ithvar(C,globalIndex)&fdd_ithvar(K,i)&fdd_ithvar(D,0);
				heapOut=pawalker.mypa.circleOutHeapResult&fdd_ithvar(C,globalIndex)&fdd_ithvar(K,i)&fdd_ithvar(D,0);
			}else{

			}
			if(bddfalse==bddMatched)//结果为空，可能是当前对应分支还没有遍历过
			{
				return bddfalse;
			}
			bddMatched=bdd_exist(bddMatched,fdd_ithset(K)&fdd_ithset(D)&fdd_ithset(C));
			return bddMatched; 
		}else{//模式不匹配，将当前模式记录下来
			curNode->addCalledNum();
			bdd patternSaved=pawalker.mypa.pointto&fdd_ithvar(K,curNode->getCalledNum())&fdd_ithvar(C,globalIndex)&fdd_ithvar(D,1);
			pawalker.mypa.circleInPointerResult|=patternSaved;
		}
	}
	return bddfalse;
}*/
/*
作用：遇到循环节点时，判断分析到当前节点的指针模式是否曾经出现过
参数：curNode：指向当前分析的节点
	  path：TurePath表示当前验证的是循环节点的真分支；FalsePath表示当前验证的是循环节点的假分支
备注：在记录模式的时候用D域表示分支，1表示假分支，2表示真分支
*/
bool matchCircleInPattern(PATreeWalker &pawalker,VexNode *curNode)
{
	map<VexNode*,map<int,bool>* >::iterator it=gLoopVexPatterns.find(curNode);
	if(gLoopVexPatterns.end() == it)
	{
		map<int ,bool> *newPattern = new map<int ,bool>;
		newPattern->insert(pair<int, bool>(pawalker.mypa.pointto.id() , true));
		gLoopVexPatterns.insert(pair<VexNode*,map<int,bool>* >(curNode,newPattern));
		return false;
	}

	map<int,bool>* pattern = it->second;
	map<int ,bool>::iterator itP = pattern->find(pawalker.mypa.pointto.id());
	if(pattern->end() == itP)
	{
		pattern->insert(pair<int ,bool>(pawalker.mypa.pointto.id(),true));
		return false;
	}else{
		return true;
	}
#if 0	
	int globalIndex=curNode->getGlobalIndex();
	bdd patternBdd=bddfalse;
	//得到节点已经记录的模式
	patternBdd=pawalker.mypa.circleInPointerResult&fdd_ithvar(C,globalIndex);

	if(bddfalse==patternBdd)//模式为空
	{
		return false;
	}
	//遍历每一个模式，查看是否匹配当前的pawalker.mypa.pointto
	int calledNum=curNode->getCalledNum();
	for(int i=1;i<=calledNum;i++)
	{
		/*之所以先给pointto保留C域，是因为如果此时模式为空，那么有效域只有C和K域，如果此时把C和K域全去掉，结果不是bddfalse，是bddtrue*/
		bdd onePattern=patternBdd&fdd_ithvar(K,i);//取出每一次被调用时的模式
		onePattern=bdd_exist(onePattern,fdd_ithset(K));
		bdd temp;
		/*如果pointto为bddfalse，此时如果用pointto & fdd_ithvar(C,1)的方式给它添加C域，得到的仍然是bddfalse
		  */
		if(bddfalse == pawalker.mypa.pointto)
		{
			temp=fdd_ithvar(C,globalIndex);
		}else{
			temp=pawalker.mypa.pointto & fdd_ithvar(C,globalIndex);
		}
		if(bddfalse==onePattern-temp)//如果当前模式是以记录模式的子集则模式匹配
		{
			ret=true;
			break;
		}
	}
#endif
	return false;
}
/*
作用：遇到循环节点时，决定是否分析循环及以后的路径,如果是一个新的模式，会记录下来
参数：curNode：指向当前分析的节点
	  path：TurePath表示当前验证的是循环节点的真分支；FalsePath表示当前验证的是循环节点的假分支
备注：在记录模式的时候用D域表示分支，1表示假分支，2表示真分支
*/
bool isContinueRun(PATreeWalker& pawalker,VexNode* curNode)
{
	bool ret=true;
	if(vCircle==curNode->getNodeType()|| vSwitch==curNode->getNodeType() || vIf==curNode->getNodeType() || vDoWhile==curNode->getNodeType())//如果是循环，进行模式匹配
	{
		if(vCircle == curNode->getNodeType() || vDoWhile == curNode->getNodeType())//如果是循环，进行模式匹配
		{
			curNode->addCalledNum();
			if(curNode->getCalledNum() > 10)
			{
				//curNode->resetCalledNum();
				return false;
			}
		}
		
		if(vIf == curNode->getNodeType() || vSwitch == curNode->getNodeType())//如果是循环，进行模式匹配
		{
			curNode->addCalledNum();
			if(curNode->getCalledNum() > 10)
			{
				//curNode->resetCalledNum();
				return false;
			}
		}
		bdd bddMatched;
		bdd heapOut;
		if(matchCircleInPattern(pawalker,curNode))//查看是否有匹配的模式，没有的话记录下来
		{
#ifdef ANALYZE_OPT_TRACE
			openConsole();
			cout<<"branch node compatable,quit current path...."<<endl;
			closeConsole();
#endif
			ret=false;   
		}else{
			/*//存储这个新的模式
			curNode->addCalledNum();
			int k=curNode->getCalledNum();
			if(bddfalse==pawalker.mypa.pointto)
			{
				pawalker.mypa.circleInPointerResult |= fdd_ithvar(C,curNode->getGlobalIndex())&fdd_ithvar(K,k);

			}else{
				pawalker.mypa.circleInPointerResult |= pawalker.mypa.pointto&fdd_ithvar(C,curNode->getGlobalIndex())&fdd_ithvar(K,k);
			}*/
			return true;
		}
	}
	return ret;
}



