# Documentation for `Inc` Directory

## 1. `CCaptureScreenAndSendBitMap.h`
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
    - GDI 리소스 해제.
1.3. 주요 메서드:
    - `CaptureScreen`: 화면 캡처 및 비트맵 반환.
    - `UpdateNewBitmap`: 새로운 비트맵 업데이트.
    - `CheckSuccessCapture`: 캡처 성공 여부 확인.

===========================

## 2. `CPrintScreen.h`
### Class: `CPrintScreen`
<입력 파라미터> :
- 생성자: 없음 또는 `HINSTANCE hInstance` — 애플리케이션 인스턴스 핸들.

<리턴 값> :
- 없음

<하위 단계들>
2.1. 생성자:
    - 멤버 변수 초기화.
    - 화면 크기 기본값 설정.
2.2. 주요 메서드:
    - `setCaptureScreen`: 캡처 객체 설정.
    - `sethInstance`: 인스턴스 핸들 설정.

===========================

## 3. `CRemoteControlRecvMode.h`
### Class: `CRemoteControlRecvMode`
<입력 파라미터> :
- 생성자: 없음 또는 사용자 인증 정보 및 ID.

<리턴 값> :
- 없음

<하위 단계들>
3.1. 생성자:
    - 멤버 변수 초기화.
    - 인증 정보 복사.
3.2. 주요 메서드:
    - `StartClient`: 클라이언트 시작 및 서버 연결.
    - `Communication`: 데이터 통신 처리.
    - `ReconstructBitmapFromMessage`: 메시지로부터 비트맵 재구성.

===========================

## 4. `CRemoteControlSendMode.h`
### Class: `CRemoteControlSendMode`
<입력 파라미터> :
- 생성자: 없음 또는 사용자 인증 정보 및 ID.

<리턴 값> :
- 없음

<하위 단계들>
4.1. 생성자:
    - 멤버 변수 초기화.
    - 인증 정보 복사.
4.2. 주요 메서드:
    - `StartClient`: 클라이언트 시작 및 서버 연결.
    - `Communication`: 데이터 통신 처리.
    - `PerformHandshake`: 핸드셰이크 수행.
