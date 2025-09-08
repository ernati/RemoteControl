# Documentation for `main.cpp`

## 1. Main Function
<입력 파라미터> :
- 없음

<리턴 값> :
- `int` — 프로그램 종료 코드

<하위 단계들>
1.1. WinSock 초기화:
    - `WSAStartup` 호출로 WinSock 라이브러리 초기화.
    - 실패 시 에러 메시지 출력 후 종료.
1.2. 전역 리소스 초기화:
    - `InitializeCriticalSection` 호출로 임계영역 초기화.
    - `SetConsoleCtrlHandler`로 Ctrl+C 처리기 등록.
1.3. 서버 소켓 생성:
    - `socket` 호출로 TCP 소켓 생성.
    - 실패 시 에러 메시지 출력 후 종료.
1.4. 소켓 옵션 설정:
    - `setsockopt` 호출로 `SO_REUSEADDR` 옵션 활성화.
1.5. 로컬 IP 주소 출력:
    - `gethostname` 및 `getaddrinfo` 호출로 IP 주소 목록 출력.
1.6. 소켓 바인딩:
    - `bind` 호출로 소켓을 특정 포트에 바인딩.
    - 실패 시 에러 메시지 출력 후 종료.
1.7. 리슨 상태로 전환:
    - `listen` 호출로 클라이언트 연결 대기 상태로 전환.
1.8. 매칭 쓰레드 생성:
    - `CreateThread` 호출로 매칭 쓰레드 시작.
1.9. 클라이언트 연결 처리:
    - `accept` 호출로 클라이언트 연결 수락.
    - 각 연결에 대해 핸드셰이크 쓰레드 생성.
1.10. 종료 처리:
    - 리소스 해제 및 종료.

===========================

## 2. `ClientHandshake` Function
<입력 파라미터> :
- `LPVOID lpParam` — 클라이언트 소켓 핸들.

<리턴 값> :
- `DWORD` — 쓰레드 종료 코드.

<하위 단계들>
2.1. 요청 헤더 수신:
    - `recvn` 호출로 클라이언트 요청 헤더 수신.
    - 실패 시 소켓 종료 후 반환.
2.2. 사용자 인증:
    - MySQL 드라이버 및 커넥션 생성.
    - 사용자 ID와 비밀번호를 조회 및 검증.
    - 인증 실패 시 에러 메시지 전송 후 소켓 종료.
2.3. 클라이언트 등록:
    - 요청 모드(`hdr.mode`)에 따라 송신자 또는 수신자로 등록.
    - 매칭 실패 시 에러 메시지 전송.
2.4. 응답 전송:
    - 인증 및 등록 결과를 클라이언트에 전송.

===========================

## 3. `MatchThread` Function
<입력 파라미터> :
- 없음

<리턴 값> :
- `DWORD` — 쓰레드 종료 코드.

<하위 단계들>
3.1. 매칭 검사:
    - 임계영역 진입 후 수신자와 송신자 매칭 시도.
    - 매칭 성공 시 데이터 포워딩 쓰레드 생성.
3.2. 대기:
    - `Sleep` 호출로 일정 시간 대기 후 반복.

===========================

## 4. `ForwardThread` Function
<입력 파라미터> :
- `LPVOID lpParam` — 송신자 및 수신자 소켓 정보.

<리턴 값> :
- `DWORD` — 쓰레드 종료 코드.

<하위 단계들>
4.1. 데이터 포워딩:
    - 송신자로부터 메시지 헤더 및 페이로드 수신.
    - 수신자에게 데이터 전송.
4.2. 리소스 해제:
    - 소켓 종료 및 송신자 정보 제거.

===========================

## Dependencies
### `CommonAPI.h`
- `recvn(SOCKET sock, void* buf, size_t len)` — 데이터 수신 함수.
- `sendn(SOCKET sock, const char* buffer, int totalBytes)` — 데이터 전송 함수.

### `Message.h`
- `struct Message` — 메시지 구조체 정의.
- `calculateCRC32` — CRC32 체크섬 계산 함수.
- `createMessageBuffer_tmp` — 메시지 버퍼 생성 함수.
