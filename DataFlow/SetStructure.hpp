#ifndef __SET_HPP__
#define __SET_HPP__
#pragma warning(disable:4786)
#include <map>
#include <list>
using namespace std;

#include "../SymTable.hpp"
class VexNode;

//1205
typedef list<SymbolItem*> QualifiedSymbolItem;
typedef list<SymbolItem*>::iterator Iter;
typedef list<SymbolItem*>::reverse_iterator Riter;

struct Definition					// "��ֵ"��
{
	QualifiedSymbolItem defSbl;		// ������ֵ�ķ�����Ŀ
	VexNode* defVex;				// ���ĸ��ڵ��ϱ���ֵ
};

class InOutSet	// "��ֵ"����
{
public:
	map<QualifiedSymbolItem, list<Definition*> >defMap;
public:
	InOutSet();
	InOutSet(const InOutSet&);
	InOutSet(list<Definition*>&);
	
	list<Definition*> findDefOnSbl(QualifiedSymbolItem);
	
	InOutSet& operator = (const InOutSet&); 
	InOutSet& operator = (list<Definition*>&);	//out=gen
	InOutSet& operator+=(InOutSet&);
	InOutSet operator - (list<Definition*>&);		// in-kill
	bool operator !=( InOutSet&);
	
	friend InOutSet operator + (list<Definition*>&, InOutSet&); // gen+(in-kill)
};

list<Definition*>& operator+= (list<Definition*>&, list<Definition*>&);

#endif