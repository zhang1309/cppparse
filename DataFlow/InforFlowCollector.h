// by kong 20100520
#ifndef INFORFLOWCOLLECTOR_H
#define INFORFLOWCOLLECTOR_H

#include "../buddy/bdd.h"
#include "../buddy/fdd.h"
#include "../SymTable/ScopeSymbolTable.h"
#include "../SymTable/SymbolTable.h"

class InforFlowCollector {
public:
    bdd CCAll; // �����ռ�һ���������������ɵ�������Ϣ��
	bdd CCAPath; // ���ڼ�¼һ��������·�������ɵ���Ϣ������ҪΪ�˱��ⲻͬ·������Ϣ������
	bdd CCBranch; // ���ڼ�¼��������������Ϣ��
	bdd CCAFunc; // ��¼һ��������ǰ��������Ϣ��
    bdd CCToBeDeleted; // ��¼��Ҫɾ������Ϣ��

	//PATreeWalker& pawalker;
    InforFlowCollector();
	//void addInforFlow(PATreeWalker& pawalker, bdd curBdd);
};

#endif
