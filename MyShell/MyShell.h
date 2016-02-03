#include <string>
using namespace std;
void MakeMyShell(string MyShellName);
void ExecuteAll(string fullcmd);
void FillCommand(string cmd);
void ChangeDir(int i);
void ExecuteWithPipes(int i);
int HistoryLineCount();
int CheckBuiltIn(int i);
void ShowHistory(int i);
void Echo(int i);
void Grep(int i);
void FillHistoryVector();
void ExecLastCommand();
void ExecHistoryCommand();
void ExitSigHandler(int s);
void Export(int i);
void PrintVarMap();
