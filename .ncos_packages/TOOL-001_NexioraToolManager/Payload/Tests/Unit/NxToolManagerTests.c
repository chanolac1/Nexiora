#include "Nexiora/Tools/NxToolManager.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#if defined(_WIN32)
#include <direct.h>
#define MKDIR(p) _mkdir(p)
#else
#include <unistd.h>
#define MKDIR(p) mkdir((p),0777)
#endif
static int fail(const char* m){fprintf(stderr,"NxToolManagerTests: FAIL: %s\n",m);return 1;}
int main(void){
    NxToolRecord in; NxToolRecord out; FILE* f; NxToolStatus s;
    (void)MKDIR("nx_tool_test"); (void)MKDIR("nx_tool_test/Tools"); (void)MKDIR("nx_tool_test/Tools/Managed"); (void)MKDIR("nx_tool_test/Tools/Managed/demo");
    f=fopen("nx_tool_test/Tools/Managed/demo/demo.exe","wb"); if(f==NULL)return fail("artifact create"); if(fputs("demo",f)<0||fclose(f)!=0)return fail("artifact write");
    memset(&in,0,sizeof(in));
    strcpy(in.id,"demo"); strcpy(in.version,"1.0.0"); strcpy(in.executable,"nx_tool_test/Tools/Managed/demo/demo.exe"); strcpy(in.sha256,"0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"); strcpy(in.source_url,"https://example.invalid/demo.exe");
    s=NxToolManager_Register("nx_tool_test",&in); if(s!=NX_TOOL_OK)return fail("register");
    s=NxToolManager_Verify("nx_tool_test","demo",&out); if(s!=NX_TOOL_OK)return fail("verify");
    if(strcmp(out.version,"1.0.0")!=0)return fail("version roundtrip");
    s=NxToolManager_Remove("nx_tool_test","demo"); if(s!=NX_TOOL_OK)return fail("remove");
    puts("NxToolManagerTests: PASS"); return 0;
}
