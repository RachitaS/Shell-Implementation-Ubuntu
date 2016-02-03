#include "MyShell.h"
#include "MyUtility.h"
#include <cstdio>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <vector>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include<map>

using namespace std;
vector<char*> cmdVector; /* Vector used for temporary holding intermediate commands to fill command array */
char*** commandPtr; /* Array used for calling execvp */
map<string,string> varsMap; /* Map used for storing and displaying local variables */
int pp = 0; /* Indicates number of pipes is in the command */
void MakeMyShell(string MyShellName)
{
	/* To prevent program termination on cntrl+c */
	signal(SIGINT, ExitSigHandler);
	while(1)
	{
		unsigned int i,j;
		i =0 ;
		while(i<cmdVector.size())
			cmdVector[i++]=NULL;
		commandPtr = new char**[100];
		for (i = 0; i < 100; ++i)
		{
		    commandPtr[i] = new char*[100];
		    for (j = 0; j < 100; ++j)
		    {
			 *(commandPtr[i]+j) = new char[100];
		    }
		}
		cout << MyShellName << ":" << getcwd(NULL,1000) << "$ ";
		string fullcmd;
		getline(cin, fullcmd);
		
		if(fullcmd == "exit")
		{
			cout << "Bye.." << endl;
		 	exit(0);
		}
		
		if(fullcmd.size()>0)
		{
			/* Fill the history file to add this command */
			WriteToHistory(fullcmd);
			/* Excute the command by breaking it into separate parts */
			ExecuteAll(fullcmd);
		}
		delete[] commandPtr;
	}
}


void ExitSigHandler(int s) 
{
	signal(SIGINT, ExitSigHandler);
	cout << endl;
	cout << "My_Shell" << ":" << getcwd(NULL,1000) << "$ ";
	cout.flush();
}


void ExecuteAll(string fullcmd)
{
	if(fullcmd.find('=',0) != string::npos && fullcmd.find("export",0) == string::npos)
	{
		varsMap[fullcmd.substr(0,fullcmd.find('='))] = fullcmd.substr(fullcmd.find('=')+1,string::npos);
		return;
	}
	int fd=-1, stdincpy = dup(0), stdoutcpy = dup(1);
	string cmd;
	if(fullcmd.find('>') != string::npos)
	{
		cmd = fullcmd.substr(0, fullcmd.find('>'));
		string outfilename = fullcmd.substr(fullcmd.find('>')+1, string::npos);	
		outfilename =  TrimExtra(outfilename);
		if(outfilename.find(' ') != string::npos && NumberOfQuotes(outfilename)%2 !=0)
		{
			cout << "Invalid file name" << endl;
			return;
		}
		outfilename = RemoveQuotes(outfilename);
		//change output fd
		fd = open(outfilename.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0640);
		//write(fd,"HEY",4);
		dup2(fd, 1);
		ExecuteAll(cmd);
		if(fd!=-1)
		{
			close(fd);
			dup2(stdincpy,0);
			dup2(stdoutcpy,1);
		}
		return;
	}
	else
	{
		cmd = string(fullcmd);
	}
	FillCommand(cmd);

	if(pp==0)
	{
		if(cmd.find('<') != string::npos)
		{	
			string filename = cmd.substr(cmd.find('<')+1, string::npos);
			if(filename.find(' ') != string::npos && NumberOfQuotes(filename)%2 !=0)
			{
				cout << "Invalid file name" << endl;
				return;
			}
			filename = RemoveQuotes(filename);
			filename =  TrimExtra(filename);
			fd = open(filename.c_str(), O_RDWR | O_CREAT, 0640);
			dup2(fd, 0);
			cmd = cmd.substr(0, cmd.find('<'));
			ExecuteAll(cmd);
			if(fd!=-1)
			{
				close(fd);
				dup2(stdincpy,0);
				dup2(stdoutcpy,1);
			}
			return;
		}

		if(!CheckBuiltIn(0))
		{
			int id = fork();
			if(id<0)
				cout << "FORK ERROR" << endl;
			else if(id==0)
			{
				execvp(commandPtr[0][0], commandPtr[0]);
				cout << commandPtr[0][0] << ": " << "Command not found" << endl;
				exit(0);
			}
			else
			{
				wait(0);
			}
		}
	}
	else
	{
		int r =0;
		if(cmd.find('<') != string::npos)
		{
			r=1;
		}
		ExecuteWithPipes(r);
		pp=0;
	}
	if(fd!=-1)
	{
		close(fd);
		dup2(stdincpy,0);
		dup2(stdoutcpy,1);
	}
}
string GetCommand(int i)
{
	if(commandPtr[i][1]==NULL)
	{
		cout << endl;
		return NULL;
	}
	string dname = new char[100];
	int p=1;
	char* cmp = NULL;
	cmp=commandPtr[i][0];
	dname = dname.append(string(cmp));
	
	//IDIOT LOOP
	while((cmp=commandPtr[i][p]))
	{
		dname.push_back(' ');
		dname = dname.append(string(cmp));
		p++;
	}
	return dname;

}

