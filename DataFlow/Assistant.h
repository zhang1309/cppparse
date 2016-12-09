#ifndef ASSISTANT_H
#define ASSISTANT_H

#pragma warning(disable:4786)
#include "ValueStructure.hpp"
#include "../CFG/CFGStructure.h"
#include <map>


int octAtoi(const char*);		// ��ʾ�˽��������ַ���ת��Ϊ����
int hexAtoi(const char*);		// ��ʾʮ�����������ַ���ת��Ϊ����

bool isChildScope(ScopeSymbolTable*c, ScopeSymbolTable* p);		// �жϷ��ű�c�Ƿ���p���ӷ��ű�
bool isBrotherScope(ScopeSymbolTable*b1, ScopeSymbolTable* b2);	// �ж��������ű��Ƿ����ֵ�

VexNode* getTailVexNode(VexNode* v, bool b);	// ��ȡv�ڵ�b�ߵĺ�̽ڵ�

void calculateGlobalVariableValues(VexValueSet* vs);		// ����ȫ�����ͱ�����ֵ, ���д�����

bool isMemberOfType(SymbolItem*, Type*);		// ĳ�������Ƿ���ĳ�����͵ĳ�Ա, ���Ƿ���ĳ����ĳ�Ա����

class PATreeWalker;
IntegerValueSet getIntValuesFromAst(RefMyAST ast, VexNode* v, VexNode* preVex,PATreeWalker& pawalker);

#endif