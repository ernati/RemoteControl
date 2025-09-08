# Documentation for `Message.cpp`

## 1. `calculateCRC32` Function
<입력 파라미터> :
- `const uint8_t* data` — 체크섬을 계산할 데이터.
- `size_t length` — 데이터 길이.

<리턴 값> :
- `uint32_t` — 계산된 CRC32 체크섬 값.

<하위 단계들>
1.1. 초기화:
    - `crc`를 `0xFFFFFFFF`로 초기화.
1.2. 데이터 순회:
    - 각 바이트에 대해 XOR 연산 수행.
    - 각 비트에 대해 CRC 계산.
1.3. 결과 반환:
    - 계산된 CRC 값을 반전하여 반환.

===========================

## 2. `createMessageBuffer_tmp` Function
<입력 파라미터> :
- `const BITMAP& bmp` — 비트맵 구조체.
- `const uint8_t* bits` — 비트맵 데이터.
- `size_t bitsSize` — 비트맵 데이터 크기.
- `size_t& outTotal` — 생성된 메시지 버퍼 크기.

<리턴 값> :
- `uint8_t*` — 생성된 메시지 버퍼 (사용 후 `delete[]` 필요).

<하위 단계들>
2.1. 헤더 크기 계산:
    - `Message` 구조체 크기에서 가변 배열 멤버 크기를 제외한 값.
2.2. 전체 버퍼 크기 계산:
    - 헤더 크기와 비트맵 데이터 크기의 합.
2.3. 버퍼 메모리 할당:
    - `new`를 사용하여 버퍼 메모리 할당.
2.4. 헤더 필드 초기화:
    - `magic`, `width`, `height`, `planes`, `bitCount`, `widthBytes`, `payloadSize` 필드 설정.
2.5. 페이로드 복사:
    - 비트맵 데이터를 메시지 버퍼에 복사.
2.6. 체크섬 계산:
    - `calculateCRC32` 호출로 체크섬 계산 후 설정.
2.7. 버퍼 반환:
    - 생성된 메시지 버퍼 반환.
