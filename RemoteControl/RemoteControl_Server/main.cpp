#include <WinSock2.h>
#include <windows.h>
#include <cstdio>
#include <vector>
#include <algorithm>

#pragma comment(lib, "ws2_32.lib")

#include "Message.h"

// ——— 클라이언트 정보용 구조체 ———
struct SenderInfo {
    SOCKET   sock;
    uint32_t id;
};

struct ReceiverInfo {
    SOCKET   sock;
    uint32_t id;
    uint32_t targetId;
};

// ——— 포워딩 파라미터 ———
struct ForwardParam {
    SOCKET senderSock;
    SOCKET recvSock;
};

// ——— 전역 데이터 ———
CRITICAL_SECTION                   g_cs;
std::vector<SenderInfo>            g_senders;
std::vector<ReceiverInfo>          g_receivers;

// 함수 선언
DWORD WINAPI ClientHandshake(LPVOID lpParam);
DWORD WINAPI MatchThread(LPVOID lpParam);
DWORD WINAPI ForwardThread(LPVOID lpParam);
bool recvn_server(SOCKET sock, void* buf, size_t len);
int sendn_server(SOCKET sock, const char* buf, int len);

// ——— 클라이언트 핸드셰이크 처리 쓰레드 ———
DWORD WINAPI ClientHandshake(LPVOID lpParam) {
    SOCKET s = (SOCKET)lpParam;
    ConnRequestHeader hdr;

    // 1. 요청 헤더 수신
    if (!recvn_server(s, &hdr, sizeof(hdr))) {
        closesocket(s);
        return 0;
    }

    // (네트워크 바이트 순서 → 호스트 바이트 순서)
    uint32_t myId = ntohl(hdr.myId);
    uint32_t targetId = ntohl(hdr.target);

    ConnResponse resp = {};
    EnterCriticalSection(&g_cs);
    if (hdr.mode == 's') {
        // — 송신자 등록 —
        g_senders.push_back({ s, myId });
        resp.success = 1;
        strcpy_s(resp.info, "Registered as sender");
    }
    else if (hdr.mode == 'r') {
        // — 수신자 요청 시 매칭 검사 —
        auto it = std::find_if(
            g_senders.begin(), g_senders.end(),
            [targetId](const SenderInfo& si) { return si.id == targetId; }
        );
        if (it != g_senders.end()) {
            // 매칭 성공
            g_receivers.push_back({ s, myId, targetId });
            resp.success = 1;
            strcpy_s(resp.info, "Matched sender found");
        }
        else {
            // 매칭 실패
            resp.success = 0;
            strcpy_s(resp.info, "No sender with given ID");
        }
    }
    else {
        resp.success = 0;
        strcpy_s(resp.info, "Invalid mode");
    }
    LeaveCriticalSection(&g_cs);

    // 2) 응답 전송
    ::send(s, reinterpret_cast<char*>(&resp), sizeof(resp), 0);

    return 0; // 이후 실제 데이터 송수신 스레드로 확장 가능
}

// ——— Ctrl+C 처리기 ———
BOOL CtrlHandler(DWORD type) {
    if (type == CTRL_C_EVENT) {
        EnterCriticalSection(&g_cs);
        for (auto& si : g_senders)    closesocket(si.sock);
        for (auto& ri : g_receivers)  closesocket(ri.sock);
        g_senders.clear();
        g_receivers.clear();
        LeaveCriticalSection(&g_cs);

        WSACleanup();
        exit(0);
    }
    return FALSE;
}

int main() {
    // WinSock 초기화
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed\n");
        return -1;
    }

    InitializeCriticalSection(&g_cs);
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE);

    SOCKET listenSock = ::socket(AF_INET, SOCK_STREAM, 0);
    SOCKADDR_IN addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(25000);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // 스레드1: 매칭 쓰레드
    DWORD matchTid;
    HANDLE hMatch = CreateThread(nullptr, 0, MatchThread, nullptr, 0, &matchTid);
    CloseHandle(hMatch);

    bind(listenSock, (SOCKADDR*)&addr, sizeof(addr));
    listen(listenSock, SOMAXCONN);
    printf("Server listening on port 25000...\n");

    while (true) {
        SOCKET client = accept(listenSock, nullptr, nullptr);
        if (client == INVALID_SOCKET) continue;

        // 연결된 클라이언트마다 핸드셰이크 쓰레드 생성
        DWORD tid;
        HANDLE h = CreateThread(
            nullptr, 0,
            ClientHandshake,
            (LPVOID)client,
            0, &tid
        );
        CloseHandle(h);
    }

    // (절대 못 옴)
    DeleteCriticalSection(&g_cs);
    closesocket(listenSock);
    WSACleanup();
    return 0;
}

