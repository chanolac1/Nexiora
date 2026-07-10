#include "Nexiora/Multimodal/NxMultimodalIngestion.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#if defined(_WIN32)
#include <direct.h>
#define nx_mkdir(path) _mkdir(path)
#else
#include <sys/stat.h>
#define nx_mkdir(path) mkdir((path),0777)
#endif
static void write_text(const char* p,const char* t){FILE* f=fopen(p,"wb");assert(f!=NULL);assert(fwrite(t,1,strlen(t),f)==strlen(t));assert(fclose(f)==0);} 
int main(void){NxMediaProbe p;NxIngestionResult r;NxMultimodalStatus s;(void)nx_mkdir("nx_mm_test");write_text("nx_mm_test/a.txt","Nexiora aprende contenido verificable.\n");s=NxMultimodal_Probe("nx_mm_test/a.txt",&p);assert(s==NX_MM_OK);assert(p.type==NX_MEDIA_TEXT);s=NxMultimodal_Ingest("nx_mm_test","nx_mm_test/a.txt","Prueba Texto",&r);assert(s==NX_MM_OK);assert(r.extracted_bytes>0U);assert(strstr(r.knowledge_path,"prueba_texto.nxknowledge")!=NULL);write_text("nx_mm_test/video.mp4","fake");s=NxMultimodal_Ingest("nx_mm_test","nx_mm_test/video.mp4","Video",&r);assert(s==NX_MM_TRANSCRIPT_REQUIRED);write_text("nx_mm_test/video.txt","Transcripcion verificable.\n");s=NxMultimodal_Ingest("nx_mm_test","nx_mm_test/video.mp4","Video",&r);assert(s==NX_MM_OK);remove("nx_mm_test/a.txt");remove("nx_mm_test/video.mp4");remove("nx_mm_test/video.txt");remove("nx_mm_test/Knowledge/Multimodal/prueba_texto.nxknowledge");remove("nx_mm_test/Knowledge/Multimodal/video.nxknowledge");puts("NxMultimodalIngestionTests: PASS");return 0;}
