//////////////////////////////////////////////////////////////////////////
//	Author: Day															//		
//																		//
//////////////////////////////////////////////////////////////////////////


#include "ValueStructure.hpp"
#include "Assistant.h"


//////////////////////////////////////////////////////////////////////////
// IntegerValue的所有方法的定义
// 一个IntegerValue对象封装一个整型值, 用来保存一个整型变量的值
// 由于一个变量可能存在值仍未知的情况, 因此用一个属性unknown来表示 
//////////////////////////////////////////////////////////////////////////
bool IntegerValue::isLessThan (const IntegerValue& r)
{
	if(false==r.unknown && false==unknown)
		return value<r.getValue();
	else if(r.guessing&&guessing)
		return value<r.getValue();
	else if(r.guessing&&!guessing)
		return 1;
	else if(!r.guessing&&guessing)
		return 0;
	return 0;

}
bool IntegerValue::operator<(const IntegerValue& r)const	// IntegerValue作为set的模板参数时, 必须定义该方法
{
	if(!unknown && !r.unknown)
		return value<r.value;
	if(unknown && r.unknown) return false;
	if(unknown) return true;
	if(r.unknown) return false;
	return false; // by 20100719
} 

IntegerValue IntegerValue::operator+( const IntegerValue& r)const
{
	if(unknown || r.unknown)return IntegerValue();
	return IntegerValue(value+r.value, guessing || r.guessing);
}

IntegerValue IntegerValue::operator-( const IntegerValue& r)const
{
	if(unknown || r.unknown)return IntegerValue();
	return IntegerValue(value-r.value, guessing || r.guessing);
}

IntegerValue IntegerValue::operator*( const IntegerValue& r)const
{
	if(unknown || r.unknown)return IntegerValue();
	return IntegerValue(value*r.value, guessing || r.guessing);
}
	
IntegerValue IntegerValue::operator/( const IntegerValue& r)const
{
	if(unknown || r.unknown)return IntegerValue();
	if(r.value == 0) return IntegerValue();
	return IntegerValue(value/r.value, guessing || r.guessing);
}

IntegerValue IntegerValue::operator%( const IntegerValue& r)const
{
	if(unknown || r.unknown)return IntegerValue();
	if(r.value==0)return IntegerValue();
	return IntegerValue(value%r.value, guessing || r.guessing);
}

IntegerValue IntegerValue::operator<<( const IntegerValue& r)const
{
	if(unknown || r.unknown)return IntegerValue();
	return IntegerValue(value<<r.value, guessing || r.guessing);
}
	
IntegerValue IntegerValue::operator>>( const IntegerValue& r)const
{
	if(unknown || r.unknown)return IntegerValue();
	return IntegerValue(value>>r.value, guessing || r.guessing);
}

IntegerValue IntegerValue::operator&( const IntegerValue& r)const
{
	if(unknown || r.unknown)return IntegerValue();
	return IntegerValue(value&r.value, guessing || r.guessing);
}

IntegerValue IntegerValue::operator|( const IntegerValue& r)const
{
	if(unknown || r.unknown)return IntegerValue();
	return IntegerValue(value|r.value, guessing || r.guessing);
}
	
IntegerValue IntegerValue::operator^( const IntegerValue& r)const
{
	if(unknown || r.unknown)return IntegerValue();
	return IntegerValue(value^r.value, guessing || r.guessing);
}

IntegerValue IntegerValue::operator~()const
{
	if(unknown) return IntegerValue();
	return IntegerValue(~value, guessing);
}

IntegerValue& IntegerValue::operator+=( const IntegerValue& r)
{
	if(unknown || r.unknown)
	{
		unknown=true; return *this;
	}
	if(!guessing)
	{	
		value += r.value;
		guessing = r.guessing;
	}
	return *this;
}

IntegerValue& IntegerValue::operator-=( const IntegerValue& r)
{
	if(unknown || r.unknown)
	{
		unknown=true; return *this;
	}
	if(!guessing)
	{	
		value -= r.value;
		guessing = r.guessing;
	}
	return *this;
}
	
IntegerValue& IntegerValue::operator*=( const IntegerValue& r)
{
	if(unknown || r.unknown)
	{
		unknown=true; return *this;
	}
	if(!guessing)
	{	
		value *= r.value;
		guessing = r.guessing;
	}
	return *this;
}

IntegerValue& IntegerValue::operator/=( const IntegerValue& r)
{
	if(unknown || r.unknown)
	{
		unknown=true; return *this;
	}
	if(r.value==0){ unknown = true; return *this;} 
	if(!guessing)
	{	
		value /= r.value;
		guessing = r.guessing;
	}
	return *this;
}

IntegerValue& IntegerValue::operator%=( const IntegerValue& r)
{
	if(unknown || r.unknown)
	{
		unknown=true; return *this;
	}
	if(r.value==0){ unknown = true; return *this;}
	if(!guessing)
	{	
		value %= r.value;
		guessing = r.guessing;
	}
	return *this;
}
	
IntegerValue& IntegerValue::operator<<=( const IntegerValue& r)
{
	if(unknown || r.unknown)
	{
		unknown=true; return *this;
	}
	if(!guessing)
	{	
		value <<= r.value;
		guessing = r.guessing;
	}
	return *this;
}
	
IntegerValue& IntegerValue::operator>>=( const IntegerValue& r)
{
	if(unknown || r.unknown)
	{
		unknown=true; return *this;
	}
	if(!guessing)
	{	
		value >>= r.value;
		guessing = r.guessing;
	}
	return *this;
}
	
IntegerValue& IntegerValue::operator &=(const IntegerValue& r)
{
	if(unknown || r.unknown)
	{
		unknown=true; return *this;
	}
	if(!guessing)
	{	
		value &= r.value;
		guessing = r.guessing;
	}
	return *this;
}
	
