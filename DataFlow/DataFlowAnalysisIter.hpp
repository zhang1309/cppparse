#ifndef __DATAFLOWANALYSISITER_HPP__
#define __DATAFLOWANALYSISITER_HPP__

#include "SetStructure.hpp"

extern ofstream dataflowIter;
extern list<Definition*> allDef;	// �������ж�ֵ����
extern map<VexNode*, list<Definition*> > gen;	// ����gen����
extern map<VexNode*, list<Definition*> > kill;	// ����kill����
// 1205extern map<SymbolItem*, list<Definition*> > sblDefSet;
extern map<QualifiedSymbolItem, list<Definition*> > sblDefSet;	// Ϊ������Ŀ���������ж�ֵ

void walkTree();
void clearDefinition();

#endif