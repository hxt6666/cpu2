#include <cinttypes>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <android/log.h>
#include

<sys/mount.h>
#include <dlfcn.h>
#include <link.h>
#include <sys/auxv.h>
#include <sys/stat.h>
#include <vector>
#include <string>

#include "zygisk_next_api.h"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "KirinSpoof", __VA_ARGS__)

static ZygiskNextAPI api_table;
void* handle = nullptr;
bool spoofed = false;

// 硬编码的 Kirin 9030 cpuinfo 数据
const char* KIRIN_9030_DATA = R"(Processor : AArch64 Processor rev 0 (aarch64)
processor	: 0
BogoMIPS	: 2000.00
Features	: fp asimd evtstrm aes pmull sha1 sha2 crc32 atomics fphp asimdhp cpuid asimdrdm jscvt fcma lrcpc dcpop sha3 sm3 sm4 asimddp sha512 sve asimdfhm dit uscat ilrcpc flagm ssbs sb paca pacg dcpodp flagm2 frint svei8mm svebf16 i8mm bf16 dgh bti
CPU implementer	: 0x48
CPU architecture: 8
CPU variant	: 0x2
CPU part	: 0xd24
CPU revision	: 0
CPU physical	: 0

processor	: 1
BogoMIPS	: 2000.00
Features	: fp asimd evtstrm aes pmull sha1 sha2 crc32 atomics fphp asimdhp cpuid asimdrdm jscvt fcma lrcpc dcpop sha3 sm3 sm4 asimddp sha512 sve asimdfhm dit uscat ilrcpc flagm ssbs sb paca pacg dcpodp flagm2 frint svei8mm svebf16 i8mm bf16 dgh bti
CPU implementer	: 0x48
CPU architecture: 8
CPU variant	: 0x2
CPU part	: 0xd24
CPU revision	: 0
CPU physical	: 1

processor	: 2
BogoMIPS	: 2000.00
Features	: fp asimd evtstrm aes pmull sha1 sha2 crc32 atomics fphp asimdhp cpuid asimdrdm jscvt fcma lrcpc dcpop sha3 sm3 sm4 asimddp sha512 sve asimdfhm dit uscat ilrcpc flagm ssbs sb paca pacg dcpodp flagm2 frint svei8mm svebf16 i8mm bf16 dgh bti
CPU implementer	: 0x48
CPU architecture: 8
CPU variant	: 0x2
CPU part	: 0xd24
CPU revision	: 0
CPU physical	: 2

processor	: 3
BogoMIPS	: 2000.00
Features	: fp asimd evtstrm aes pmull sha1 sha2 crc32 atomics fphp asimdhp cpuid asimdrdm jscvt fcma lrcpc dcpop sha3 sm3 sm4 asimddp sha512 sve asimdfhm dit uscat ilrcpc flagm ssbs sb paca pacg dcpodp flagm2 frint svei8mm svebf16 i8mm bf16 dgh bti
CPU implementer	: 0x48
CPU architecture: 8
CPU variant	: 0x2
CPU part	: 0xd24
CPU revision	: 0
CPU physical	: 3

processor	: 4
BogoMIPS	: 2000.00
Features	: fp asimd evtstrm aes pmull sha1 sha2 crc32 atomics fphp asimdhp cpuid asimdrdm jscvt fcma lrcpc dcpop sha3 sm3 sm4 asimddp sha512 sve asimdfhm dit uscat ilrcpc flagm ssbs sb paca pacg dcpodp flagm2 frint svei8mm svebf16 i8mm bf16 dgh bti
CPU implementer	: 0x48
CPU architecture: 8
CPU variant	: 0x2
CPU part	: 0xd47
CPU revision	: 0
CPU physical	: 4

processor	: 5
BogoMIPS	: 2000.00
Features	: fp asimd evtstrm aes pmull sha1 sha2 crc32 atomics fphp asimdhp cpuid asimdrdm jscvt fcma lrcpc dcpop sha3 sm3 sm4 asimddp sha512 sve asimdfhm dit uscat ilrcpc flagm ssbs sb paca pacg dcpodp flagm2 frint svei8mm svebf16 i8mm bf16 dgh bti
CPU implementer	: 0x48
CPU architecture: 8
CPU variant	: 0x2
CPU part	: 0xd47
CPU revision	: 0
CPU physical	: 5