IntegerValue& IntegerValue::operator |=(const IntegerValue& r)
{
	if(unknown || r.unknown)
	{
		unknown=true; return*this;
	}
	if(!guessing)
	{	
		value |= r.value;
		guessing = r.guessing;
	}
	return *this;
}
	
IntegerValue& IntegerValue::operator ^=(const IntegerValue& r)
{
	if(unknown || r.unknown)
	{
		unknown=true; return *this;
	}
	if(!guessing)
	{	
		value ^= r.value;
		guessing = r.guessing;
	}
	return *this;
}

IntegerValue IntegerValue::operator++()
{
	if(unknown) return IntegerValue();
	if(!guessing) ++value;
	return IntegerValue(*this);
}

IntegerValue IntegerValue::operator++(int)
{
	if(unknown) return IntegerValue();
	IntegerValue r(*this);
	if(!guessing)++value;
	return r;
}

IntegerValue IntegerValue::operator--()
{
	if(unknown) return IntegerValue();
	if(!guessing)--value;
	return IntegerValue(*this);
}

IntegerValue IntegerValue::operator--(int)
{
	if(unknown) return IntegerValue();
	IntegerValue r(*this);
	if(!guessing)--value;
	return r;
}

IntegerValue IntegerValue::operator-()const
{
	if(unknown)return IntegerValue();
	return IntegerValue(-value, guessing);
}

IntegerValue IntegerValue::operator && (const IntegerValue& r)
{
	if(unknown || r.unknown || guessing || r.guessing)
		return IntegerValue();
	return IntegerValue((long)(value && r.value));
}

IntegerValue IntegerValue::operator || (const IntegerValue& r)
{
	if(unknown || r.unknown || guessing || r.guessing)
		return IntegerValue();
	return IntegerValue((long)(value || r.value));
}

IntegerValue IntegerValue::operator ! ()
{
	if(unknown || guessing )
		return IntegerValue();
	return IntegerValue((long)(!value));
}

IntegerValue IntegerValue::operator <= (const IntegerValue& r)
{
	if(unknown || r.unknown || guessing || r.guessing)
		return IntegerValue();
	return IntegerValue((long)(value<=r.value));
}
//ft add
bool IntegerValue::operator < (const IntegerValue& r)
{
	if(false==r.unknown && false==unknown)
		return r.getValue()<value;
	else if(r.guessing&&guessing)
		return r.getValue()<value;
	else if(r.guessing&&!guessing)
		return 1;
	else if(!r.guessing&&guessing)
		return 0;
	return 0;

}
//ft add end
IntegerValue IntegerValue::operator >= (const IntegerValue& r)
{
	if(unknown || r.unknown || guessing || r.guessing)
		return IntegerValue();
	return IntegerValue((long)(value>=r.value));
}

IntegerValue IntegerValue::operator > (const IntegerValue& r)
{
	if(unknown || r.unknown || guessing || r.guessing)
		return IntegerValue();
	return IntegerValue((long)(value>r.value));
}

IntegerValue IntegerValue::operator == (const IntegerValue& r)
{
	if(unknown || r.unknown || guessing || r.guessing)
		return false;
	return IntegerValue((long)(value==r.value));
}

IntegerValue IntegerValue::operator != (const IntegerValue& r)
{
	if(unknown || r.unknown || guessing || r.guessing)
		return IntegerValue();
	return IntegerValue((long)(value!=r.value));
}

ostream& operator << (ostream& o, const IntegerValue& v)
{
	if(v.isUnknown())
	{
		o<<"unknown";
	}
	else 
	{
		o<<v.getValue();
		if(v.guessing)o<<" guess";
	}
	o<<" at line "<<v.getLine();
	return o;
}


//////////////////////////////////////////////////////////////////////////
// IntegerValueSet的成员方法定义
//////////////////////////////////////////////////////////////////////////

void IntegerValueSet::addValues(const IntegerValueSet& v)
{
	set<IntegerValue>::const_iterator it;  const set<IntegerValue>& vs = v.values;
	for(it = vs.begin(); it!=vs.end(); it++)
	{
	//	values.insert(*it);
		addValue(*it);
	}
}
//ft modify
/*void IntegerValueSet::setLine(unsigned int l, bool guessAlsoSet)
{
	set<IntegerValue>::iterator it;
	for(it=values.begin(); it!=values.end(); it++)
	{
		if((*it).isGuessing())
		{
			if(guessAlsoSet) (*it).setLine(l);
		}
		else
			(*it).setLine(l);
	}
}*/
void IntegerValueSet::setLine(unsigned int l, bool guessAlsoSet)
{
	set<IntegerValue>::iterator it;
	IntegerValue temp;
	for(it=values.begin(); it!=values.end();)
	{
		if((*it).isGuessing())
		{
			if(guessAlsoSet) 
			{
				//(*it).setLine(l);
				temp=*it;
				temp.setLine(l);
				values.erase(it++);
				values.insert(temp);
			}else{
				it++;
			}
		}
		else{
			temp=*it;
			temp.setLine(l);
			values.erase(it++);
			values.insert(temp);
		}
	}
}
//ft modify
/*void IntegerValueSet::setGuessing(bool g)
{
	set<IntegerValue>::iterator it;
	for(it=values.begin(); it!=values.end(); it++)
		(*it).setGuessing(g);
}	
}*/
void IntegerValueSet::setGuessing(bool g)
{
	set<IntegerValue>::iterator it;
	IntegerValue temp;
	for(it=values.begin(); it!=values.end();)
	{
		temp=*it;
		temp.setGuessing(g);
		values.erase(it++);
		values.insert(temp);
	}
}
//ft modify end
void IntegerValueSet::unionValues(const IntegerValueSet& r)
{
	set<IntegerValue> vs = r.values;
	set<IntegerValue>::iterator it;
	for(it = vs.begin(); it != vs.end(); it++)
	{
		values.insert(*it);
	}		
}
IntegerValueSet IntegerValueSet::operator + (const IntegerValueSet& r)const
{
	set<IntegerValue>::const_iterator it1, it2;
	const set<IntegerValue>& rValues = r.getValues();
	set<IntegerValue> vs;
	for(it1 = values.begin(); it1 !=  values.end(); it1++)
	{
		for(it2 = rValues.begin(); it2 != rValues.end(); it2++)
		{
			IntegerValue v1 = *it1, v2 = *it2;
			vs.insert(v1+v2);	
		}
	}
	return IntegerValueSet(vs);
}
bool IntegerValueSet::findGuessingOrUnknow()const
{
	if(values.empty())
		return true;
	set<IntegerValue>::const_iterator it=values.begin();
	while(it!=values.end())
	{
		if(it->isGuessing()||it->isUnknown())
		{
			return true;
		}
		it++;
	}
	return false;
}
IntegerValueSet IntegerValueSet::operator - (const IntegerValueSet& r)const
{
	set<IntegerValue>::const_iterator it1, it2;
	const set<IntegerValue>& rValues = r.getValues();
	set<IntegerValue> vs;
	for(it1 = values.begin(); it1 !=  values.end(); it1++)
	{
		for(it2 = rValues.begin(); it2 != rValues.end(); it2++)
		{
			IntegerValue v1 = *it1, v2 = *it2;
			vs.insert(v1-v2);
		}
	}
	return IntegerValueSet(vs);
}
	
