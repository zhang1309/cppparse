#pragma warning(disable:4786)

// by kong 20100520
#include "InforFlowCollector.h"
#include <fstream>
#include "../PA/pa.h"
/*
enum DomainType {
	V1=0, V2, F1, F2, M1, M2, C, K, P, T1, T2, D, Sc, Z
};  
*/
extern float gfVariableCountFactor;
extern int giTotalPathCount;
extern int giTotalVexCount;
extern ofstream futeng;
InforFlowCollector::InforFlowCollector() {
	//int domian[] = {1000,1000};   //域，用来编码,每个域中整型的范围只能是（0-n-1）
	//fdd_extdomain(domian,2);   //2个域
	int mydomain[14];
	mydomain[V1]=mydomain[V2]=(ScopeSymbolTable::getVariableCount())*gfVariableCountFactor+100;
	mydomain[F1]=mydomain[F2]=(ScopeSymbolTable::getVariableCount())*gfVariableCountFactor+100;
	mydomain[M1]=mydomain[M2]=ScopeSymbolTable::getVariableCount()+100;
	mydomain[T1]=mydomain[T2]=ScopeSymbolTable::getTypeCount()+100;
	mydomain[K]=5000;
	mydomain[P]=giTotalPathCount;
	mydomain[C]=giTotalVexCount;
	mydomain[D]=5000;
	mydomain[Sc]=SymbolTable::getAllScopeNumber()+100;//(ScopeSymbolTable::)
	mydomain[Z] = 5000; // by kong 20100520
/*
	paTrace<<"mydomain[V1] = "<<mydomain[V1]<<endl;
	paTrace<<"mydomain[V2] = "<<mydomain[V2]<<endl;
	paTrace<<"mydomain[F1] = "<<mydomain[F1]<<endl;
	paTrace<<"mydomain[F2] = "<<mydomain[F2]<<endl;
	paTrace<<"mydomain[M1] = "<<mydomain[M1]<<endl;
	paTrace<<"mydomain[M2] = "<<mydomain[M2]<<endl;
	paTrace<<"mydomain[T1] = "<<mydomain[T1]<<endl;
	paTrace<<"mydomain[T2] = "<<mydomain[T2]<<endl;
	paTrace<<"mydomain[K] = "<<mydomain[K]<<endl;
	paTrace<<"mydomain[P] = "<<mydomain[P]<<endl;
	paTrace<<"mydomain[C] = "<<mydomain[C]<<endl;
	paTrace<<"mydomain[D] = "<<mydomain[D]<<endl;
	paTrace<<"mydomain[Sc] = "<<mydomain[Sc]<<endl;
	paTrace<<"mydomain[Z] = "<<mydomain[Z]<<endl;
*/
	fdd_extdomain(mydomain,14);
	CCAll = bddfalse; 
    CCAPath = bddfalse;
    CCBranch = bddfalse;
	CCAFunc = bddfalse;
	CCToBeDeleted = bddfalse;
}
