//////////////////////////////////////////////////////////////////////////
//	Author: Day															//
//																		//
//////////////////////////////////////////////////////////////////////////


#ifndef __VALUE_STRUCTURE__
#define __VALUE_STRUCTURE__
#pragma warning(disable:4786)  // ����4786����, �������ʱ������Ϣջ���. �þ������ʹ��STLʱ����.

#include "../SymTable.hpp"
#include <list>
#include <set>
#include <map>
#include <deque>
using namespace std;

extern map<int, SymbolItem*> heapObjs;
bool isHeapObjs(SymbolItem* sbl);

//////////////////////////////////////////////////////////////////////////
// һ��IntegerValue�����װһ������ֵ, ��������һ�����ͱ�����ֵ
// ����һ���������ܴ���ֵ��δ֪�����, �����һ������unknown����ʾ
//////////////////////////////////////////////////////////////////////////
class IntegerValue;
ostream& operator << (ostream&, const IntegerValue&);

class IntegerValue
{
private:
	bool unknown;		// ֵδ֪, ��δ��ʼ��, �������Զ�̬����
	bool guessing;		// ֵ�Ƿ����Ʋ�����, ��λ��ѭ�������еı��ʽ, ��Ϊ��Ӧ��������һ����Ӧ���Ʋ�ֵ
						// ��while(i<=10), Ϊi����һ��10��ֵ, ������guessing����Ϊ��
	long value;
	unsigned int line;	// ��ֵ������Դ��������һ��
public:
	IntegerValue():unknown(true), guessing(false){}
	IntegerValue(long v, bool guess=false):value(v), unknown(false), guessing(guess){}
	IntegerValue(const IntegerValue& r):value(r.value), unknown(r.unknown), guessing(r.guessing){line = r.line;}
	IntegerValue& operator=( const IntegerValue& r)
	{
		value = r.getValue(); unknown = r.isUnknown(); line = r.line;
		guessing = r.guessing; return *this;
	}
	bool isLessThan (const IntegerValue& r);
	bool isUnknown()const		// ����ֵ�Ƿ�δ֪
	{
		return unknown;
	}
	void setUnknown(bool b)		// ����ֵ�Ƿ�δ֪
	{
		unknown = b;
	}

	bool isGuessing()const		// ����ֵ�Ƿ�Ϊ�²�
	{
		return guessing;
	}
	void setGuessing(bool b)	// ����ֵ�Ƿ�Ϊ�²�
	{
		guessing = b;
	}

	void setValue(long l)		// ����ֵ
	{
		value = l; unknown = false;
	}
	long getValue()const		// ��ȡֵ
	{
		return value;
	}

	unsigned int getLine()const		// ��ȡֵ������Դ�ļ��е�����
	{
		return line;
	}
	void setLine(unsigned int l)	// ����ֵ������Դ�ļ��е�����
	{
		line = l;
	}
	
	// �����߼�����... 

	//int operator < (const IntegerValue& r);

	IntegerValue operator+( const IntegerValue& r)const;
	IntegerValue operator-( const IntegerValue& r)const;
	IntegerValue operator*( const IntegerValue& r)const;
	IntegerValue operator/( const IntegerValue& r)const;
	IntegerValue operator%( const IntegerValue& r)const;
	IntegerValue operator<<( const IntegerValue& r)const;
	IntegerValue operator>>( const IntegerValue& r)const;

	IntegerValue operator&( const IntegerValue& r)const;
	IntegerValue operator|( const IntegerValue& r)const;
	IntegerValue operator^( const IntegerValue& r)const;
	IntegerValue operator~()const;

	IntegerValue& operator+=( const IntegerValue& r);
	IntegerValue& operator-=( const IntegerValue& r);
	IntegerValue& operator*=( const IntegerValue& r);
	IntegerValue& operator/=( const IntegerValue& r);
	IntegerValue& operator%=( const IntegerValue& r);
	IntegerValue& operator<<=( const IntegerValue& r);
	IntegerValue& operator>>=( const IntegerValue& r);

	IntegerValue& operator &=(const IntegerValue& r);
	IntegerValue& operator |=(const IntegerValue& r);
	IntegerValue& operator ^=(const IntegerValue& r);