IntegerValueSet IntegerValueSet::operator * (const IntegerValueSet& r)const
{
	set<IntegerValue>::const_iterator it1, it2;
	const set<IntegerValue>& rValues = r.getValues();
	set<IntegerValue> vs;
	for(it1 = values.begin(); it1 !=  values.end(); it1++)
	{
		for(it2 = rValues.begin(); it2 != rValues.end(); it2++)
		{
			IntegerValue v1 = *it1, v2 = *it2;
			vs.insert(v1*v2);
		}
	}
	return IntegerValueSet(vs);
}
	
IntegerValueSet IntegerValueSet::operator / (const IntegerValueSet& r)const
{
	set<IntegerValue>::const_iterator it1, it2;
	const set<IntegerValue>& rValues = r.getValues();
	set<IntegerValue> vs;
	for(it1 = values.begin(); it1 !=  values.end(); it1++)
	{
		for(it2 = rValues.begin(); it2 != rValues.end(); it2++)
		{
			IntegerValue v1 = *it1, v2 = *it2;
			vs.insert(v1/v2);
		}
	}
	return IntegerValueSet(vs);
}
	
IntegerValueSet IntegerValueSet::operator % (const IntegerValueSet& r)const
{
	set<IntegerValue>::const_iterator it1, it2;
	const set<IntegerValue>& rValues = r.getValues();
	set<IntegerValue> vs;
	for(it1 = values.begin(); it1 !=  values.end(); it1++)
	{
		for(it2 = rValues.begin(); it2 != rValues.end(); it2++)
		{
			IntegerValue v1 = *it1, v2 = *it2;
			vs.insert(v1%v2);
		}
	}
	return IntegerValueSet(vs);
}

	
IntegerValueSet IntegerValueSet::operator & (const IntegerValueSet& r)const
{
	set<IntegerValue> vs;
	/*
	//2010-11-6,ZLY,BEGIN,handle & 0xff etc
	IntegerValue vs1_oneValue = getMinValue();
	set<IntegerValue>& vs2_values = r.getValues();
	IntegerValue vs2_oneValue = r.getMinValue();
	if(values.size() == 1 && 
		vs1_oneValue.isUnknown() == false && 
		vs1_oneValue.isGuessing() == false)
	{
	}else if(vs2_values.size() == 1 && 
		vs2_oneValue.isUnknown() == false && 
		vs2_oneValue.isGuessing() == false)
	{
	}
	//2010-11-6,ZLY,END,handle & 0xff etc
	*/

	set<IntegerValue>::const_iterator it1, it2;
	const set<IntegerValue>& rValues = r.getValues();
	for(it1 = values.begin(); it1 !=  values.end(); it1++)
	{
		for(it2 = rValues.begin(); it2 != rValues.end(); it2++)
		{
			IntegerValue v1 = *it1, v2 = *it2;
			vs.insert(v1&v2);
		}
	}
	return IntegerValueSet(vs);
}
	
IntegerValueSet IntegerValueSet::operator | (const IntegerValueSet& r)const
{
	set<IntegerValue>::const_iterator it1, it2;
	const set<IntegerValue>& rValues = r.getValues();
	set<IntegerValue> vs;
	for(it1 = values.begin(); it1 !=  values.end(); it1++)
	{
		for(it2 = rValues.begin(); it2 != rValues.end(); it2++)
		{
			IntegerValue v1 = *it1, v2 = *it2;
			vs.insert(v1|v2);
		}
	}
	return IntegerValueSet(vs);
}
	
IntegerValueSet IntegerValueSet::operator ^ (const IntegerValueSet& r)const
{
	set<IntegerValue>::const_iterator it1, it2;
	const set<IntegerValue>& rValues = r.getValues();
	set<IntegerValue> vs;
	for(it1 = values.begin(); it1 !=  values.end(); it1++)
	{
		for(it2 = rValues.begin(); it2 != rValues.end(); it2++)
		{
			IntegerValue v1 = *it1, v2 = *it2;
			vs.insert(v1^v2);	
		}
	}
	return IntegerValueSet(vs);
}
	
