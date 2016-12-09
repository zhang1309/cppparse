#ifndef TRAVEL_CIRCLE_H
#define TRAVEL_CIRCLE_H

#include "../SymTable/SymbolItem.h"
#include "../CFG/CFGStructure.h"
#include "../PATreeWalker.hpp"

void genPASCall(PATreeWalker& pawalker,bdd variableInFunc,vector<bdd>& tempvbdd,vector<FunParaTemp*>& tempvFuncPara);
bool travelCircleAgain(SymbolItem *funcSymbolItem,PATreeWalker& pawalker,VexNode* curVex,bool isFirstTraveled);


bdd pointerPatternMatching(PATreeWalker &pawalker,bdd &heapOut,VexNode *curNode,pathToTravel path);
/*
作用：遇到循环节点时，判断分析到当前节点的指针模式是否曾经出现过
参数：curNode：指向当前分析的节点
	  path：TurePath表示当前验证的是循环节点的真分支；FalsePath表示当前验证的是循环节点的假分支
备注：在记录模式的时候用D域表示分支，1表示假分支，2表示真分支
*/
bool matchCircleInPattern(PATreeWalker &pawalker,VexNode *curNode);
/*
作用：遇到循环节点时，决定是否分析循环及以后的路径
参数：curNode：指向当前分析的节点
	  path：TurePath表示当前验证的是循环节点的真分支；FalsePath表示当前验证的是循环节点的假分支
备注：在记录模式的时候用D域表示分支，1表示假分支，2表示真分支
*/
bool isContinueRun(PATreeWalker& pawalker,VexNode* curNode);
#endif