	IntegerValue operator++();
	IntegerValue operator++(int);
	IntegerValue operator--();
	IntegerValue operator--(int);
	IntegerValue operator-()const;

	IntegerValue operator && (const IntegerValue&);
	IntegerValue operator || (const IntegerValue&);
	IntegerValue operator !  ();
	IntegerValue operator <= (const IntegerValue&);
	IntegerValue operator >= (const IntegerValue&);
	IntegerValue operator >  (const IntegerValue&);	
	IntegerValue operator != (const IntegerValue&);
	IntegerValue operator == (const IntegerValue&);  
	bool operator < (const IntegerValue& r);
	friend ostream& operator<<(ostream&, const IntegerValue&);

	bool operator<(const IntegerValue& r)const;	// IntegerValue��Ϊset��ģ�����ʱ, ���붨��÷���

};


/************************************************************************/
/* IntegerValueSet����һ�����ͱ������еĿ���ֵ��װ����                */
/************************************************************************/
class IntegerValueSet;
ostream& operator << (ostream&, const IntegerValueSet&);

class IntegerValueSet
{
private:
	set<IntegerValue> values;
public:
	IntegerValueSet(){}
	IntegerValueSet(const set<IntegerValue>& v):values(v){}
	IntegerValueSet(const IntegerValue& v){values.insert(v);}

	bool findGuessingOrUnknow()const;
	const set<IntegerValue>& getValues()const		// ����ֵ����
	{return values;}
	void setValues(const set<IntegerValue>& v)		// ����ֵ����
	{values = v;}

	void setLine(unsigned int l, bool guessAlsoSet = false);	// ����ֵ����������ֵ������Դ�ļ��е�����
	void setGuessing(bool g);						// ����ֵ����������ֵ�Ƿ�Ϊ�²�

	void addValue(const IntegerValue& v)			// ���뵥��ֵ
	{
		values.insert(v);
	}
	void addValues(const IntegerValueSet& v);		// ����һ��ֵ����

	bool empty()				// ֵ�����Ƿ�Ϊ��
	{return values.empty();}
	void clear()				// ��ռ���
	{values.clear();}

	IntegerValue getMinValue();		// ���ؼ�������С��ֵ
	IntegerValue getMaxValue();		// ���ؼ���������ֵ

	IntegerValueSet& operator = (const IntegerValueSet& r)
	{
		values.clear();
		values = r.values; return *this;
	}
	IntegerValueSet operator + (const IntegerValueSet& r)const;
	IntegerValueSet operator - (const IntegerValueSet& r)const;
	IntegerValueSet operator * (const IntegerValueSet& r)const;
	IntegerValueSet operator / (const IntegerValueSet& r)const;
	IntegerValueSet operator % (const IntegerValueSet& r)const;

	IntegerValueSet operator & (const IntegerValueSet& r)const;
	IntegerValueSet operator | (const IntegerValueSet& r)const;
	IntegerValueSet operator ^ (const IntegerValueSet& r)const;
	IntegerValueSet operator ~ ()const;

	IntegerValueSet operator << (const IntegerValueSet& r)const;
	IntegerValueSet operator >> (const IntegerValueSet& r)const;
	IntegerValueSet& operator += (const IntegerValueSet& r);
	IntegerValueSet& operator -= (const IntegerValueSet& r);
	IntegerValueSet& operator *= (const IntegerValueSet& r);
	IntegerValueSet& operator /= (const IntegerValueSet& r);
	IntegerValueSet& operator %= (const IntegerValueSet& r);
	IntegerValueSet& operator <<= (const IntegerValueSet& r);
	IntegerValueSet& operator >>= (const IntegerValueSet& r);

	IntegerValueSet& operator &= (const IntegerValueSet& r);
	IntegerValueSet& operator |= (const IntegerValueSet& r);
	IntegerValueSet& operator ^= (const IntegerValueSet& r);

	IntegerValueSet operator ++ ();
	IntegerValueSet operator ++ (int);
	IntegerValueSet operator -- ();
	IntegerValueSet operator -- (int);
	IntegerValueSet operator - ()const;

