#include "Nexiora/NCOS/NxPatchVerificationGate.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef _WIN32
#include <direct.h>
#define MKDIR(p) _mkdir(p)
#else
#include <sys/stat.h>
#define MKDIR(p) mkdir(p,0777)
#endif
static int fails=0; static void check(int c,const char*m){ if(!c){printf("FAIL: %s\n",m); fails++;}}
static void mkdirs(const char* p){ char tmp[512]; snprintf(tmp,sizeof(tmp),"%s",p); for(char*s=tmp;*s;++s){ if(*s=='/'||*s=='\\'){ char o=*s; *s='\0'; if(strlen(tmp)>0)MKDIR(tmp); *s=o; } } MKDIR(tmp); }
static void write_file(const char* p,const char* t){ FILE*f=fopen(p,"wb"); if(f){fputs(t,f); fclose(f);} }
int main(void){
    const char* root="Build/ncos015_test_sandbox";
    system("if exist Build\\ncos015_test_sandbox rmdir /s /q Build\\ncos015_test_sandbox >nul 2>nul");
    mkdirs("Build/ncos015_test_sandbox/Knowledge/NCOS/PatchApprovals/demo");
    mkdirs("Build/ncos015_test_sandbox/Knowledge/NCOS/PatchApplications/demo");
    write_file("Build/ncos015_test_sandbox/Knowledge/NCOS/PatchApprovals/demo/approval.md","Status: APPROVED\nApproved by: test\n");
    write_file("Build/ncos015_test_sandbox/Knowledge/NCOS/PatchApplications/demo/application.md","Application record\nTests: PASSED\nValidation: PASSED\n");
    NxPatchVerificationResult r;
    check(NxPatchVerificationGate_Verify(root,"demo",&r),"verify should succeed");
    check(strcmp(r.status,"PROMOTABLE")==0,"status should be promotable");
    check(r.approval_found,"approval should be found");
    check(r.application_found,"application should be found");
    check(r.tests_passed,"tests should be detected");
    check(r.promotable,"candidate should be promotable");
    check(strstr(r.verification_report,"verification.md")!=0,"report path should be exposed");
    NxPatchVerificationResult s;
    check(NxPatchVerificationGate_Status(root,"demo",&s),"status should load");
    check(strcmp(s.status,"PROMOTABLE")==0,"loaded status should be promotable");
    return fails?1:0;
}
