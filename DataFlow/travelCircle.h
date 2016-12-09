#ifndef TRAVEL_CIRCLE_H
#define TRAVEL_CIRCLE_H

#include "../SymTable/SymbolItem.h"
#include "../CFG/CFGStructure.h"
#include "../PATreeWalker.hpp"

void genPASCall(PATreeWalker& pawalker,bdd variableInFunc,vector<bdd>& tempvbdd,vector<FunParaTemp*>& tempvFuncPara);
bool travelCircleAgain(SymbolItem *funcSymbolItem,PATreeWalker& pawalker,VexNode* curVex,bool isFirstTraveled);


bdd pointerPatternMatching(PATreeWalker &pawalker,bdd &heapOut,VexNode *curNode,pathToTravel path);
/*
���ã�����ѭ���ڵ�ʱ���жϷ�������ǰ�ڵ��ָ��ģʽ�Ƿ��������ֹ�
������curNode��ָ��ǰ�����Ľڵ�
	  path��TurePath��ʾ��ǰ��֤����ѭ���ڵ�����֧��FalsePath��ʾ��ǰ��֤����ѭ���ڵ�ļٷ�֧
��ע���ڼ�¼ģʽ��ʱ����D���ʾ��֧��1��ʾ�ٷ�֧��2��ʾ���֧
*/
bool matchCircleInPattern(PATreeWalker &pawalker,VexNode *curNode);
/*
���ã�����ѭ���ڵ�ʱ�������Ƿ����ѭ�����Ժ��·��
������curNode��ָ��ǰ�����Ľڵ�
	  path��TurePath��ʾ��ǰ��֤����ѭ���ڵ�����֧��FalsePath��ʾ��ǰ��֤����ѭ���ڵ�ļٷ�֧
��ע���ڼ�¼ģʽ��ʱ����D���ʾ��֧��1��ʾ�ٷ�֧��2��ʾ���֧
*/
bool isContinueRun(PATreeWalker& pawalker,VexNode* curNode);
#endif