IntegerValueSet IntegerValueSet::operator ~ ()const
{
	set<IntegerValue>::const_iterator it1;
	set<IntegerValue> vs;
	for(it1 = values.begin(); it1 !=  values.end(); it1++)
	{
			IntegerValue v = *it1;
			vs.insert(~v);
	}
	return IntegerValueSet(vs);
}

	
IntegerValueSet IntegerValueSet::operator << (const IntegerValueSet& r)const
{
	set<IntegerValue>::const_iterator it1, it2;
	const set<IntegerValue>& rValues = r.getValues();
	set<IntegerValue> vs;
	for(it1 = values.begin(); it1 !=  values.end(); it1++)
	{
		for(it2 = rValues.begin(); it2 != rValues.end(); it2++)
		{
			IntegerValue v1 = *it1, v2 = *it2;
			vs.insert(v1<<v2);
		}
	}
	return IntegerValueSet(vs);
}
	
IntegerValueSet IntegerValueSet::operator >> (const IntegerValueSet& r)const
{
	set<IntegerValue>::const_iterator it1, it2;
	const set<IntegerValue>& rValues = r.getValues();
	set<IntegerValue> vs;
	for(it1 = values.begin(); it1 !=  values.end(); it1++)
	{
		for(it2 = rValues.begin(); it2 != rValues.end(); it2++)
		{
			IntegerValue v1 = *it1, v2 = *it2;
			vs.insert(v1>>v2);
		}
	}
	return IntegerValueSet(vs);
}
	
IntegerValueSet& IntegerValueSet::operator += (const IntegerValueSet& r)
{
	set<IntegerValue>::const_iterator it1, it2;
	const set<IntegerValue>& rValues = r.getValues();
	set<IntegerValue> vs;
	for(it1 = values.begin(); it1 !=  values.end(); it1++)
	{
		for(it2 = rValues.begin(); it2 != rValues.end(); it2++)
		{
			if((*it1).isGuessing())
			{	vs.insert(*it1);	break;	}
			vs.insert(*it1 + *it2);			
		}
	}
	values.clear();
	values = vs;	
	return *this;
}
	
IntegerValueSet& IntegerValueSet::operator -= (const IntegerValueSet& r)
{
	set<IntegerValue>::const_iterator it1, it2;
	const set<IntegerValue>& rValues = r.getValues();
	set<IntegerValue> vs;
	for(it1 = values.begin(); it1 !=  values.end(); it1++)
	{
		for(it2 = rValues.begin(); it2 != rValues.end(); it2++)
		{
			if((*it1).isGuessing())
			{	vs.insert(*it1);	break;	}
			vs.insert(*it1 - *it2);			
		}
	}
	values.clear();
	values = vs;	
	return *this;
}

IntegerValueSet& IntegerValueSet::operator *= (const IntegerValueSet& r)
{
	set<IntegerValue>::const_iterator it1, it2;
	const set<IntegerValue>& rValues = r.getValues();
	set<IntegerValue> vs;
	for(it1 = values.begin(); it1 !=  values.end(); it1++)
	{
		for(it2 = rValues.begin(); it2 != rValues.end(); it2++)
		{
			if((*it1).isGuessing())
			{	vs.insert(*it1);	break;	}
			vs.insert(*it1 * *it2);			
		}
	}
	values.clear();
	values = vs;	
	return *this;
}

IntegerValueSet& IntegerValueSet::operator /= (const IntegerValueSet& r)
{
	set<IntegerValue>::const_iterator it1, it2;
	const set<IntegerValue>& rValues = r.getValues();
	set<IntegerValue> vs;
	for(it1 = values.begin(); it1 !=  values.end(); it1++)
	{
		for(it2 = rValues.begin(); it2 != rValues.end(); it2++)
		{
			if((*it1).isGuessing())
			{	vs.insert(*it1);	break;	}
			vs.insert(*it1 / *it2);			
		}
	}
	values.clear();
	values = vs;	
	return *this;
}
	
IntegerValueSet& IntegerValueSet::operator %= (const IntegerValueSet& r)
{
	set<IntegerValue>::const_iterator it1, it2;
	const set<IntegerValue>& rValues = r.getValues();
	set<IntegerValue> vs;
	for(it1 = values.begin(); it1 !=  values.end(); it1++)
	{
		for(it2 = rValues.begin(); it2 != rValues.end(); it2++)
		{
			if((*it1).isGuessing())
			{	vs.insert(*it1);	break;	}
			vs.insert(*it1 % *it2);			
		}
	}
	values.clear();
	values = vs;	
	return *this;
}

IntegerValueSet& IntegerValueSet::operator <<= (const IntegerValueSet& r)
{
	set<IntegerValue>::const_iterator it1, it2;
	const set<IntegerValue>& rValues = r.getValues();
	set<IntegerValue> vs;
	for(it1 = values.begin(); it1 !=  values.end(); it1++)
	{
		for(it2 = rValues.begin(); it2 != rValues.end(); it2++)
		{
			if((*it1).isGuessing())
			{	vs.insert(*it1);	break;	}
			vs.insert(*it1 << *it2);			
		}
	}
	values.clear();
	values = vs;	
	return *this;
}
	
IntegerValueSet& IntegerValueSet::operator >>= (const IntegerValueSet& r)
{
	set<IntegerValue>::const_iterator it1, it2;
	const set<IntegerValue>& rValues = r.getValues();
	set<IntegerValue> vs;
	for(it1 = values.begin(); it1 !=  values.end(); it1++)
	{
		for(it2 = rValues.begin(); it2 != rValues.end(); it2++)
		{
			if((*it1).isGuessing())
			{	vs.insert(*it1);	break;	}
			vs.insert(*it1 >> *it2);			
		}
	}
	values.clear();
	values = vs;	
	return *this;
}

