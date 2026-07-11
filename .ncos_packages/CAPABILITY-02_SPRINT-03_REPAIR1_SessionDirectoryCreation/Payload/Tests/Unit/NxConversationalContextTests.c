#include "Nexiora/Reasoning/NxConversationalContext.h"
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif
static int check(int c,const char*m){if(!c){fprintf(stderr,"NxConversationalContextTests: FAIL: %s\n",m);return 0;}return 1;}
static void cleanup(void){remove("nx_cc_nested/session/test.nxconversation");remove("nx_cc_test.nxevidence");
#ifdef _WIN32
_rmdir("nx_cc_nested/session");_rmdir("nx_cc_nested");
#else
rmdir("nx_cc_nested/session");rmdir("nx_cc_nested");
#endif
}
int main(void){const char* ev="nx_cc_test.nxevidence";const char* ss="nx_cc_nested/session/test.nxconversation";FILE*f;NxCcTurnResult r;cleanup();f=fopen(ev,"wb");if(!f)return 1;fputs("nxevidence/1\nchunk=1\nsource=demo@1\ntext=La memoria episodica conserva experiencias personales con tiempo y contexto.\nchunk=2\nsource=demo@2\ntext=La memoria semantica conserva conocimientos generales sin contexto autobiografico.\n",f);fclose(f);if(!check(NxConversationalContext_Create(ss,ev,"memoria episodica")==NX_CC_OK,"nested session create failed"))return 1;f=fopen(ss,"rb");if(!check(f!=NULL,"session file was not created"))return 1;fclose(f);if(!check(NxConversationalContext_Ask(ss,"Que conserva la memoria episodica",&r)==NX_CC_OK,"first ask failed"))return 1;if(!check(r.grounded.evidence_count>0,"first answer lacks evidence"))return 1;if(!check(NxConversationalContext_Ask(ss,"Y como se diferencia de la semantica",&r)==NX_CC_OK,"follow-up failed"))return 1;if(!check(strstr(r.resolved_question,"memoria episodica")!=NULL,"reference was not resolved"))return 1;if(!check(r.turn_index==2U,"turn index was not persisted"))return 1;cleanup();puts("NxConversationalContextTests: PASS");return 0;}
