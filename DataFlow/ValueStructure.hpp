//////////////////////////////////////////////////////////////////////////
//	Author: Day															//
//																		//
//////////////////////////////////////////////////////////////////////////


#ifndef __VALUE_STRUCTURE__
#define __VALUE_STRUCTURE__
#pragma warning(disable:4786)  // 消除4786警告, 避免编译时编译信息栈溢出. 该警告多由使用STL时产生.

#include "../SymTable.hpp"
#include <list>
#include <set>
#include <map>
#include <deque>
using namespace std;

extern map<int, SymbolItem*> heapObjs;
bool isHeapObjs(SymbolItem* sbl);

//////////////////////////////////////////////////////////////////////////
// 一个IntegerValue对象封装一个整型值, 用来保存一个整型变量的值
// 由于一个变量可能存在值仍未知的情况, 因此用一个属性unknown来表示
//////////////////////////////////////////////////////////////////////////
class IntegerValue;
ostream& operator << (ostream&, const IntegerValue&);

class IntegerValue
{
private:
	bool unknown;		// 值未知, 如未初始化, 或者来自动态输入
	bool guessing;		// 值是否是推测来的, 如位于循环条件中的表达式, 将为对应变量增加一个对应的推测值
						// 如while(i<=10), 为i增加一个10的值, 并设置guessing属性为真
	long value;
	unsigned int line;	// 该值出现在源程序中哪一行
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
	bool isUnknown()const		// 返回值是否未知
	{
		return unknown;
	}
	void setUnknown(bool b)		// 设置值是否未知
	{
		unknown = b;
	}

	bool isGuessing()const		// 返回值是否为猜测
	{
		return guessing;
	}
	void setGuessing(bool b)	// 设置值是否为猜测
	{
		guessing = b;
	}

	void setValue(long l)		// 设置值
	{
		value = l; unknown = false;
	}
	long getValue()const		// 获取值
	{
		return value;
	}

	unsigned int getLine()const		// 获取值出现在源文件中的行数
	{
		return line;
	}
	void setLine(unsigned int l)	// 设置值出现在源文件中的行数
	{
		line = l;
	}
	
	// 算术逻辑运算... 

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

	bool operator<(const IntegerValue& r)const;	// IntegerValue作为set的模板参数时, 必须定义该方法

};


/************************************************************************/
/* IntegerValueSet对象将一个整型变量所有的可能值封装起来                */
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
	const set<IntegerValue>& getValues()const		// 返回值集合
	{return values;}
	void setValues(const set<IntegerValue>& v)		// 设置值集合
	{values = v;}

	void setLine(unsigned int l, bool guessAlsoSet = false);	// 设置值集合中所有值出现在源文件中的行数
	void setGuessing(bool g);						// 设置值集合中所有值是否为猜测

	void addValue(const IntegerValue& v)			// 加入单个值
	{
		values.insert(v);
	}
	void addValues(const IntegerValueSet& v);		// 加入一个值集合

	bool empty()				// 值集合是否为空
	{return values.empty();}
	void clear()				// 清空集合
	{values.clear();}

	IntegerValue getMinValue();		// 返回集合中最小的值
	IntegerValue getMaxValue();		// 返回集合中最大的值

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
// 一个符号完整的表示, 如												
// class A{public:int x;};												
// A a;																	
// a.x; a.x的完整表示是{a,x}												                          
//////////////////////////////////////////////////////////////////////////
class MingledSymbolItem;
ostream& operator << (ostream&, const MingledSymbolItem&);

class MingledSymbolItem
{
private:
	// 用一个双端队列保存符号的完整表示, 如符号a.x就用该队列保存a和x两个条目
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
	
	const deque<SymbolItem*>& getSblFullName()const		// 返回符号完整表示的队列
	{return sblFullName;}
	void addSubSblItem(SymbolItem* s)					// 向符号队列中添加一项符号条目
	{sblFullName.push_back(s);}

	//
	void addSubSblItem_head(SymbolItem* s)					// 向符号队列中添加一项符号条目
	{sblFullName.push_front(s);}
	//dzh	
	void addMingleItem(MingledSymbolItem s);
	//dzn end

	bool isGlobalSymbol()const;					// 判断一项完整表示的符号是否为全局符号
												// a.b.c这项条目是否全局变量取决于a是否是全局变量	
	SymbolItem* getLastSymbol()const;			// 返回最后一项符号条目
	SymbolItem* getFirstSymbol()const;			// 返回第一项符号条目
	SymbolTable* getParentSymbolTable()const;	// 返回符号所属的作用域, a.b.c的作用域应该是a的作用域

	Type* getType()const;						// 返回符号的类型, a.b.c返回c的类型
	
	void setHeader(const MingledSymbolItem&);	// 向符号队列的头端中插入一个符号队列
	MingledSymbolItem getHeader()const;			// 返回一个符号完整表示的队列中前n-1个符号条目队列	

	bool empty()const							// 符号条目队列是否为空
	{return sblFullName.empty();}
	void clear()								// 清空符号条目队列
	{sblFullName.clear();}
	int size()const{return sblFullName.size();}

	bool isIntegerType()const;					// 符号是否是整型条目
	bool isUnsignedIntegerType()const;			// 符号是否是无符号整型条目
	bool isRefIntegerType()const;				// 符号是否是整型引用类型
	bool isReferenceType()const;				// 符号是否是引用类型
	bool isPointerIntegerType()const;			// 符号是否是整型指针类型
	bool isPointerType()const;					// 符号是否是指针类型
	bool isStatic()const;						// 符号是否被声明为静态
	bool isFloatType()const;					// 符号是否是浮点型

	bool isMemberOf(const MingledSymbolItem& parent)const;	// 符号是否是另一符号所属类的成员

	// 090512
	bool startWith(const MingledSymbolItem&)const;		// 符号条目队列的前若干项是否是参数所指定的符号队列
	void replaceStartWith(const MingledSymbolItem&);	// 符号条目队列的前若干项用参数指定的队列替换
	
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
// 挂在CFG节点上的整型变量值集合
//////////////////////////////////////////////////////////////////////////
 
class VexValueSet;
ostream& operator << (ostream&, const VexValueSet&);

class VexValueSet
{
private:
	// 利用map表保存多个整型变量和它对应的整型变量值集合,
	// 将一个用MingledSymbolItem表示的符号条目映射为一个变量值集合
	// 若该map中有一项{{a,b}, {100,20}}, 表示a.b的值可能为100或者20
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
	
	// 利用索引操作符重载, 根据一个符号条目访问该条目的整型变量值
	IntegerValueSet& operator [] (const MingledSymbolItem& sbl)
	{ return vexValues[sbl]; }

	// 从本map表中找出以srcSbl开头的符号对应的项, 插入到vs中, 并在vs中将符号头改为destSbl
	// 该方法主要用在分析函数调用时, 类或结构类型对象的参数传递
	// 如原型为f(A in)的函数的一次具体调用f(obj), 将调用点处obj中所有的整型成员的值映射到in中的整型成员, 
	// 并写入函数f的入口流值
	void copyValuesOfSblHeaderAndChangeHeader(const MingledSymbolItem& srcSbl, const MingledSymbolItem& destSbl, 
		VexValueSet& vs);
	// 从本map表中找出特定开头的符号对应的项, 复制到目标集合中
	// 该方法主要用于在一个对象上调用一个方法时, 如obj.f(), 将obj中所有的整型成员的值写入函数f的入口流值
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