IntegerValueSet& IntegerValueSet::operator &= (const IntegerValueSet& r)
{
	set<IntegerValue>::const_iterator it1, it2;
	const set<IntegerValue>& rValues = r.getValues();
	set<IntegerValue> vs;
	for(it1 = values.begin(); it1 !=  values.end(); it1++)
	{
		for(it2 = rValues.begin(); it2 != rValues.end(); it2++)
		{
			if((*it1).isGuessing())
			{	vs.insert(*it1);	break;	}
			vs.insert(*it1 & *it2);			
		}
	}
	values.clear();
	values = vs;	
	return *this;
}

IntegerValueSet& IntegerValueSet::operator |= (const IntegerValueSet& r)
{
	set<IntegerValue>::const_iterator it1, it2;
	const set<IntegerValue>& rValues = r.getValues();
	set<IntegerValue> vs;
	for(it1 = values.begin(); it1 !=  values.end(); it1++)
	{
		for(it2 = rValues.begin(); it2 != rValues.end(); it2++)
		{
			if((*it1).isGuessing())
			{	vs.insert(*it1);	break;	}
			vs.insert(*it1 | *it2);			
		}
	}
	values.clear();
	values = vs;	
	return *this;
}

IntegerValueSet& IntegerValueSet::operator ^= (const IntegerValueSet& r)
{
	set<IntegerValue>::const_iterator it1, it2;
	const set<IntegerValue>& rValues = r.getValues();
	set<IntegerValue> vs;
	for(it1 = values.begin(); it1 !=  values.end(); it1++)
	{
		for(it2 = rValues.begin(); it2 != rValues.end(); it2++)
		{
			if((*it1).isGuessing())
			{	vs.insert(*it1);	break;	}
			vs.insert(*it1 ^ *it2);			
		}
	}
	values.clear();
	values = vs;	
	return *this;
}
	
IntegerValueSet IntegerValueSet::operator ++ ()
{
	return *this += IntegerValueSet(IntegerValue(1));
}
	
IntegerValueSet IntegerValueSet::operator ++ (int)
{
	IntegerValueSet r(*this);
	//////////(*this)++;
    ++(*this);
	return r;
}
	
IntegerValueSet IntegerValueSet::operator -- ()
{
	return *this -= IntegerValueSet(IntegerValue(1));
}
	
IntegerValueSet IntegerValueSet::operator -- (int)
{
	IntegerValueSet r(*this);
	////(*this)--;
    --(*this);
	return r;
}
	
IntegerValueSet IntegerValueSet::operator - ()const
{
	set<IntegerValue>::const_iterator it;
	set<IntegerValue> vs;
	for(it = values.begin(); it != values.end(); it++)
	{
		IntegerValue v(-(*it));
		vs.insert(v);
	}
	return IntegerValueSet(vs);
}

IntegerValueSet IntegerValueSet::operator && (const IntegerValueSet& r)
{
	set<IntegerValue>::const_iterator it1, it2;
	const set<IntegerValue>& rValues = r.getValues();
	set<IntegerValue> vs;
	for(it1 = values.begin(); it1 !=  values.end(); it1++)
	{
		for(it2 = rValues.begin(); it2 != rValues.end(); it2++)
		{
			IntegerValue v1 = *it1, v2 = *it2;
			vs.insert(v1&&v2);
		}
	}
	return IntegerValueSet(vs);
}
IntegerValueSet IntegerValueSet::operator || (const IntegerValueSet& r)
{
	set<IntegerValue>::const_iterator it1, it2;
	const set<IntegerValue>& rValues = r.getValues();
	set<IntegerValue> vs;
	for(it1 = values.begin(); it1 !=  values.end(); it1++)
	{
		for(it2 = rValues.begin(); it2 != rValues.end(); it2++)
		{
			IntegerValue v1 = *it1, v2 = *it2;
			vs.insert(v1||v2);
		}
	}
	return IntegerValueSet(vs);
}

IntegerValueSet IntegerValueSet::operator !  ()
{
	set<IntegerValue>::iterator it;
	set<IntegerValue> vs;
	for(it=values.begin(); it!=values.end(); it++)
	{
		IntegerValue v = *it;
		vs.insert(!v);
	}
	return IntegerValueSet(vs);
}

IntegerValueSet IntegerValueSet::operator <  (const IntegerValueSet& r)
{
	set<IntegerValue>::const_iterator it1, it2;
	const set<IntegerValue>& rValues = r.getValues();
	set<IntegerValue> vs;
	for(it1 = values.begin(); it1 !=  values.end(); it1++)
	{
		for(it2 = rValues.begin(); it2 != rValues.end(); it2++)
		{
			IntegerValue v1 = *it1, v2 = *it2;
			if(v1.isUnknown() || v2.isUnknown() || v1.isGuessing() || v2.isGuessing())
				vs.insert(IntegerValue());
			else vs.insert(IntegerValue((long)(v1.getValue()<v2.getValue())));
		}
	}
	return IntegerValueSet(vs);
}

IntegerValueSet IntegerValueSet::operator <= (const IntegerValueSet& r) 
{
	set<IntegerValue>::const_iterator it1, it2;
	const set<IntegerValue>& rValues = r.getValues();
	set<IntegerValue> vs;
	for(it1 = values.begin(); it1 !=  values.end(); it1++)
	{
		for(it2 = rValues.begin(); it2 != rValues.end(); it2++)
		{
			IntegerValue v1 = *it1, v2 = *it2;
			vs.insert(v1<=v2);
		}
	}
	return IntegerValueSet(vs);
}
IntegerValueSet IntegerValueSet::operator >  (const IntegerValueSet& r)
{
	set<IntegerValue>::const_iterator it1, it2;
	const set<IntegerValue>& rValues = r.getValues();
	set<IntegerValue> vs;
	for(it1 = values.begin(); it1 !=  values.end(); it1++)
	{
		for(it2 = rValues.begin(); it2 != rValues.end(); it2++)
		{
			IntegerValue v1 = *it1, v2 = *it2;
			vs.insert(v1>v2);
		}
	}
	return IntegerValueSet(vs);
}