processor	: 6
BogoMIPS	: 2000.00
Features	: fp asimd evtstrm aes pmull sha1 sha2 crc32 atomics fphp asimdhp cpuid asimdrdm jscvt fcma lrcpc dcpop sha3 sm3 sm4 asimddp sha512 sve asimdfhm dit uscat ilrcpc flagm ssbs sb paca pacg dcpodp flagm2 frint svei8mm svebf16 i8mm bf16 dgh bti
CPU implementer	: 0x48
CPU architecture: 8
CPU variant	: 0x2
CPU part	: 0xd47
CPU revision	: 0
CPU physical	: 6

processor	: 7
BogoMIPS	: 2000.00
Features	: fp asimd evtstrm aes pmull sha1 sha2 crc32 atomics fphp asimdhp cpuid asimdrdm jscvt fcma lrcpc dcpop sha3 sm3 sm4 asimddp sha512 sve asimdfhm dit uscat ilrcpc flagm ssbs sb paca pacg dcpodp flagm2 frint svei8mm svebf16 i8mm bf16 dgh bti
CPU implementer	: 0x48
CPU architecture: 8
CPU variant	: 0x2
CPU part	: 0xd47
CPU revision	: 0
CPU physical	: 7

processor	: 8
BogoMIPS	: 2000.00
Features	: fp asimd evtstrm aes pmull sha1 sha2 crc32 atomics fphp asimdhp cpuid asimdrdm jscvt fcma lrcpc dcpop sha3 sm3 sm4 asimddp sha512 sve asimdfhm dit uscat ilrcpc flagm ssbs sb paca pacg dcpodp flagm2 frint svei8mm svebf16 i8mm bf16 dgh bti
CPU implementer	: 0x48
CPU architecture: 8
CPU variant	: 0x2
CPU part	: 0xd47
CPU revision	: 0
CPU physical	: 8

processor	: 9
BogoMIPS	: 2000.00
Features	: fp asimd evtstrm aes pmull sha1 sha2 crc32 atomics fphp asimdhp cpuid asimdrdm jscvt fcma lrcpc dcpop sha3 sm3 sm4 asimddp sha512 sve asimdfhm dit uscat ilrcpc flagm ssbs sb paca pacg dcpodp flagm2 frint svei8mm svebf16 i8mm bf16 dgh bti
CPU implementer	: 0x48
CPU architecture: 8
CPU variant	: 0x2
CPU part	: 0xd47
CPU revision	: 0
CPU physical	: 9

processor	: 10
BogoMIPS	: 2000.00
Features	: fp asimd evtstrm aes pmull sha1 sha2 crc32 atomics fphp asimdhp cpuid asimdrdm jscvt fcma lrcpc dcpop sha3 sm3 sm4 asimddp sha512 sve asimdfhm dit uscat ilrcpc flagm ssbs sb paca pacg dcpodp flagm2 frint svei8mm svebf16 i8mm bf16 dgh bti
CPU implementer	: 0x48
CPU architecture: 8
CPU variant	: 0x2
CPU part	: 0xd47
CPU revision	: 0
CPU physical	: 10

processor	: 11
BogoMIPS	: 2000.00
Features	: fp asimd evtstrm aes pmull sha1 sha2 crc32 atomics fphp asimdhp cpuid asimdrdm jscvt fcma lrcpc dcpop sha3 sm3 sm4 asimddp sha512 sve asimdfhm dit uscat ilrcpc flagm ssbs sb paca pacg dcpodp flagm2 frint svei8mm svebf16 i8mm bf16 dgh bti
CPU implementer	: 0x48
CPU architecture: 8
CPU variant	: 0x2
CPU part	: 0xd47
CPU revision	: 0
CPU physical	: 11

processor	: 12
BogoMIPS	: 2000.00
Features	: fp asimd evtstrm aes pmull sha1 sha2 crc32 atomics fphp asimdhp cpuid asimdrdm jscvt fcma lrcpc dcpop sha3 sm3 sm4 asimddp sha512 sve asimdfhm dit uscat ilrcpc flagm ssbs sb paca pacg dcpodp flagm2 frint svei8mm svebf16 i8mm bf16 dgh bti
CPU implementer	: 0x48
CPU architecture: 8
CPU variant	: 0x2
CPU part	: 0xd06
CPU revision	: 0
CPU physical	: 12

processor	: 13
BogoMIPS	: 2000.00
Features	: fp asimd evtstrm aes pmull sha1 sha2 crc32 atomics fphp asimdhp cpuid asimdrdm jscvt fcma lrcpc dcpop sha3 sm3 sm4 asimddp sha512 sve asimdfhm dit uscat ilrcpc flagm ssbs sb paca pacg dcpodp flagm2 frint svei8mm svebf16 i8mm bf16 dgh bti
CPU implementer	: 0x48
CPU architecture: 8
CPU variant	: 0x2
CPU part	: 0xd06
CPU revision	: 0
CPU physical	: 13

