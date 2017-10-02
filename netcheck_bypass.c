#include <psp2/kernel/modulemgr.h>
#include <taihen.h>

#include "log.h"

#define SCE_NETCHECK_DIALOG_MODE_PSN 2
#define SCE_NETCHECK_DIALOG_MODE_PSN_ONLINE 3

#define SCE_COMMON_DIALOG_ERROR_NOT_IN_USE 0x80020411
#define SCE_COMMON_DIALOG_STATUS_NONE 0
#define SCE_COMMON_DIALOG_STATUS_FINISHED 2

typedef struct SceCommonDialogParam {
  void *infobarParam;
  void *bgColor;
  void *dimmerColor;
  char reserved[60];
  int magic;
} SceCommonDialogParam;

typedef struct SceNetCheckDialogAgeRestriction {
  char countryCode[2];
  char age;
  char padding;
} SceNetCheckDialogAgeRestriction;

typedef struct SceNetCheckDialogParam {
  int sdkVersion;
  SceCommonDialogParam commonParam;
  int mode;
  int npCommunicationId;
  int *ps3ConnectParam;
  void *groupName;
  int timeoutUs;
  char defaultAgeRestriction;
  char padding[3];
  int ageRestrictionCount;
  const SceNetCheckDialogAgeRestriction *ageRestriction;
  char reserved[104];
} SceNetCheckDialogParam;

static SceUID g_hooks[10];

static tai_hook_ref_t g_sceNetCheckDialogInit_hook;
static int sceNetCheckDialogInit_patched(SceNetCheckDialogParam *param) {
  int ret;
  if (param->mode == SCE_NETCHECK_DIALOG_MODE_PSN || param->mode == SCE_NETCHECK_DIALOG_MODE_PSN_ONLINE) {
    ret = 0;
  } else {
    ret = TAI_CONTINUE(int, g_sceNetCheckDialogInit_hook, param);
  }
  return ret;
}

static tai_hook_ref_t g_sceNetCheckDialogInit2_hook;
static int sceNetCheckDialogInit2_patched(SceNetCheckDialogParam *param, void *opt) {
  int ret;
  if (param->mode == SCE_NETCHECK_DIALOG_MODE_PSN || param->mode == SCE_NETCHECK_DIALOG_MODE_PSN_ONLINE) {
    ret = 0;
  } else {
    ret = TAI_CONTINUE(int, g_sceNetCheckDialogInit2_hook, param, opt);
  }
  return ret;
}

static tai_hook_ref_t g_sceNetCheckDialogAbort_hook;
static int sceNetCheckDialogAbort_patched(void) {
  int ret;
  ret = TAI_CONTINUE(int, g_sceNetCheckDialogAbort_hook);
  if (ret == SCE_COMMON_DIALOG_ERROR_NOT_IN_USE) {
    ret = 0;
  }
  return ret;
}

static tai_hook_ref_t g_sceNetCheckDialogGetResult_hook;
static int sceNetCheckDialogGetResult_patched(void *result) {
  int ret;
  ret = TAI_CONTINUE(int, g_sceNetCheckDialogGetResult_hook, result);
  if (ret == SCE_COMMON_DIALOG_ERROR_NOT_IN_USE) {
    ret = 0;
  }
  return ret;
}

static tai_hook_ref_t g_sceNetCheckDialogGetStatus_hook;
static int sceNetCheckDialogGetStatus_patched(void) {
  int ret;
  ret = TAI_CONTINUE(int, g_sceNetCheckDialogGetStatus_hook);
  if (ret == SCE_COMMON_DIALOG_ERROR_NOT_IN_USE || ret == SCE_COMMON_DIALOG_STATUS_NONE) {
    ret = SCE_COMMON_DIALOG_STATUS_FINISHED;
  }
  return ret;
}

static tai_hook_ref_t g_sceNetCheckDialogTerm_hook;
static int sceNetCheckDialogTerm_patched(void) {
  int ret;
  ret = TAI_CONTINUE(int, g_sceNetCheckDialogTerm_hook);
  if (ret == SCE_COMMON_DIALOG_ERROR_NOT_IN_USE) {
    ret = 0;
  }
  return ret;
}

// #define FAKE_AID 0x21191d6d5de7c6dbLL
#define FAKE_AID 0x0123456789ABCDEFLL

