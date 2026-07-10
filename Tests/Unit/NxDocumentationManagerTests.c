#include "Nexiora/NCOS/NxDocumentationManager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#if defined(_WIN32)
#include <direct.h>
#define NX_MKDIR(path) _mkdir(path)
#else
#define NX_MKDIR(path) mkdir((path), 0777)
#endif

static int failures = 0;
static void expect(int condition, const char* message) { if (!condition) { printf("FAIL: %s\n", message); failures++; } }
static void make_dir(const char* path) { (void)NX_MKDIR(path); }
static void write_text(const char* path, const char* text) { FILE* f = fopen(path, "wb"); if (f != NULL) { (void)fputs(text, f); (void)fclose(f); } }
static int contains(const char* path, const char* token) { FILE* f=fopen(path,"rb"); char line[1024]; if(f==NULL)return 0; while(fgets(line,(int)sizeof(line),f)!=NULL){if(strstr(line,token)!=NULL){(void)fclose(f);return 1;}} (void)fclose(f); return 0; }

int main(void)
{
    const char* root = "Build/docs_manager_test";
    const char* names[] = {"MASTER_CONTEXT.md","PROJECT_STATE.md","ROADMAP.md","CHANGELOG.md","DECISIONS.md","ARCHITECTURE.md","CODING_STANDARD.md","TESTING_STANDARD.md","PACKAGE_STANDARD.md"};
    char path[512];
    size_t i;
    NxDocumentationValidationResult result;
    NxDocumentationRelease release;
    char message[512];
    make_dir("Build"); make_dir(root); (void)snprintf(path,sizeof(path),"%s/Docs",root); make_dir(path);
    (void)snprintf(path,sizeof(path),"%s/Include",root); make_dir(path); (void)snprintf(path,sizeof(path),"%s/Include/Nexiora",root); make_dir(path); (void)snprintf(path,sizeof(path),"%s/Include/Nexiora/NCOS",root); make_dir(path);
    (void)snprintf(path,sizeof(path),"%s/Source",root); make_dir(path); (void)snprintf(path,sizeof(path),"%s/Source/NCOS",root); make_dir(path);
    (void)snprintf(path,sizeof(path),"%s/Tools",root); make_dir(path); (void)snprintf(path,sizeof(path),"%s/Tools/NCOS",root); make_dir(path);
    (void)snprintf(path,sizeof(path),"%s/Tests",root); make_dir(path); (void)snprintf(path,sizeof(path),"%s/Tests/Unit",root); make_dir(path);
    for(i=0U;i<sizeof(names)/sizeof(names[0]);++i){const char* content="# DOCUMENT\n"; (void)snprintf(path,sizeof(path),"%s/Docs/%s",root,names[i]); if(strcmp(names[i],"PROJECT_STATE.md")==0)content="# PROJECT STATE\nSprint actual:\nSprint 20\nEstado:\nEn desarrollo\n"; else if(strcmp(names[i],"CHANGELOG.md")==0)content="# CHANGELOG\n"; else if(strcmp(names[i],"DECISIONS.md")==0)content="# DECISIONS\n"; write_text(path,content);}
    expect(NxDocumentationManager_Validate(root,&result)==1,"base document set validates");
    write_text("Build/docs_manager_release.nxdoc","sprint_id=Sprint 20\nsprint_name=Self Validation Engine\ncompletion_date=2026-07-10\nnext_sprint_id=Sprint 21\nnext_sprint_name=Patch Ranking Engine\ncapabilities=Validación autónoma y documentación sincronizada.\ndecision_id=ADR-0002\ndecision_title=Documentation Manager transaccional\ndecision_body=La documentación forma parte del contrato de entrega.\n");
    expect(NxDocumentationManager_LoadRelease("Build/docs_manager_release.nxdoc",&release,message,sizeof(message))==1,"release descriptor loads");
    write_text("Build/docs_manager_test/Include/Nexiora/NCOS/NxDocumentationManager.h","test\n");
    write_text("Build/docs_manager_test/Source/NCOS/NxDocumentationManager.c","test\n");
    write_text("Build/docs_manager_test/Tools/NCOS/NxDocumentationTool.c","test\n");
    write_text("Build/docs_manager_test/Tests/Unit/NxDocumentationManagerTests.c","test\n");
    expect(NxDocumentationManager_FinalizeSprint(root,&release,message,sizeof(message))==1,"valid release finalizes atomically");
    (void)snprintf(path,sizeof(path),"%s/Docs/CHANGELOG.md",root); expect(contains(path,"Self Validation Engine") == 0,"capability text is used rather than sprint name"); expect(contains(path,"Validación autónoma") == 1,"changelog receives capability");
    (void)snprintf(path,sizeof(path),"%s/Docs/DECISIONS.md",root); expect(contains(path,"ADR-0002") == 1,"decision is recorded");
    expect(NxDocumentationManager_Validate(root,&result)==1,"updated document set validates");
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