/* Execution of command containing pipes */
void ExecuteWithPipes(int r)
{
	int fd=-1, stdincpy = dup(0);
	int pfd1[2];
	pipe(pfd1);
	pid_t pid;
	pid = fork();
		
	if (pid==0) 
	{
		/* Making write end of the pipe work same as stdout and closing stdout */
		dup2(pfd1[1],1);
		/* closing read end of pipe as we don't need it now */
		close(pfd1[0]);
		/* Executing first command using stdin as input and sending output to pipe */
		if(!CheckBuiltIn(0))
		{
			if(r==1)
			{
				string cmd = GetCommand(0);
				if(cmd.find('<') != string::npos)
				{	
					string filename = cmd.substr(cmd.find('<')+1, string::npos);
					if(filename.find(' ') != string::npos && NumberOfQuotes(filename)%2 !=0)
					{
						cout << "Invalid file name" << endl;
						//return;
						exit(0);
					}
					filename = RemoveQuotes(filename);
					filename =  TrimExtra(filename);
					fd = open(filename.c_str(), O_RDWR | O_CREAT, 0640);
					dup2(fd, 0);
					cmd = cmd.substr(0, cmd.find('<'));
					ExecuteAll(cmd);
					if(fd!=-1)
					{
						close(fd);
						dup2(stdincpy,0);
					}
					exit(0);
				}


			}

			execvp(commandPtr[0][0], commandPtr[0]);
			printf("Command not found\n");
			//return;
			exit(0);
		}
		else
			exit(0);
	}
	else if(pid>0)
	{
		wait(0);
	}
	else
		printf("fork ERROR\n");
	if(pp>2)
	{
		int i;
		for(i=1;i<pp-1;i++)
		{
			int pfd2[2];
			pipe(pfd2);
			pid = fork();
			
			if (pid==0) 
			{
				/* Making read end of pipe work same as stdin and close stdin */
				dup2(pfd1[0],0);  
				/* Closing write end of pipe as we don't need it now */
				close(pfd1[1]);

				/* Making write end of pipe work same as stdout and close stdout */
				dup2(pfd2[1],1);
				/* Closing read end of the pipe as we don't need it now*/
				close(pfd2[0]); 

				/* Executing current command using the pipe data as input */
				if(!CheckBuiltIn(i))
				{
					if(r==1)
					{
						string cmd = GetCommand(i);
						if(cmd.find('<') != string::npos)
						{	
							string filename = cmd.substr(cmd.find('<')+1, string::npos);
							if(filename.find(' ') != string::npos && NumberOfQuotes(filename)%2 !=0)
							{
								cout << "Invalid file name" << endl;
							//	return;
								exit(0);
							}
							filename = RemoveQuotes(filename);
							filename =  TrimExtra(filename);
							fd = open(filename.c_str(), O_RDWR | O_CREAT, 0640);
							dup2(fd, 0);
							cmd = cmd.substr(0, cmd.find('<'));
							ExecuteAll(cmd);
							if(fd!=-1)
							{
								close(fd);
								dup2(stdincpy,0);
							}
							exit(0);
						}


					}
					execvp(commandPtr[i][0],commandPtr[i]);

					/* This will be executed only when execvp fails */
					printf("Command not found\n");
					//return;
					exit(0);
				}
				else
					exit(0);
			}
			else if(pid<0)
				printf("fork ERROR\n");

			/* Closing the pipe we are done with */
			close(pfd1[0]);
			close(pfd1[1]);
			wait(0);
			/* Assigning current pipe descripters to earlier variables to be recognized in the loop */
			pfd1[0]=pfd2[0];
			pfd1[1]=pfd2[1];
		}
	}
	pid = fork();	
	if (pid==0) 
	{
		/* Making read end of the pipe work same as stdin and closing stdin */
		dup2(pfd1[0],0);
		/* Closing write end of pipe as we don't need it now */
		close(pfd1[1]);
		/* Executing last command using the pipe data as input */
		if(!CheckBuiltIn(pp-1))
		{
			if(r==1)
			{
				string cmd = GetCommand(pp-1);
				if(cmd.find('<') != string::npos)
				{	
					string filename = cmd.substr(cmd.find('<')+1, string::npos);
					if(filename.find(' ') != string::npos && NumberOfQuotes(filename)%2 !=0)
					{
						cout << "Invalid file name" << endl;
						//return;
						exit(0);
					}
					filename = RemoveQuotes(filename);
					filename =  TrimExtra(filename);
					fd = open(filename.c_str(), O_RDWR | O_CREAT, 0640);
					dup2(fd, 0);
					cmd = cmd.substr(0, cmd.find('<'));
					ExecuteAll(cmd);
					if(fd!=-1)
					{
						close(fd);
						dup2(stdincpy,0);
					}
					exit(0);
				}


			}
			execvp(commandPtr[pp-1][0],commandPtr[pp-1]);
			printf("Command not found\n");
			//return;
			exit(0);
		}
		else
			exit(0);
	}
	else if(pid<0)
		printf("fork ERROR\n");
	/* Closing the pipe ends as we are done with it */
	close(pfd1[0]);
	close(pfd1[1]);
	wait(0);
	return;
}