Hardware	: HUAWEI Kirin9030


)";// 目标游戏包名列表
static const char* TARGET_PACKAGES[] = {
    "com.tencent.tmgp.pubgmhd",     // 和平精英（海外版）
};

static bool shouldSpoof(const char* processName) {
    if (!processName) return false;
    for (auto& pkg : TARGET_PACKAGES) {
        if (std::string(processName) == pkg) return true;
    }
    return false;
}

static const char* FAKE_CPUINFO_PATH = "/data/local/tmp/.kirin_cpuinfo";

static int (*old_openat)(int dfd, const char* pathname, int flags, ...) = nullptr;

static int my_openat(int dfd, const char* pathname, int flags, ...) {
    if (pathname && strcmp(pathname, "/proc/cpuinfo") == 0) {
        int flag_fd = open("/data/local/tmp/.kirin_spoof_flag", O_RDONLY);
        if (flag_fd >= 0) {
            char buf[2] = {0};
            read(flag_fd, buf, 1);
            close(flag_fd);
            if (buf[0] == '1') {int fake_fd = open(FAKE_CPUINFO_PATH, O_RDONLY);
                LOGI("[KirinSpoof] Hooked /proc/cpuinfo, returning fake fd=%d", fake_fd);
                return fake_fd;
            }
        }
    }
    va_list va;
    va_start(va, flags);
    int mode = va_arg(va, int);
    va_end(va);
    return old_openat(dfd, pathname, flags, mode);
}

static void doKirinSpoof() {
    if (spoofed) return;
    spoofed = true;
    LOGI("[KirinSpoof] Starting Kirin 9030 spoofing...");

    int fd = open(FAKE_CPUINFO_PATH, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd < 0) {
        LOGI("[KirinSpoof] Failed to create fake cpuinfo");
        return;
    }
    size_t data_len = strlen(KIRIN_9030_DATA);
    write(fd, KIRIN_9030_DATA, data_len);
    close(fd);

    if (mount(FAKE_CPUINFO_PATH, "/proc/cpuinfo", nullptr, MS_BIND, nullptr) == 0) {
        LOGI("[KirinSpoof] mount success!");unlink(FAKE_CPUINFO_PATH);
    } else {
        LOGI("[KirinSpoof] mount failed! errno=%d", errno);
        unlink(FAKE_CPUINFO_PATH);
    }
}

void onModuleLoaded(void* self_handle, const struct ZygiskNextAPI* api) {
    memcpy(&api_table, api, sizeof(struct ZygiskNextAPI));
    handle = self_handle;
    LOGI("[KirinSpoof] Module loaded!");

    char cmdline[256] = {0};
    int cfd = open("/proc/self/cmdline", O_RDONLY);
    if (cfd >= 0) { read(cfd, cmdline, sizeof(cmdline) - 1); close(cfd); }
    const char* processName = cmdline;
    LOGI("[KirinSpoof] Process: %s", processName ? processName : "unknown");

    if (!shouldSpoof(processName)) return;

    LOGI("[KirinSpoof] Target detected!");

    int flag_fd = open("/data/local/tmp/.kirin_spoof_flag", O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (flag_fd >= 0) { write(flag_fd, "1", 1); close(flag_fd); }

   doKirinSpoof();auto resolver = api_table.newSymbolResolver("libc.so", nullptr);
    if (!resolver) return;
    void* openat_addr = api_table.symbolLookup(resolver, "__openat", false, nullptr);
    api_table.freeSymbolResolver(resolver);
    if (!openat_addr) return;

    if (api_table.inlineHook(openat_addr, (void*)my_openat, (void**)&old_openat) == ZN_SUCCESS) {
        LOGI("[KirinSpoof] __openat hook success!");
    }
}

__attribute__((visibility("default"), unused))
struct ZygiskNextModule zn_module = {
    .target_api_version = ZYGISK_NEXT_API_VERSION_1,
    .onModuleLoaded = onModuleLoaded
};

static void onCompanionLoaded() {}
static void onModuleConnected(int fd) {
    char buf[128];
    int n = read(fd, buf, sizeof(buf) - 1);
    if (n > 0) { buf[n] = 0; LOGI("[KirinSpoof] Companion: %s", buf); }

}

__attribute__((visibility("default"), unused))
struct ZygiskNextCompanionModule zn_companion_module = {
    .target_api_version = ZYGISK_NEXT_API_VERSION_1,
    .onCompanionLoaded = onCompanionLoaded,
    .onModuleConnected = onModuleConnected,
};
