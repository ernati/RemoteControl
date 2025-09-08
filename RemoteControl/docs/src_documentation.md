# Documentation for `Src` Directory

## 1. `CCaptureScreenAndSendBitMap.cpp`
### Class: `CCaptureScreenAndSendBitMap`
<입력 파라미터> :
- 생성자: 없음 또는 `UINT capture_interval` — 캡처 간격(ms).

<리턴 값> :
- 없음

<하위 단계들>
1.1. 생성자:
    - 멤버 변수 초기화.
    - `CAPTURE_INTERVAL` 설정.
1.2. 소멸자:
    - `DeleteObject` 호출로 GDI 리소스 해제.
1.3. `CaptureScreen` 메서드:
    - 화면 DC 및 메모리 DC 생성.
    - 화면 크기 가져오기.
    - 호환 비트맵 생성.

===========================

## 2. `CommonAPI.cpp`
### Functions
#### `recvn`
<입력 파라미터> :
- `SOCKET sock` — 소켓 핸들.
- `void* buf` — 데이터 버퍼.
- `size_t len` — 수신할 데이터 길이.

<리턴 값> :
- `bool` — 성공 여부.

<하위 단계들>
2.1. 데이터 수신:
    - `recv` 호출로 데이터 수신.
    - 수신된 바이트 수를 누적.
    - 실패 시 `false` 반환.

#### `sendn`
<입력 파라미터> :
- `SOCKET sock` — 소켓 핸들.
- `const char* buffer` — 전송할 데이터 버퍼.
- `int totalBytes` — 전송할 데이터 길이.

<리턴 값> :
- `int` — 전송된 바이트 수.

<하위 단계들>
2.2. 데이터 전송:
    - `send` 호출로 데이터 전송.
    - 전송된 바이트 수를 누적.
    - 실패 시 `SOCKET_ERROR` 반환.

===========================

## 3. `CPrintScreen.cpp`
### Class: `CPrintScreen`
<입력 파라미터> :
- 생성자: 없음 또는 `HINSTANCE hInstance` — 애플리케이션 인스턴스 핸들.

<리턴 값> :
- 없음

<하위 단계들>
3.1. 생성자:
    - 멤버 변수 초기화.
    - 화면 크기 기본값 설정.
3.2. 소멸자:
    - 리소스 해제.

===========================

## 4. `CRemoteControlRecvMode.cpp`
### Class: `CRemoteControlRecvMode`
<입력 파라미터> :
- 생성자: 없음 또는 사용자 인증 정보 및 ID.

<리턴 값> :
- 없음

<하위 단계들>
4.1. 생성자:
    - 멤버 변수 초기화.
    - 인증 정보 복사.
4.2. 소멸자:
    - 소켓 종료 및 WinSock 정리.

===========================

## 5. `CRemoteControlSendMode.cpp`
### Class: `CRemoteControlSendMode`
<입력 파라미터> :
- 생성자: 없음 또는 사용자 인증 정보 및 ID.

<리턴 값> :
- 없음

<하위 단계들>
5.1. 생성자:
    - 멤버 변수 초기화.
    - 인증 정보 복사.
5.2. 소멸자:
    - 소켓 종료 및 WinSock 정리.