/* fill the global array as per the format needed by exevp */
void FillCommand(string cmd)
{
	if(cmd.find('|') == string::npos)
	{
		unsigned int k=0;
		while(k<cmdVector.size())
			cmdVector[k++]=NULL;
		cmdVector.clear();
		char* cm = (char*) cmd.c_str();
		cm = strtok(cm," ");
		while(cm != NULL)
		{
			cmdVector.push_back(cm);
			cm = strtok(NULL," ");
		}
		cmdVector.push_back(NULL);
		commandPtr[0] = &cmdVector[0];
	}
	else
	{
		char* str = (char*) cmd.c_str();
		int i,j;
		for (i = 0; i < 100; ++i)
		{
		    commandPtr[i] = (char**) malloc(sizeof(char*) * 100);
		    for (j = 0; j < 100; ++j)
			*(commandPtr[i]) = (char*) malloc(100);
		}
		char* res = (char*) malloc(1000);
		char** part= (char**) malloc(sizeof(char*));;
		res = strtok(str,"|");
		pp=0;
		while(res != NULL)
		{
			*(part+pp)= (char*) malloc(1000);
			part[pp++]=res;
			res = strtok(NULL,"|");
		}
		for(i=0; i<pp; i++)
		{
			res = strtok(part[i]," ");
			int j=0;
			while(res != NULL)
			{
				*(commandPtr[i]+j)=(char*) malloc(100);
				commandPtr[i][j++]=res;
				res = strtok(NULL," ");
			}
			commandPtr[i][j] = NULL;
		}
		commandPtr[pp][0] = NULL;
	}
}


