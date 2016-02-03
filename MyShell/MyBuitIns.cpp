/* TO handle all builtIn and special commands which ca't be included in general execvp executable */
#include "MyShell.h"
#include "MyUtility.h"
#include <string.h>
#include <string>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <map>
#include <vector>
#include <sys/stat.h>
#include <fcntl.h>
#include <cctype>
using namespace std;
extern char*** commandPtr; 
extern map<string,string> varsMap;
vector<string> vs;

int CheckBuiltIn(int i)
{
	if(strcmp(commandPtr[i][0],"cd") && strcmp(commandPtr[i][0],"history") && strcmp(commandPtr[i][0],"export") && strcmp(commandPtr[i][0],"echo") && commandPtr[i][0][0] != '!' && strcmp(commandPtr[i][0],"grep"))
		return 0;

	vs.clear();
	FillHistoryVector();
	if(strcmp(commandPtr[i][0],"cd")==0)
		ChangeDir(i);
	else if(strcmp(commandPtr[i][0],"history")==0)
		ShowHistory(i);
	else if(commandPtr[i][0][0]=='!')
		ExecHistoryCommand();
	else if(strcmp(commandPtr[i][0],"export")==0)
		Export(i);
	else if(strcmp(commandPtr[i][0],"echo")==0)
		Echo(i);
	else if(strcmp(commandPtr[i][0],"grep")==0)
		Grep(i);
	return 1;
		//ShowHistory(i);
		//Export(i);
		//cout << "history" << endl;
}


void ChangeDir(int i)
{
	int res = 0;
	if(commandPtr[i][1]==NULL)
	{
		char* home = getenv("HOME");
		res = chdir(home);
		return;
	}
	string dname = string(commandPtr[i][1]);
	if(dname==" " || dname=="" || dname=="~")
	{
		char* home = getenv("HOME");
		res = chdir(home);
	}
	else
		res = chdir(commandPtr[i][1]);
	if(res!=0)
		cout << strerror(errno) << endl;
}

void Grep(int i)
{
	cout.flush();
	if(commandPtr[i][1]==NULL)
	{
		cout << endl;
		return;
	}
	string par = new char[100];
	int p=2;
	char* cmp = new char[100];
	cmp=commandPtr[i][1];
	int qflag=0;
	par = string(cmp);	

	vector<string> v;
		if(NumberOfQuotes(par)%2!=0)
			qflag = 1;
	//IDIOT LOOP
	while((cmp=commandPtr[i][p]))
	{
		if(qflag==1)
		{
			par.push_back(' ');
			par = par.append(string(cmp));
			if(NumberOfQuotes(par)%2!=0)
				qflag = 1;
			else
				qflag=0;
		}
		else
		{
			par = RemoveQuotes(par);
			v.push_back(par);
			par.clear();
			par = string(cmp);	
			if(NumberOfQuotes(par)%2!=0)
				qflag = 1;
			else
				qflag = 0;
		}
		p++;
	}
	if(v.size()==0)
	{
		par = RemoveQuotes(par);
		v.push_back(par);
	}
	int j;
		char** arr = new char*[100];
		for (j = 0; j < 100; ++j)
		{
		    arr[j] = new char[100];
		}
	strcpy(arr[0],"grep");
	vector<string>::iterator it = v.begin();
	int k=1;
	while(it!=v.end())
	{
		strcpy(arr[k],((*(it)).c_str()));
		it++;
		k++;
	}
	arr[k]=NULL;
	int pid=fork();
	if(pid==0)
	{
		execvp(arr[0], arr);
		cout << commandPtr[0][0] << ": " << "Command not found" << endl;
		exit(0);
	}
	else
		wait(0);
}

void FillHistoryVector()
{

	char buf[1];
	int lcount =1;
	char c;
	int fd = open("HISTORY", O_RDWR, 0640);
	vs.push_back(string(" "));
	string s;
	while(read(fd,buf,1)!=0)
	{
		c = buf[0];
		s.push_back(c);
		if(c == '\n')
		{
			vs.push_back(s);
			lcount++;
			s.clear();
		}
	}
	close(fd);

}
void ExecHistoryCommand()
{
	int ll = HistoryLineCount();
	if(strcmp(commandPtr[0][0],"!!") == 0)
		ExecLastCommand();
	else
	{
		string arg(commandPtr[0][0]);
		arg.erase(arg.begin());
		//cout << arg << endl;


		if(isdigit(*(arg.begin())))
		{
			//EXECUTE the line in history
			int n = stringToInt((char*) arg.c_str());
			if(n>ll)
			{
				cout << "bash: " << commandPtr[0][0] << ": event not found" << endl;
				exit(0);
			}
			string cm = *(vs.begin() + n);
			if(cm.size () > 0)
			{
				cm.resize(cm.size () - 1);
				ExecuteAll(cm);
			}
		}
		else if((*(arg.begin()))=='-' && isdigit(*(arg.begin()+1)))
		{
			//EXECUTE last - n +1 line 
			arg.erase(arg.begin());
			int n = stringToInt((char*) arg.c_str());
			if(n>ll)
			{	
				cout << "bash: " << commandPtr[0][0] << ": event not found" << endl;
				exit(0);
			}
			string cm = *(vs.end() - n);
			if(cm.size () > 0)
			{
				cm.resize(cm.size () - 1);
				ExecuteAll(cm);
			}
		}
		else if((*(arg.begin())) != '\0')
		{
		int flag =1;
			//EXECUTE last command begining with string 
			vector<string>::reverse_iterator it = vs.rbegin();
			while(it != vs.rend())
			{
				if(arg.size() <= (*it).size())
				{
					if(arg == (*it).substr(0, arg.size()))
					{
						string cm = (*it);
						if(cm.size() > 0 && flag)
						{
							cm.resize(cm.size () - 1);
							ExecuteAll(cm);
							flag=0;
						}
						
					}
				}
				it++;
			}

			//after loop
			if(flag)
				cout << "bash: "<< commandPtr[0][0] <<": event not found"<< endl;
		}
		//ELSE ONLY ! given go nothing
	}
}

