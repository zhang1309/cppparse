// by kong 20100520
#ifndef INFORFLOWCOLLECTOR_H
#define INFORFLOWCOLLECTOR_H

#include "../buddy/bdd.h"
#include "../buddy/fdd.h"
#include "../SymTable/ScopeSymbolTable.h"
#include "../SymTable/SymbolTable.h"

class InforFlowCollector {
public:
    bdd CCAll; // 用于收集一个函数中最终生成的所有信息流
	bdd CCAPath; // 用于记录一条数据流路径中生成的信息流，主要为了避免不同路径的信息流交叉
	bdd CCBranch; // 用于记录分析语句产生的信息流
	bdd CCAFunc; // 记录一个函数当前的所有信息流
    bdd CCToBeDeleted; // 记录需要删除的信息流

	//PATreeWalker& pawalker;
    InforFlowCollector();
	//void addInforFlow(PATreeWalker& pawalker, bdd curBdd);
};

#endif