IntegerValueSet IntegerValueSet::operator >= (const IntegerValueSet& r)
{
	set<IntegerValue>::const_iterator it1, it2;
	const set<IntegerValue>& rValues = r.getValues();
	set<IntegerValue> vs;
	for(it1 = values.begin(); it1 !=  values.end(); it1++)
	{
		for(it2 = rValues.begin(); it2 != rValues.end(); it2++)
		{
			IntegerValue v1 = *it1, v2 = *it2;
			vs.insert(v1>=v2);
		}
	}
	return IntegerValueSet(vs);
}

IntegerValueSet IntegerValueSet::operator == (const IntegerValueSet& r)
{
	set<IntegerValue>::const_iterator it1, it2;
	const set<IntegerValue>& rValues = r.getValues();
	set<IntegerValue> vs;
	for(it1 = values.begin(); it1 !=  values.end(); it1++)
	{
		for(it2 = rValues.begin(); it2 != rValues.end(); it2++)
		{
			IntegerValue v1 = *it1, v2 = *it2;
			vs.insert(v1==v2);
		}
	}
	return IntegerValueSet(vs);
}

IntegerValueSet IntegerValueSet::operator != (const IntegerValueSet& r)
{
	set<IntegerValue>::const_iterator it1, it2;
	const set<IntegerValue>& rValues = r.getValues();
	set<IntegerValue> vs;
	for(it1 = values.begin(); it1 !=  values.end(); it1++)
	{
		for(it2 = rValues.begin(); it2 != rValues.end(); it2++)
		{
			IntegerValue v1 = *it1, v2 = *it2;
			vs.insert(v1!=v2);
		}
	}
	return IntegerValueSet(vs);
}

ostream& operator << (ostream& o, const IntegerValueSet& vs)
{
	const set<IntegerValue>& values = vs.getValues();
	set<IntegerValue>::const_iterator it;

	o<<'{';
	for(it = values.begin(); it != values.end(); )
	{
		o << *it;
		if(++it != values.end()) o<<",";
	}
	o<<'}';
	return o;
}

IntegerValue IntegerValueSet::getMinValue()
{
	if(values.empty())return IntegerValue();
	return *(values.begin());
}
IntegerValue IntegerValueSet::getMaxValue()
{
	if(values.empty())return IntegerValue();
	const IntegerValue& v = *(values.begin());
	if(v.isUnknown()) return v;
	return *(values.rbegin());
}



map<int, SymbolItem*> heapObjs;
bool isHeapObjs(SymbolItem* sbl)
{
	if(!sbl) return false;
	int id = sbl->getSymbolID();
	return heapObjs.find(id)!=heapObjs.end();
}

//////////////////////////////////////////////////////////////////////////
// MingledSymbolItem 的方法定义
//////////////////////////////////////////////////////////////////////////

ostream& operator<< (ostream& o, const MingledSymbolItem& sbl)
{
	deque<SymbolItem*> sbls = sbl.getSblFullName();
	deque<SymbolItem*>::iterator it;
	o<<'(';
	for(it=sbls.begin(); it!=sbls.end(); )
	{
		if((*it) != NULL)
			o<<(*it)->getName(); 
		else
			o<<"NULL";
		if(++it != sbls.end()) o<<"::";
	}
	o<<')';
	return o;
}


//dzh add

void MingledSymbolItem::addMingleItem(MingledSymbolItem m)
{
	deque<SymbolItem*> tmp = m.sblFullName;
	int i = 0, sz = tmp.size();
	while(i<sz)
		sblFullName.push_back(tmp[i++]);
}


//begin 表示的是队列中第begin个，真正的序列是begin---->end-1
MingledSymbolItem MingledSymbolItem::SubSequence(int begin, int end)
{
	deque<SymbolItem*>  tmp;
	while(begin < end)
	{
		tmp.push_back(*(sblFullName.begin()+begin-1));
		++begin;
	}
	return MingledSymbolItem(tmp);
	
}
//dzh end
extern ofstream smTrace;
#define SM_TRACE

bool MingledSymbolItem::operator==(const MingledSymbolItem& r) const
{
	//2010-08-08,ZLY,Compare all the symbol item
	if(sblFullName.size() != r.sblFullName.size()){
#ifdef SM_TRACE
		smTrace<<"size not same!"<<sblFullName.size()<<","<<r.sblFullName.size()<<endl;
#endif
		return false;
	}
	deque<SymbolItem *>::const_iterator itMy;
	deque<SymbolItem*>::const_iterator itR;
	itMy=sblFullName.begin();
	itR=r.sblFullName.begin();
	for(; itMy != sblFullName.end(); itMy++, itR++){
		if((*itMy) != (*itR)){
#ifdef SM_TRACE
			smTrace<<"symbol not same! '"<<(*itMy)->getName()<<"', '"<<(*itR)->getName()<<"'"<<endl;
#endif
			return false;
		}
	}
	return true;
	//return (sblFullName == r.sblFullName);
	//2010-08-08,ZLY,END
}

bool MingledSymbolItem::isGlobalSymbol()const
{
	if(sblFullName.empty())return false;
	if(isHeapObjs(sblFullName[0])) return true;
	extern SymRoot* root;
	ScopeSymbolTable* glbTbl = root->getGlobalScope();
	list<SymbolItem*> glbSbls = glbTbl->getVariableList();
	SymbolItem* sbl = *(sblFullName.begin());
	list<SymbolItem*>::iterator it;
	for(it=glbSbls.begin(); it!=glbSbls.end(); it++)
		if(*it == sbl)return true;
		return false;
}

SymbolItem* MingledSymbolItem::getLastSymbol()const
{
	if(sblFullName.empty())return 0;
	return *(sblFullName.rbegin());
}

