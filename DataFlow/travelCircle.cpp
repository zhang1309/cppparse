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

/*·��ѡ�����*/
/*******futeng modify
@���ã��ж��Ƿ��ڱ���һ��ѭ���ڵĽ��
@������
	funcSymbolItem:��ѭ�������ڵĺ����ķ��ű���Ŀ
	curVex:��ǰ��������ѭ������׽ڵ�
	isFirstTraveled:�ǲ��ǵ�һ�ν���ѭ��
@����ֵ:�����Ҫ���·�������true�����򷵻�false
*/
bool travelCircleAgain(SymbolItem *funcSymbolItem,PATreeWalker& pawalker,VexNode* curVex,bool isFirstTraveled)
{
	bool returnValue=false;
	//���curVex���Ƿ�֧�ڵ㣬����false
	if(curVex->getOutNum()<2)
	{
		return returnValue;
	}
	//�õ��ڸú������ڶ�������еı���
	int funcID=funcSymbolItem->getSymbolID();
	bdd variableBdd=pawalker.mypa.variableInFunc&fdd_ithvar(M1,funcID);
	if(bddfalse==variableBdd)//��û�аѸú����ı����浽bdd��
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
	//���ȴ���ȫ�ֱ���
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
			futeng_path<<"genPASCall error:����һ�������ڵ�ȫ�ֱ�����"<<endl;
			globalbdd-=globalOne;
			globalID=fdd_scanvar(globalbdd,V1);
			continue;
		}
		SymbolItem* globalSymbol=variableMapIt->second;
#ifdef FUTENG_DEBUG
		futeng_debug<<"global: "<<globalSymbol->getName()<<";ID: "<<globalSymbol->getSymbolID()<<endl;
#endif
		
		//�����������Ҫ�Ǹ�ָ�������ÿһ��ȡһ������
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
		bdd bddItem=globalOne&fdd_ithvar(F1,0);//(V1)=>(V1,F1)������һ��F1��
		while(ftemp->num>=0)
		{
			int lv=fdd_scanvar(bddItem,V1);
			bdd bddtemp1=fdd_ithvar(V1,lv)&pawalker.mypa.v_struct;
			bdd bddtemp2=fdd_ithvar(V1,lv)&pawalker.mypa.v_point;
			if (bddtemp1!=bddfalse&&bddtemp2==bddfalse)
			{
				/*����ȫ�ֱ���ʱ�ṹ�������Ⱥ��ԣ�������Щ����
				if(ftemp->num==0)
					genIntraRepreStructBdd(fdd_ithvar(V1,ftemp->item)&fdd_ithvar(F1,0),callbdd,calleebdd,bdditem,bddCall2Return,false);
				else
					genIntraRepreStructBdd(bdditem,callbdd,calleebdd,bdditem,bddCall2Return,false);*/
			}
			pawalker.add2TempFunTable(tempvbdd,tempvFuncPara,bddItem,ftemp);//��������Ӧ��bdd���뵽tempvbdd�У�ָ���ϵ���뵽tempvFuncPara��
			ftemp++;
			bdd bddItem1=pawalker.mypa.pointto&bddItem;
			if(bddfalse==bddItem1)//ָ�뱻û��ָ���κζ���
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
	//�õ��ڵ��Ѿ���¼��ģʽ
	patternBdd=pawalker.mypa.circleInPointerResult&fdd_ithvar(C,globalIndex);
	if(bddfalse==patternBdd)//ģʽΪ��
	{
		return bddfalse;
	}
	//����ÿһ��ģʽ���鿴�Ƿ�ƥ�䵱ǰ��pawalker.mypa.pointto
	int calledNum=curNode->getCalledNum();
	for(int i=1;i<=calledNum;i++)
	{
		bdd onePattern=patternBdd&fdd_ithvar(K,i);//ȡ��ÿһ�α�����ʱ��ģʽ
		onePattern=bdd_exist(onePattern,fdd_ithset(K)&fdd_ithset(C));
		if(bddfalse==onePattern-pawalker.mypa.pointto)//�����ǰģʽ���Լ�¼ģʽ���Ӽ���ģʽƥ��
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
			if(bddfalse==bddMatched)//���Ϊ�գ������ǵ�ǰ��Ӧ��֧��û�б�����
			{
				return bddfalse;
			}
			bddMatched=bdd_exist(bddMatched,fdd_ithset(K)&fdd_ithset(D)&fdd_ithset(C));
			return bddMatched; 
		}else{//ģʽ��ƥ�䣬����ǰģʽ��¼����
			curNode->addCalledNum();
			bdd patternSaved=pawalker.mypa.pointto&fdd_ithvar(K,curNode->getCalledNum())&fdd_ithvar(C,globalIndex)&fdd_ithvar(D,1);
			pawalker.mypa.circleInPointerResult|=patternSaved;
		}
	}
	return bddfalse;
}*/
/*
���ã�����ѭ���ڵ�ʱ���жϷ�������ǰ�ڵ��ָ��ģʽ�Ƿ��������ֹ�
������curNode��ָ��ǰ�����Ľڵ�
	  path��TurePath��ʾ��ǰ��֤����ѭ���ڵ�����֧��FalsePath��ʾ��ǰ��֤����ѭ���ڵ�ļٷ�֧
��ע���ڼ�¼ģʽ��ʱ����D���ʾ��֧��1��ʾ�ٷ�֧��2��ʾ���֧
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
	//�õ��ڵ��Ѿ���¼��ģʽ
	patternBdd=pawalker.mypa.circleInPointerResult&fdd_ithvar(C,globalIndex);

	if(bddfalse==patternBdd)//ģʽΪ��
	{
		return false;
	}
	//����ÿһ��ģʽ���鿴�Ƿ�ƥ�䵱ǰ��pawalker.mypa.pointto
	int calledNum=curNode->getCalledNum();
	for(int i=1;i<=calledNum;i++)
	{
		/*֮�����ȸ�pointto����C������Ϊ�����ʱģʽΪ�գ���ô��Ч��ֻ��C��K�������ʱ��C��K��ȫȥ�����������bddfalse����bddtrue*/
		bdd onePattern=patternBdd&fdd_ithvar(K,i);//ȡ��ÿһ�α�����ʱ��ģʽ
		onePattern=bdd_exist(onePattern,fdd_ithset(K));
		bdd temp;
		/*���pointtoΪbddfalse����ʱ�����pointto & fdd_ithvar(C,1)�ķ�ʽ�������C�򣬵õ�����Ȼ��bddfalse
		  */
		if(bddfalse == pawalker.mypa.pointto)
		{
			temp=fdd_ithvar(C,globalIndex);
		}else{
			temp=pawalker.mypa.pointto & fdd_ithvar(C,globalIndex);
		}
		if(bddfalse==onePattern-temp)//�����ǰģʽ���Լ�¼ģʽ���Ӽ���ģʽƥ��
		{
			ret=true;
			break;
		}
	}
