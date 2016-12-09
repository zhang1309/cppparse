#ifndef ASSISTANT_H
#define ASSISTANT_H

#pragma warning(disable:4786)
#include "ValueStructure.hpp"
#include "../CFG/CFGStructure.h"
#include <map>


int octAtoi(const char*);		// 表示八进制数的字符串转化为整数
int hexAtoi(const char*);		// 表示十六进制数的字符串转化为整数

bool isChildScope(ScopeSymbolTable*c, ScopeSymbolTable* p);		// 判断符号表c是否是p的子符号表
bool isBrotherScope(ScopeSymbolTable*b1, ScopeSymbolTable* b2);	// 判断两个符号表是否是兄弟

VexNode* getTailVexNode(VexNode* v, bool b);	// 获取v节点b边的后继节点

void calculateGlobalVariableValues(VexValueSet* vs);		// 计算全局整型变量的值, 结果写入参数

bool isMemberOfType(SymbolItem*, Type*);		// 某个符号是否是某个类型的成员, 如是否是某个类的成员属性

class PATreeWalker;
IntegerValueSet getIntValuesFromAst(RefMyAST ast, VexNode* v, VexNode* preVex,PATreeWalker& pawalker);

#endif