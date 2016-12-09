#ifndef __DATAFLOWANALYSISITER_HPP__
#define __DATAFLOWANALYSISITER_HPP__

#include "SetStructure.hpp"

extern ofstream dataflowIter;
extern list<Definition*> allDef;	// 保存所有定值集合
extern map<VexNode*, list<Definition*> > gen;	// 保存gen集合
extern map<VexNode*, list<Definition*> > kill;	// 保存kill集合
// 1205extern map<SymbolItem*, list<Definition*> > sblDefSet;
extern map<QualifiedSymbolItem, list<Definition*> > sblDefSet;	// 为符号条目保存其所有定值

void walkTree();
void clearDefinition();

#endif