#endif
	return false;
}
/*
���ã�����ѭ���ڵ�ʱ�������Ƿ����ѭ�����Ժ��·��,�����һ���µ�ģʽ�����¼����
������curNode��ָ��ǰ�����Ľڵ�
	  path��TurePath��ʾ��ǰ��֤����ѭ���ڵ�����֧��FalsePath��ʾ��ǰ��֤����ѭ���ڵ�ļٷ�֧
��ע���ڼ�¼ģʽ��ʱ����D���ʾ��֧��1��ʾ�ٷ�֧��2��ʾ���֧
*/
bool isContinueRun(PATreeWalker& pawalker,VexNode* curNode)
{
	bool ret=true;
	if(vCircle==curNode->getNodeType()|| vSwitch==curNode->getNodeType() || vIf==curNode->getNodeType() || vDoWhile==curNode->getNodeType())//�����ѭ��������ģʽƥ��
	{
		if(vCircle == curNode->getNodeType() || vDoWhile == curNode->getNodeType())//�����ѭ��������ģʽƥ��
		{
			curNode->addCalledNum();
			if(curNode->getCalledNum() > 10)
			{
				//curNode->resetCalledNum();
				return false;
			}
		}
		
		if(vIf == curNode->getNodeType() || vSwitch == curNode->getNodeType())//�����ѭ��������ģʽƥ��
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
		if(matchCircleInPattern(pawalker,curNode))//�鿴�Ƿ���ƥ���ģʽ��û�еĻ���¼����
		{
#ifdef ANALYZE_OPT_TRACE
			openConsole();
			cout<<"branch node compatable,quit current path...."<<endl;
			closeConsole();
#endif
			ret=false;   
		}else{
			/*//�洢����µ�ģʽ
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