SymbolItem* MingledSymbolItem::getFirstSymbol()const
{
	if(sblFullName.empty()) return 0;
	return *(sblFullName.begin());
}

SymbolTable* MingledSymbolItem::getParentSymbolTable()const
{
	if(sblFullName.empty())return 0;
	extern SymRoot* root;
	if(isHeapObjs(sblFullName[0])) return root->getGlobalScope();
	SymbolItem* sbl = *(sblFullName.begin());
	return sbl->getParentSymTable();
}

MingledSymbolItem MingledSymbolItem::getHeader()const
{
	deque<SymbolItem*> s;
	int i = 0, sz = sblFullName.size()-1;
	while(i<sz)
	{	
		s.push_back(sblFullName[i++]);
	}
	return MingledSymbolItem(s);
}

void MingledSymbolItem::setHeader(const MingledSymbolItem& m)
{
	deque<SymbolItem*> tmp = m.sblFullName;
	int i = 0, sz = sblFullName.size();
	while(i<sz)
		tmp.push_back(sblFullName[i++]);
	sblFullName.clear();
	sblFullName = tmp;
}

bool MingledSymbolItem::startWith(const MingledSymbolItem& start)const
{
	if(sblFullName.size()<start.sblFullName.size())
		return false;
	int i = 0;
	while(i<start.sblFullName.size())
	{
		if(sblFullName[i] != start.sblFullName[i]) return false;
		i++;
	}
	return true;
}

Type* MingledSymbolItem::getType()const
{
	if(sblFullName.empty()) return 0;
	return (*(sblFullName.rbegin()))->getTypeItem();
}

void MingledSymbolItem::replaceStartWith(const MingledSymbolItem& nStart)
{
	int i = 0, sz = nStart.sblFullName.size();
	while(i<sz)
	{
		sblFullName[i] = nStart.sblFullName[i];
		i++;
	}
}

bool MingledSymbolItem::isIntegerType()const
{
	if(sblFullName.empty()) return false;
//	if(isHeapObjs(*(sblFullName.rbegin()))) return true;
	SymbolItem* s = *(sblFullName.rbegin());
	
	Type* type = s->getTypeItem();
	if(!type) return false;  // 20100719
	ItemKind kind = type ->getItemKind();
	if(kind == TYPE_SHORT || kind == TYPE_UNSHORT || kind == TYPE_INT || kind == TYPE_UNINT
		|| kind == TYPE_LONG || kind == TYPE_UNLONG || kind == TYPE_LONGLONG || kind == TYPE_ULONGLONG )
		return true;
	return false;
}
bool MingledSymbolItem::isUnsignedIntegerType()const
{
	if(sblFullName.empty()) return false;
	//	if(isHeapObjs(*(sblFullName.rbegin()))) return true;
	SymbolItem* s = *(sblFullName.rbegin());

	Type* type = s->getTypeItem();
	if(!type) return false;  // 20100719
	ItemKind kind = type ->getItemKind();
	if( kind == TYPE_UNSHORT || kind == TYPE_UNINT||  kind == TYPE_UNLONG || TYPE_ULONGLONG )
		return true;
	return false;
}
bool MingledSymbolItem::isFloatType()const
{
	if(sblFullName.empty()) return false;
	SymbolItem* s = *(sblFullName.rbegin());

	Type* type = s->getTypeItem();
	if(!type) return false;  // 20100719
	ItemKind kind = type ->getItemKind();
	if(kind == TYPE_FLOAT || kind == TYPE_DOUBLE || kind == TYPE_LONGDOUBLE)
		return true;
	return false;
}
bool MingledSymbolItem::isRefIntegerType()const
{
	if(sblFullName.empty() ) return false;
	SymbolItem* s = *(sblFullName.rbegin());
	Type* type = s->getTypeItem();
	if(!type)	return false;			//20100719
	ItemKind kind = type->getItemKind();
	if(kind == TYPE_Reference)
		kind = ((PointerReferArray*)type)->getElementType()->getItemKind();
	if(kind == TYPE_SHORT || kind == TYPE_UNSHORT || kind == TYPE_INT || kind == TYPE_UNINT
		|| kind == TYPE_LONG || kind == TYPE_UNLONG )
		return true;
	return false;
}

bool MingledSymbolItem::isReferenceType()const
{
	if(sblFullName.empty() || isHeapObjs(*(sblFullName.rbegin())))
		return false;
	SymbolItem* s = *(sblFullName.rbegin());
	Type* type = s->getTypeItem();
	ItemKind kind = type->getItemKind();
	if(kind == TYPE_Reference) return true;
	return false;
}

bool MingledSymbolItem::isPointerIntegerType()const
{
	if(sblFullName.empty() || isHeapObjs(*(sblFullName.rbegin())))
		return false;
	SymbolItem* s = *(sblFullName.rbegin());
	Type* type = s->getTypeItem();
	ItemKind kind = type->getItemKind();
	if(kind == TYPE_Reference)
	{
		type = ((PointerReferArray*)type)->getElementType();
		kind = type->getItemKind();
	}

	if(kind == TYPE_POINTER)
		kind = ((PointerReferArray*)type)->getElementType()->getItemKind();
	if(kind == TYPE_SHORT || kind == TYPE_UNSHORT || kind == TYPE_INT || kind == TYPE_UNINT
		|| kind == TYPE_LONG || kind == TYPE_UNLONG )
		return true;
	return false;
}

bool MingledSymbolItem::isPointerType()const
{
	if(sblFullName.empty() || isHeapObjs(*(sblFullName.rbegin())))return false;
	SymbolItem* s = *(sblFullName.rbegin());
	Type* type = s->getTypeItem();
	ItemKind kind = type->getItemKind();
	if(kind == TYPE_Reference)
	{
		type = ((PointerReferArray*)type)->getElementType();
		kind = type->getItemKind();
	}
	if(kind == TYPE_POINTER) return true;
	return false;
}