	IntegerValueSet operator && (const IntegerValueSet&);
	IntegerValueSet operator || (const IntegerValueSet&);
	IntegerValueSet operator !  ();
	IntegerValueSet operator <  (const IntegerValueSet&);
	IntegerValueSet operator <= (const IntegerValueSet&);
	IntegerValueSet operator >  (const IntegerValueSet&);
	IntegerValueSet operator >= (const IntegerValueSet&);
	IntegerValueSet operator == (const IntegerValueSet&);
	IntegerValueSet operator != (const IntegerValueSet&);

	friend ostream& operator << (ostream&, const IntegerValueSet&);

	void unionValues(const IntegerValueSet& r);
};


//////////////////////////////////////////////////////////////////////////
// һ�����������ı�ʾ, ��												
// class A{public:int x;};												
// A a;																	
// a.x; a.x��������ʾ��{a,x}												                          
//////////////////////////////////////////////////////////////////////////
class MingledSymbolItem;
ostream& operator << (ostream&, const MingledSymbolItem&);

class MingledSymbolItem
{
private:
	// ��һ��˫�˶��б�����ŵ�������ʾ, �����a.x���øö��б���a��x������Ŀ
	deque<SymbolItem *> sblFullName;	
public:
	MingledSymbolItem(){}
	MingledSymbolItem(SymbolItem* sbl){
		if( sbl != NULL )
			sblFullName.push_back(sbl);
	}
	MingledSymbolItem(const deque<SymbolItem*> s){sblFullName = s;}
	MingledSymbolItem(const MingledSymbolItem& s){sblFullName=s.sblFullName;}
	MingledSymbolItem& operator= (const MingledSymbolItem& r)
	{
		sblFullName = r.sblFullName;
		return *this;
	}
	
	const deque<SymbolItem*>& getSblFullName()const		// ���ط���������ʾ�Ķ���
	{return sblFullName;}
	void addSubSblItem(SymbolItem* s)					// ����Ŷ��������һ�������Ŀ
	{sblFullName.push_back(s);}

	//
	void addSubSblItem_head(SymbolItem* s)					// ����Ŷ��������һ�������Ŀ
	{sblFullName.push_front(s);}
	//dzh	
	void addMingleItem(MingledSymbolItem s);
	//dzn end

	bool isGlobalSymbol()const;					// �ж�һ��������ʾ�ķ����Ƿ�Ϊȫ�ַ���
												// a.b.c������Ŀ�Ƿ�ȫ�ֱ���ȡ����a�Ƿ���ȫ�ֱ���	
	SymbolItem* getLastSymbol()const;			// �������һ�������Ŀ
	SymbolItem* getFirstSymbol()const;			// ���ص�һ�������Ŀ
	SymbolTable* getParentSymbolTable()const;	// ���ط���������������, a.b.c��������Ӧ����a��������

	Type* getType()const;						// ���ط��ŵ�����, a.b.c����c������
	
	void setHeader(const MingledSymbolItem&);	// ����Ŷ��е�ͷ���в���һ�����Ŷ���
	MingledSymbolItem getHeader()const;			// ����һ������������ʾ�Ķ�����ǰn-1��������Ŀ����	

	bool empty()const							// ������Ŀ�����Ƿ�Ϊ��
	{return sblFullName.empty();}
	void clear()								// ��շ�����Ŀ����
	{sblFullName.clear();}
	int size()const{return sblFullName.size();}

	bool isIntegerType()const;					// �����Ƿ���������Ŀ
	bool isUnsignedIntegerType()const;			// �����Ƿ����޷���������Ŀ
	bool isRefIntegerType()const;				// �����Ƿ���������������
	bool isReferenceType()const;				// �����Ƿ�����������
	bool isPointerIntegerType()const;			// �����Ƿ�������ָ������
	bool isPointerType()const;					// �����Ƿ���ָ������
	bool isStatic()const;						// �����Ƿ�����Ϊ��̬
	bool isFloatType()const;					// �����Ƿ��Ǹ�����

	bool isMemberOf(const MingledSymbolItem& parent)const;	// �����Ƿ�����һ����������ĳ�Ա

	// 090512
	bool startWith(const MingledSymbolItem&)const;		// ������Ŀ���е�ǰ�������Ƿ��ǲ�����ָ���ķ��Ŷ���
	void replaceStartWith(const MingledSymbolItem&);	// ������Ŀ���е�ǰ�������ò���ָ���Ķ����滻
	
	bool operator==(const MingledSymbolItem& r)const;
	bool operator < (const MingledSymbolItem& r)const
	{
		return (sblFullName<r.sblFullName);
	}

	bool hasRefElement()const;
	bool hasRefOrPtrElement()const;
	SymbolItem* getLastPointerSbl()const;
	MingledSymbolItem getStaticParts()const;

	friend ostream& operator<< (ostream&, const MingledSymbolItem&);

	MingledSymbolItem SubSequence(int begin, int end);
};



//////////////////////////////////////////////////////////////////////////
// ����CFG�ڵ��ϵ����ͱ���ֵ����
//////////////////////////////////////////////////////////////////////////
 
class VexValueSet;
ostream& operator << (ostream&, const VexValueSet&);

class VexValueSet
{
private:
	// ����map���������ͱ���������Ӧ�����ͱ���ֵ����,
	// ��һ����MingledSymbolItem��ʾ�ķ�����Ŀӳ��Ϊһ������ֵ����
	// ����map����һ��{{a,b}, {100,20}}, ��ʾa.b��ֵ����Ϊ100����20
	map<MingledSymbolItem, IntegerValueSet> vexValues;
public:
	VexValueSet(){
		vexValues.clear();
	}
	~VexValueSet(){
		vexValues.clear();
		//cout<<__FILE__<<":"<<__LINE__<<" vexValues.size():"<<vexValues.size()<<endl;
		//cout<<__FILE__<<":"<<__LINE__<<" this is:"<<endl<<"\t"<<*this<<endl;
	}
	VexValueSet(const VexValueSet& r):vexValues(r.vexValues){  }
	VexValueSet(const map<MingledSymbolItem, IntegerValueSet>& m){vexValues = m;}
	VexValueSet& operator = (const VexValueSet& r)
	{
		vexValues = r.vexValues;
		return *this;	
	}

	const map<MingledSymbolItem, IntegerValueSet>& getVexValues()const
	{ return vexValues; }
	void clear()
	{	vexValues.clear();	}
	
	// ������������������, ����һ��������Ŀ���ʸ���Ŀ�����ͱ���ֵ
	IntegerValueSet& operator [] (const MingledSymbolItem& sbl)
	{ return vexValues[sbl]; }

	// �ӱ�map�����ҳ���srcSbl��ͷ�ķ��Ŷ�Ӧ����, ���뵽vs��, ����vs�н�����ͷ��ΪdestSbl
	// �÷�����Ҫ���ڷ�����������ʱ, ���ṹ���Ͷ���Ĳ�������
	// ��ԭ��Ϊf(A in)�ĺ�����һ�ξ������f(obj), �����õ㴦obj�����е����ͳ�Ա��ֵӳ�䵽in�е����ͳ�Ա, 
	// ��д�뺯��f�������ֵ
	void copyValuesOfSblHeaderAndChangeHeader(const MingledSymbolItem& srcSbl, const MingledSymbolItem& destSbl, 
		VexValueSet& vs);
	// �ӱ�map�����ҳ��ض���ͷ�ķ��Ŷ�Ӧ����, ���Ƶ�Ŀ�꼯����
	// �÷�����Ҫ������һ�������ϵ���һ������ʱ, ��obj.f(), ��obj�����е����ͳ�Ա��ֵд�뺯��f�������ֵ
	void copyValuesOfSblHeader(const MingledSymbolItem&, VexValueSet&);

	bool find(const MingledSymbolItem& sbl)const
	{
		return vexValues.find(sbl) != vexValues.end();
	}

	VexValueSet operator + (const VexValueSet& r)const;
	VexValueSet& operator += (const VexValueSet&);
	friend ostream& operator << (ostream&, const VexValueSet&);
};

#endif