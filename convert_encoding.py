import os

ROOT = r"E:\UE5\RWG\Source"
TARGET_EXT = {'.cpp', '.h'}

count = 0
for dirpath, _, filenames in os.walk(ROOT):
    for fname in filenames:
        if os.path.splitext(fname)[1] not in TARGET_EXT:
            continue
        fpath = os.path.join(dirpath, fname)

        with open(fpath, 'rb') as f:
            raw = f.read()

        # 이미 UTF-8이므로 그대로 디코딩
        try:
            text = raw.decode('utf-8-sig')  # BOM 있으면 제거, 없으면 그냥 utf-8로 처리됨
        except UnicodeDecodeError:
            print(f"  !! UTF-8 디코딩 실패: {fpath}")
            continue

        # 모든 줄바꿈을 \n으로 통일
        normalized = text.replace('\r\n', '\n').replace('\r', '\n')

        # CRLF(Windows 표준)로 다시 통일해서 저장
        final_text = normalized.replace('\n', '\r\n')

        with open(fpath, 'wb') as f:
            f.write(final_text.encode('utf-8'))  # BOM 없이 순수 UTF-8로 저장

        count += 1

print(f"총 {count}개 파일 줄바꿈 정규화 완료")