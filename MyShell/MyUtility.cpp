

/* this file contains defenation of all functions which are general purpose */
/* Function names are self explainatory */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <stdio.h>
#include "MyUtility.h"
using namespace std;
string RemoveQuotes(string dname)
{
	string::iterator sit = dname.begin();
	string res;
	while(sit != dname.end())
	{
		if((*sit) != '\"')
			res.push_back(*sit);
		sit++;
	}
	return res;
}
int NumberOfQuotes(string dname)
{
	int dqnum =0 ;
	unsigned int pos=0;
	while(dname.find_first_of("\"",pos) != string::npos)
	{
		dqnum++;
		pos = dname.find_first_of("\"",pos) + 1;
	}
	return dqnum;
}
int stringToInt(char* l)
{
	int i=0, j=0, a=0, val=0;
	for(i=0; l[i]!='\0'; i++) {}
	for(j=i-1; j>-1; j--) 
	{
		int v = 1;
		for(a=0; a<(i-j-1); a++)
		{
			v = v*10;
		}
		val = val + (l[j]-'0')*v;
		
	}
	return val;
}
char* IntToString(int n,char* buf)
{
	buf = new char[1000];
	if(n/10==0)
	{
		buf[0]= '0' + n;
		buf[1] = '\0';
		return buf;
	}
	int p,k=0,j=0;
	char tmp[1000];
	while(n!=0)
	{
		p=n%10;
		tmp[k++] = (char) p + '0';
		n=n/10;
	}
	while(k)
	{
		buf[j++] = tmp[k-1];
		k--;
	}
	buf[j]='\0';
	return buf;
}
string TrimExtra(string s)
{

	int k = s.size();
	while(s[0] == ' ' || s[0] == '\t' || s[0] == '\0')
	{
		s.erase(s.begin());
	}
	while(s[k-1] == ' ' || s[k-1] == '\t')
	{
		s.erase(s.end()-1);
		k = s.size();
	}
	return s;
}
