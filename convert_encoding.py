import os
import chardet

ROOT = r"E:\UE5\RWG\Source"
TARGET_EXT = {'.cpp', '.h'}

for dirpath, _, filenames in os.walk(ROOT):
    for fname in filenames:
        if os.path.splitext(fname)[1] not in TARGET_EXT:
            continue
        fpath = os.path.join(dirpath, fname)
        with open(fpath, 'rb') as f:
            raw = f.read()

        detected = chardet.detect(raw)
        enc = detected['encoding']
        conf = detected['confidence']

        # 이미 UTF-8이면 스킵
        if enc and enc.lower() in ('utf-8', 'ascii'):
            continue

        print(f"[{conf:.2f}] {fpath} -> detected: {enc}")

        try:
            # CP949/EUC-KR로 디코딩 시도
            text = raw.decode('cp949')
        except UnicodeDecodeError:
            print(f"  !! cp949 디코딩 실패, 수동 확인 필요: {fpath}")
            continue

        with open(fpath, 'w', encoding='utf-8') as f:
            f.write(text)
        print(f"  -> UTF-8로 변환 완료")