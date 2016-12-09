#include "SetStructure.hpp"

extern ofstream dataflow;

InOutSet::InOutSet(){;}
InOutSet::InOutSet(const InOutSet& s){defMap = s.defMap;}
InOutSet::InOutSet(list<Definition*>& lst)
{
	*this = lst;
}

// oldOut = out     ok
InOutSet& InOutSet::operator = (const InOutSet& s)
{
	defMap.clear();	// mdfy 22
	defMap = s.defMap;  
	return *this;
}

// out=gen   ok
InOutSet& InOutSet::operator = (list<Definition*>& lst)
{
	list<Definition*>::iterator it;
	this->defMap.clear();
	for( it=lst.begin(); it!=lst.end(); it++ )
	{
		Definition* def = *it;
		list<Definition*>& defs = defMap[def->defSbl];
		defs.push_back(def);
	}
	return *this;
}

// in+=out   ok
InOutSet& InOutSet::operator+=(InOutSet& s)
{
//1205 	map<SymbolItem*, list<Definition*> >::iterator it;
	map<QualifiedSymbolItem, list<Definition*> >::iterator it;
	for( it=s.defMap.begin(); it!=s.defMap.end(); it++)
	{
//1205 		SymbolItem* sbl = it->first;
		QualifiedSymbolItem sbl = it->first; 
		list<Definition*> lst = it->second;
		list<Definition*>& rdefs = defMap[sbl];
		rdefs.merge(lst);
	}

	for(it=defMap.begin(); it!=defMap.end(); it++)
	{
		list<Definition*>& defs = it->second;
		defs.unique();
	}

	return *this;
}

// in-kill  ok
InOutSet InOutSet::operator - (list<Definition*>& lst)
{
	InOutSet result(*this);
	list<Definition*>::iterator it;
	for(it=lst.begin(); it!=lst.end(); it++)
	{
		Definition* def = *it;
		list<Definition*>& defs = result.defMap[def->defSbl];
		defs.remove(def);
	}
	return result;
}

// kill += ... ok
list<Definition*>& operator+= (list<Definition*>& sd, list<Definition*>& lst)
{
	list<Definition*>::iterator it;
	for(it=lst.begin(); it!=lst.end(); it++)
	{
		sd.push_back(*it);
	} 
	sd.unique();
	return sd;
}

// gen + (in-kill)   ok
InOutSet operator + (list<Definition*>& lst, InOutSet& sd)
{
	list<Definition*>::iterator it;
	InOutSet result(sd);
	for(it=lst.begin(); it!=lst.end(); it++)
	{
		Definition* def = *it;
		list<Definition*>& defs = result.defMap[def->defSbl];
		defs.push_back(def);
	}
//1205 	map<SymbolItem*, list<Definition*> >::iterator mit;
	map<QualifiedSymbolItem, list<Definition*> >::iterator mit;
	for(mit=result.defMap.begin(); mit!=result.defMap.end(); mit++)
	{
//1205		SymbolItem* sbl = mit->first;
		QualifiedSymbolItem sbl = mit->first;
		list<Definition*>& defs = result.defMap[sbl];
		defs.unique();
	}
	return result;
}

// ok
bool InOutSet::operator != (InOutSet& sd)
{
//1205	map<SymbolItem*, list<Definition*> >::iterator it;
	map<QualifiedSymbolItem, list<Definition*> >::iterator it;
	list<Definition*> defs1, defs2;
	for(it=defMap.begin(); it!=defMap.end(); it++)
	{
		list<Definition*> tmpLst = it->second;
		defs1.merge(tmpLst);
	}
	for(it=sd.defMap.begin(); it!=sd.defMap.end(); it++)
	{
		list<Definition*> tmpLst = it->second;
		defs2.merge(tmpLst); 
	}
	defs1.sort(); defs2.sort();
	if(defs1!=defs2)return true;
	return false;
}

