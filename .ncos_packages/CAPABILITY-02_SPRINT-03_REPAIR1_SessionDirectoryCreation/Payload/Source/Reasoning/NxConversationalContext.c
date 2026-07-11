#include "Nexiora/Reasoning/NxConversationalContext.h"
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

static int nx_copy(char* d,size_t n,const char* s){size_t l;if(!d||!s||n==0)return 0;l=strlen(s);if(l>=n)return 0;memcpy(d,s,l+1);return 1;}
static void nx_trim(char* s){size_t i=0,l;if(!s)return;while(s[i]&&isspace((unsigned char)s[i]))i++;if(i)memmove(s,s+i,strlen(s+i)+1);l=strlen(s);while(l&&isspace((unsigned char)s[l-1]))s[--l]='\0';}
static int nx_has_reference(const char* q){return strstr(q,"¿y ")||strstr(q,"y como")||strstr(q,"y cómo")||strstr(q,"se diferencia")||strstr(q,"eso")||strstr(q,"esta ")||strstr(q,"este ");}
static int nx_read_field(FILE* f,const char* key,char* out,size_t cap){char line[4096];size_t k=strlen(key);int found=0;rewind(f);while(fgets(line,sizeof(line),f)){if(strncmp(line,key,k)==0&&line[k]=='='){if(!nx_copy(out,cap,line+k+1))return 0;nx_trim(out);found=1;}}return found;}
static int nx_make_directory(const char* path){int rc;
#ifdef _WIN32
    rc=_mkdir(path);
#else
    rc=mkdir(path,0777);
#endif
    return rc==0||errno==EEXIST;
}
static int nx_ensure_parent_directories(const char* file_path){char path[NX_CC_MAX_PATH];size_t i,start=0;if(!file_path||!*file_path||!nx_copy(path,sizeof(path),file_path))return 0;if(strlen(path)>=3U&&isalpha((unsigned char)path[0])&&path[1]==':')start=3U;for(i=start;path[i];++i){if(path[i]=='/'||path[i]=='\\'){char saved=path[i];if(i==0U)continue;path[i]='\0';if(path[0]&&strcmp(path,".")!=0&&!nx_make_directory(path)){path[i]=saved;return 0;}path[i]=saved;}}return 1;}
NxCcStatus NxConversationalContext_Create(const char* p,const char* e,const char* s){FILE* f;if(!p||!e||!s||!*p||!*e)return NX_CC_INVALID_ARGUMENT;if(!nx_ensure_parent_directories(p))return NX_CC_IO_ERROR;f=fopen(p,"wb");if(!f)return NX_CC_IO_ERROR;if(fprintf(f,"nxconversation/1\nevidence=%s\nsubject=%s\nturns=0\n",e,s)<0){fclose(f);return NX_CC_IO_ERROR;}if(fclose(f)!=0)return NX_CC_IO_ERROR;return NX_CC_OK;}
NxCcStatus NxConversationalContext_Ask(const char* p,const char* q,NxCcTurnResult* o){FILE* f;char e[NX_CC_MAX_PATH]={0},s[NX_CC_MAX_TEXT]={0},t[64]={0};unsigned int turns=0;NxGroundedReasoningStatus rs;if(!p||!q||!o||!*q)return NX_CC_INVALID_ARGUMENT;memset(o,0,sizeof(*o));f=fopen(p,"rb");if(!f)return NX_CC_IO_ERROR;if(!nx_read_field(f,"evidence",e,sizeof(e))||!nx_read_field(f,"subject",s,sizeof(s))){fclose(f);return NX_CC_IO_ERROR;}if(nx_read_field(f,"turns",t,sizeof(t)))sscanf(t,"%u",&turns);fclose(f);if(nx_has_reference(q)&&s[0]){int w=snprintf(o->resolved_question,sizeof(o->resolved_question),"%s Contexto activo: %s",q,s);if(w<0||(size_t)w>=sizeof(o->resolved_question))return NX_CC_INVALID_ARGUMENT;}else if(!nx_copy(o->resolved_question,sizeof(o->resolved_question),q))return NX_CC_INVALID_ARGUMENT;nx_copy(o->active_subject,sizeof(o->active_subject),s);rs=NxGroundedReasoning_Ask(e,o->resolved_question,&o->grounded);if(rs!=NX_GR_OK&&rs!=NX_GR_INSUFFICIENT_EVIDENCE){o->status=NX_CC_REASONING_ERROR;return o->status;}o->turn_index=turns+1;o->status=NX_CC_OK;f=fopen(p,"ab");if(!f)return NX_CC_IO_ERROR;if(fprintf(f,"turn.%u.question=%s\nturn.%u.resolved=%s\nturn.%u.status=%s\nturn.%u.confidence=%u\nturns=%u\n",o->turn_index,q,o->turn_index,o->resolved_question,o->turn_index,NxGroundedReasoning_StatusName(rs),o->turn_index,o->grounded.confidence,o->turn_index)<0){fclose(f);return NX_CC_IO_ERROR;}if(fclose(f)!=0)return NX_CC_IO_ERROR;return NX_CC_OK;}
const char* NxConversationalContext_StatusName(NxCcStatus s){switch(s){case NX_CC_OK:return "OK";case NX_CC_INVALID_ARGUMENT:return "INVALID_ARGUMENT";case NX_CC_IO_ERROR:return "IO_ERROR";case NX_CC_REASONING_ERROR:return "REASONING_ERROR";default:return "UNKNOWN";}}