bool MingledSymbolItem::isMemberOf(const MingledSymbolItem& prt)const
{
	if(sblFullName.empty() || prt.empty()) return false;
	if(isHeapObjs(sblFullName[0])) return false;
	SymbolItem* chdSbl = *(sblFullName.begin());
	SymbolItem* prtSbl = prt.getLastSymbol();
	Type* type = prtSbl->getTypeItem();
	ItemKind kind = type->getItemKind();
	if(kind == TYPE_Reference || kind == TYPE_POINTER)
	{
		type = (PointerReferArray*)type->getElementType();
		kind = type->getItemKind();
	}
	return isMemberOfType(chdSbl,type);
}

bool MingledSymbolItem::isStatic() const
{
	for(int i = 0; i<sblFullName.size(); i++)
	{
		SymbolItem* sbl = sblFullName[i];
		if(sbl != NULL && !isHeapObjs(sbl))
		{
			if(NULL != sbl->getDeclSpecifier())
			{
				if(sbl->getDeclSpecifier()->isSTATIC) 
					return true;
			}
		}
	}
	return false;
}

bool MingledSymbolItem::hasRefElement()const
{
	int i = 0, sz = sblFullName.size();
	while(i<sz)
	{
		SymbolItem* sbl = sblFullName[i];
		if(sbl != NULL && !isHeapObjs(sbl))
		{
			if(sbl->getTypeItem() != NULL)
			{
				ItemKind kind = sbl->getTypeItem()->getItemKind();
				if(kind == TYPE_Reference) return true;
			}
		}
		i++;
	}
	return false;
}

bool MingledSymbolItem::hasRefOrPtrElement()const
{
	int i = 0, sz = sblFullName.size();
	while(i<sz)
	{
		SymbolItem* sbl = sblFullName[i++];
		if(sbl != NULL &&!isHeapObjs(sbl))
		{
			if(sbl->getTypeItem() != NULL)
			{
				ItemKind kind = sbl->getTypeItem()->getItemKind();
				if(kind==TYPE_Reference || kind==TYPE_POINTER) return true;
			}
			
		}
	}
	return false;
}

SymbolItem* MingledSymbolItem::getLastPointerSbl()const
{
	int i = sblFullName.size()-1;
	while(i>=0)
	{
		SymbolItem* sbl = sblFullName[i];
		Type* type = sbl->getTypeItem();
		if(type != NULL)
		{
			ItemKind kind = type->getItemKind();
			if(kind==TYPE_Reference )
			{
				type = ((PointerReferArray*)type)->getElementType();
				if(type != NULL)
					kind = type->getItemKind();
			}
			if(kind == TYPE_POINTER) return sbl;
		}
		i--;
	}
	return 0;
}

MingledSymbolItem MingledSymbolItem::getStaticParts()const
{
	deque<SymbolItem*> sbls;
	int i = 0;
	for(; i<sblFullName.size(); i++)
	{
		SymbolItem* sbl = sblFullName[i];
		if(sbl !=NULL && !isHeapObjs(sbl))
		{
			if(sbl->getDeclSpecifier()->isSTATIC) break;
		}
	}
	for(; i<sblFullName.size(); i++)
		sbls.push_back(sblFullName[i]);
	return MingledSymbolItem(sbls);
}


//////////////////////////////////////////////////////////////////////////
// VexValueSet的方法定义
//////////////////////////////////////////////////////////////////////////
VexValueSet VexValueSet::operator + (const VexValueSet& r)const
{
	const map<MingledSymbolItem, IntegerValueSet>& rVexValues = r.getVexValues();
	map<MingledSymbolItem, IntegerValueSet> result = vexValues;
	map<MingledSymbolItem, IntegerValueSet>::const_iterator it;
	for(it = rVexValues.begin(); it != rVexValues.end(); it++)
	{
		MingledSymbolItem sbl = it->first;
		const IntegerValueSet& values = it->second;
		result[sbl].unionValues(values);
	}
	return VexValueSet(result);
}

VexValueSet& VexValueSet::operator += (const VexValueSet& r)
{
	return *this = *this + r;
}

ostream& operator << (ostream& o, const VexValueSet& vexValueSet)
{
	const map<MingledSymbolItem, IntegerValueSet>& vexValues = vexValueSet.getVexValues();
	if(vexValues.size() <= 0)
		return o;
	map<MingledSymbolItem, IntegerValueSet>::const_iterator it;
	for(it = vexValues.begin(); it != vexValues.end(); it++)
	{
		o << it->first;
		o << ":" << it->second<<"  ";
	}
	return o;
}

void VexValueSet::copyValuesOfSblHeaderAndChangeHeader(
		const MingledSymbolItem& srcSbl, 
		const MingledSymbolItem& destSbl, 
		VexValueSet& vs)
{
	if(srcSbl.empty()) return;
	map<MingledSymbolItem, IntegerValueSet>::iterator it;
	for(it = vexValues.begin(); it!=vexValues.end(); it++)
	{
		MingledSymbolItem sbl = it->first;
		if(sbl.startWith(srcSbl))
		{
			sbl.replaceStartWith(destSbl);
			vs[sbl] = it->second;
		}
	}
}

void VexValueSet::copyValuesOfSblHeader(const MingledSymbolItem& sblHeader, VexValueSet& vs)
{
	if(sblHeader.empty()) return;
	map<MingledSymbolItem, IntegerValueSet>::iterator it;
	for(it = vexValues.begin(); it!=vexValues.end(); it++)
	{
		const MingledSymbolItem& sbl = it->first;
		if(sbl.startWith(sblHeader))
		{
			vs[sbl] = it->second;
		}
	}
}