static tai_hook_ref_t g_sceRegMgrSystemParamGetBin_hook;
static int sceRegMgrSystemParamGetBin_patched(int param, void* dest, int size){
  if(param == 0x450F32){
    *(long long *)dest = FAKE_AID;
    LOG("ksceRegMgrSystemParamGetBin_patched: %llx %x\n", *(long long *)dest, size);
    return 0;
  }

  int res = TAI_CONTINUE(int, g_sceRegMgrSystemParamGetBin_hook, param, dest, size);
  return res;
}


static tai_hook_ref_t g_sceRegMgrSystemParamGetInt_hook;
static int sceRegMgrSystemParamGetInt_patched(int param, int* dest){
   int res = TAI_CONTINUE(int, g_sceRegMgrSystemParamGetInt_hook, param, dest);
   if(param == 0x93C981){
      LOG("ksceRegMgrSystemParamGetInt_patched: %x %x\n", *dest, res);
      *dest = 1;
     return 0;
  }
  return res;
}

void _start() __attribute__ ((weak, alias ("module_start")));
int module_start(SceSize argc, const void *args) {
  g_hooks[0] = taiHookFunctionImport(&g_sceNetCheckDialogInit_hook, 
                                      TAI_MAIN_MODULE, 
                                      0xE537816C, // SceCommonDialog
                                      0xA38A4A0D, 
                                      sceNetCheckDialogInit_patched);
  g_hooks[1] = taiHookFunctionImport(&g_sceNetCheckDialogInit2_hook, 
                                      TAI_MAIN_MODULE, 
                                      0xE537816C, // SceCommonDialog
                                      0x243D6A36, 
                                      sceNetCheckDialogInit2_patched);
  g_hooks[2] = taiHookFunctionImport(&g_sceNetCheckDialogAbort_hook, 
                                      TAI_MAIN_MODULE, 
                                      0xE537816C, // SceCommonDialog
                                      0x2D8EDF09, 
                                      sceNetCheckDialogAbort_patched);
  g_hooks[3] = taiHookFunctionImport(&g_sceNetCheckDialogGetResult_hook, 
                                      TAI_MAIN_MODULE, 
                                      0xE537816C, // SceCommonDialog
                                      0xB05FCE9E, 
                                      sceNetCheckDialogGetResult_patched);
  g_hooks[4] = taiHookFunctionImport(&g_sceNetCheckDialogGetStatus_hook, 
                                      TAI_MAIN_MODULE, 
                                      0xE537816C, // SceCommonDialog
                                      0x8027292A, 
                                      sceNetCheckDialogGetStatus_patched);
  g_hooks[5] = taiHookFunctionImport(&g_sceNetCheckDialogTerm_hook, 
                                      TAI_MAIN_MODULE, 
                                      0xE537816C, // SceCommonDialog
                                      0x8BE51C15, 
                                      sceNetCheckDialogTerm_patched);
  g_hooks[6] = taiHookFunctionImport(&g_sceRegMgrSystemParamGetBin_hook, 
                                      TAI_MAIN_MODULE, 
                                      0x0B351269,
                                      0x7FFE2CDF, 
                                      sceRegMgrSystemParamGetBin_patched);
  LOG("getbin: 0x%08X\n", g_hooks[6]);
  g_hooks[7] = taiHookFunctionImport(&g_sceRegMgrSystemParamGetInt_hook, 
    TAI_MAIN_MODULE, 
    0xB351269,
    0x347C1BDB, 
    sceRegMgrSystemParamGetInt_patched);

  LOG("getint: 0x%08X\n", g_hooks[6]);
  return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args) {
  if (g_hooks[0] >= 0) taiHookRelease(g_hooks[0], g_sceNetCheckDialogInit_hook);
  if (g_hooks[1] >= 0) taiHookRelease(g_hooks[1], g_sceNetCheckDialogInit2_hook);
  if (g_hooks[2] >= 0) taiHookRelease(g_hooks[2], g_sceNetCheckDialogAbort_hook);
  if (g_hooks[3] >= 0) taiHookRelease(g_hooks[3], g_sceNetCheckDialogGetResult_hook);
  if (g_hooks[4] >= 0) taiHookRelease(g_hooks[4], g_sceNetCheckDialogGetStatus_hook);
  if (g_hooks[5] >= 0) taiHookRelease(g_hooks[5], g_sceNetCheckDialogTerm_hook);
  if (g_hooks[6] >= 0) taiHookRelease(g_hooks[6], g_sceRegMgrSystemParamGetBin_hook);
  if (g_hooks[7] >= 0) taiHookRelease(g_hooks[6], g_sceRegMgrSystemParamGetBin_hook);
  return SCE_KERNEL_STOP_SUCCESS;
}