void ExecLastCommand()
{
	string cm = *(vs.end() -1);
	if(cm.size () > 0)
	{
		cm.resize(cm.size () - 1);
		ExecuteAll(cm);
	}
}

void ShowHistory(int i)
{
	int n,ll;
	ll = HistoryLineCount();
	if(commandPtr[i][1]==NULL)
		n = ll;
	else
	{
		string op(commandPtr[i][1]);
		n = stringToInt((char*) op.c_str());
		if(n<0)
			cout << "history: "<<op<<": invalid option" << endl;
		if(n>ll)
			n = ll;
	}
	int start = ll - n +1;

	vector<string>::iterator vit = vs.begin() + start;
	while(vit!=vs.end())
	{
		cout << " " << (start++) << "  " << (*vit);
		vit++;
	}
	
}
void Echo(int i)
{

	cout.flush();

	int dqnum=0;
	if(commandPtr[i][1]==NULL)
	{
		cout << endl;
		return;
	}
	if(commandPtr[i][1][0]=='$')
	{
		string dname = new char[100];
		dname = string(commandPtr[i][1]);
		dname.erase(dname.begin());
		map<string,string>::iterator it = varsMap.find(dname);
		if(it != varsMap.end())
			cout << it->second << endl;
		else
		{
			cout << getenv((char*) dname.c_str()) << endl;
		}
		return;
	}
	string dname = new char[100];
	int p=2;
	char* cmp = new char[100];
	cmp=commandPtr[i][1];
	dname = string(cmp);
	
	//IDIOT LOOP
	while((cmp=commandPtr[i][p]))
	{
		dname.push_back(' ');
		dname = dname.append(string(cmp));
		p++;
	}

	dqnum = NumberOfQuotes(dname);

	if(dqnum%2 != 0) // odd number of quotes
	{
		string s;
		string::iterator sit = dname.begin();
		while(sit != dname.end())
		{
			if((*sit) != '\"')
			{
				s.push_back(*sit);
			}
			sit++;
		}
		s = s.append("\n");

		string ecStr;
		getline(cin, ecStr);
		while(ecStr.find('\"',0) == string::npos)
		{	
			s.append(ecStr);
			s.push_back('\n');
			getline(cin, ecStr);
		}
		ecStr = ecStr.erase(ecStr.find_first_of('\"',0));
		s.append(ecStr);
		cout << s << endl;
	}
	else
	{
		string::iterator sit = dname.begin();
		while(sit != dname.end())
		{
			if((*sit) != '\"')
				cout << *sit;
			sit++;
		}
		cout << endl;
	}
}
int HistoryLineCount()
{
	int lcount=0;
	char buf[1];
	char c;
	int fd = open("HISTORY", O_RDWR | O_CREAT, 0640);
	while(read(fd,buf,1)!=0)
	{
		c = buf[0];
		if(c == '\n')
			lcount++;
	}
	close(fd);
	return lcount;
}
void WriteToHistory(string str)
{
	if((*(str.begin())) == '!' && str.size()>1) return;
	str = str.append("\n");
	int fd = open("HISTORY", O_RDWR | O_CREAT | O_APPEND, 0640);
	char* buff = new char[1000];
	buff = (char*) str.c_str();
	if(write(fd,buff,str.size())<0)
		cout << "History write ERROR"  << endl;
	close(fd);
}
void Export(int i)
{
	if(commandPtr[i][1]==NULL)
	{
		ExecuteAll("env");
		if(!varsMap.empty())
			PrintVarMap();
		return;
	}
	string dname = string(commandPtr[i][1]);
	int p=2;
	while(commandPtr[i][p] != NULL)
	{
		dname.push_back(' ');
		dname = dname.append(string(commandPtr[i][p]));
		p++;
	}
	if(dname.find('=',0)!=string::npos)
		ExecuteAll(dname);
}
void PrintVarMap()
{
	map<string,string>::iterator it = varsMap.begin();
	while(it != varsMap.end())
	{
		cout << it->first << "=\"" << it->second << "\"" << endl;
		it++;
	}
}