// ——— 매칭 및 데이터전달 시작 쓰레드 ———
DWORD WINAPI MatchThread(LPVOID) {
    while (true) {
        EnterCriticalSection(&g_cs);
        for (auto it = g_receivers.begin(); it != g_receivers.end(); ) {
            uint32_t target = it->targetId;
            auto sit = std::find_if(g_senders.begin(), g_senders.end(),
                [target](const SenderInfo& si) { return si.id == target; });
            if (sit != g_senders.end()) {
                // 매칭 성사: 송신->수신 연결
                ForwardParam* param = new ForwardParam{ sit->sock, it->sock };
                DWORD fid;
                HANDLE h = CreateThread(nullptr, 0, ForwardThread, param, 0, &fid);
                CloseHandle(h);
                // 벡터에서 수신자 제거
                it = g_receivers.erase(it);
                continue;
            }
            ++it;
        }
        LeaveCriticalSection(&g_cs);
        Sleep(100);
    }
    return 0;
}

// ——— 데이터 포워딩 쓰레드 ———
DWORD WINAPI ForwardThread(LPVOID lpParam) {
    ForwardParam* p = (ForwardParam*)lpParam;
    SOCKET sendSock = p->senderSock;
    SOCKET recvSock = p->recvSock;
    delete p;

    // Message 헤더 크기
    size_t headerSize = offsetof(Message, payload);
    while (true) {
        // 1) 헤더 수신
        std::vector<uint8_t> headerBuf(headerSize);
        if (!recvn_server(sendSock, headerBuf.data(), headerSize)) break;
        // 2) payloadSize 파싱
        Message* mh = reinterpret_cast<Message*>(headerBuf.data());
        if (ntohl(mh->magic) != 0x424D4350) break;
        uint64_t payloadSize = ntohll(mh->payloadSize);
        size_t totalSize = headerSize + payloadSize;
        // 3) 전체 버퍼 할당
        uint8_t* buf = new uint8_t[totalSize];
        memcpy(buf, headerBuf.data(), headerSize);
        if (!recvn_server(sendSock, buf + headerSize, payloadSize)) { delete[] buf; break; }
        // 4) 수신자에게 전송
        if (sendn_server(recvSock, (char*)buf, (int)totalSize) == SOCKET_ERROR) { delete[] buf; break; }
        delete[] buf;
    }

    closesocket(sendSock);
    closesocket(recvSock);

    // **벡터에서 sender 정보 삭제**
    EnterCriticalSection(&g_cs);
    auto it = std::find_if(
        g_senders.begin(), g_senders.end(),
        [sendSock](const SenderInfo& si) { return si.sock == sendSock; }
    );
    if (it != g_senders.end()) {
        g_senders.erase(it);
    }
    LeaveCriticalSection(&g_cs);

    return 0;
}


// ——— 유틸: 정확히 recv(len) 바이트 읽기 ———
bool recvn_server(SOCKET sock, void* buf, size_t len) {
    uint8_t* ptr = (uint8_t*)buf;
    size_t rec = 0;
    while (rec < len) {
        int r = recv(sock, (char*)(ptr + rec), int(len - rec), 0);
        if (r <= 0) return false;
        rec += r;
    }
    return true;
}

// ——— 유틸: send 전부 보내기 ———
int sendn_server(SOCKET sock, const char* buf, int len) {
    int sent = 0;
    while (sent < len) {
        int n = send(sock, buf + sent, len - sent, 0);
        if (n == SOCKET_ERROR) return SOCKET_ERROR;
        sent += n;
    }
    return sent;
}