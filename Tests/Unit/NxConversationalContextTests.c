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
int main(void){const char* ev="nx_cc_test.nxevidence";const char* ss="nx_cc_nested/session/test.nxconversation";FILE*f;NxCcTurnResult r;char first_answer[NX_GR_MAX_ANSWER];cleanup();f=fopen(ev,"wb");if(!f)return 1;fputs("nxevidence/1\nchunk=1\nsource=demo@1\ntext=La memoria episodica conserva experiencias personales asociadas con tiempo, lugar y contexto autobiografico.\nchunk=2\nsource=demo@2\ntext=La memoria semantica conserva conocimientos generales, hechos y conceptos sin contexto autobiografico.\nchunk=3\nsource=demo@3\ntext=La memoria episodica y la memoria semantica son sistemas declarativos distintos: una recuerda eventos vividos y la otra conocimiento general.\n",f);fclose(f);if(!check(NxConversationalContext_Create(ss,ev,"memoria episodica")==NX_CC_OK,"nested session create failed"))return 1;f=fopen(ss,"rb");if(!check(f!=NULL,"session file was not created"))return 1;fclose(f);if(!check(NxConversationalContext_Ask(ss,"Que conserva la memoria episodica",&r)==NX_CC_OK,"first ask failed"))return 1;if(!check(r.grounded.evidence_count>0,"first answer lacks evidence"))return 1;if(!check(strstr(r.grounded.answer,"experiencias personales")!=NULL,"first answer selected unrelated evidence"))return 1;if(!check(strlen(r.grounded.answer)<sizeof(first_answer),"first answer too long"))return 1;strcpy(first_answer,r.grounded.answer);if(!check(NxConversationalContext_Ask(ss,"Y como se diferencia de la semantica",&r)==NX_CC_OK,"follow-up failed"))return 1;if(!check(strstr(r.resolved_question,"Compara memoria episodica con memoria semantica")!=NULL,"comparison context was not resolved"))return 1;if(!check(strstr(r.grounded.answer,"memoria semantica")!=NULL,"follow-up did not select semantic-memory evidence"))return 1;if(!check(strcmp(first_answer,r.grounded.answer)!=0,"follow-up repeated the previous answer"))return 1;if(!check(r.grounded.evidence_count>=2U,"comparison requires evidence from both concepts"))return 1;if(!check(r.turn_index==2U,"turn index was not persisted"))return 1;cleanup();puts("NxConversationalContextTests: PASS");return 0;}
