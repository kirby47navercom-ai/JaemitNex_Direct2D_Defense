from __future__ import annotations

import html
import shutil
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUT_DIR = ROOT / "output" / "pdf"
TMP_DIR = ROOT / "tmp" / "pdfs" / "work_report"
PACKAGE_DIR = ROOT / "package"
PDF_NAME = "SpaceDefence_작업물_리포트.pdf"
GITHUB_URL = "https://github.com/kirby47navercom-ai/JaemitNex_Direct2D_Defense"
DEMO_VIDEO_URL = "https://www.youtube.com/watch?v=7fT5tFRw4E4"


def uri(path: Path) -> str:
    """HTML 안에서 로컬 파일을 안전하게 참조하기 위한 file URI를 만든다."""
    return path.resolve().as_uri()


def find_chrome() -> Path:
    """PDF 인쇄에 사용할 Chrome/Edge 실행 파일을 찾는다."""
    candidates = [
        Path(r"C:\Program Files\Google\Chrome\Application\chrome.exe"),
        Path(r"C:\Program Files (x86)\Google\Chrome\Application\chrome.exe"),
        Path(r"C:\Program Files\Microsoft\Edge\Application\msedge.exe"),
        Path(r"C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe"),
    ]
    for candidate in candidates:
        if candidate.exists():
            return candidate
    raise FileNotFoundError("Chrome 또는 Edge 실행 파일을 찾지 못했습니다.")


def e(text: str) -> str:
    return html.escape(text, quote=True)


def p(text: str) -> str:
    return f"<p>{e(text)}</p>"


def bullets(items: list[str]) -> str:
    return "<ul>" + "".join(f"<li>{e(item)}</li>" for item in items) + "</ul>"


def table(headers: list[str], rows: list[list[str]], class_name: str = "") -> str:
    head = "".join(f"<th>{e(h)}</th>" for h in headers)
    body = "\n".join("<tr>" + "".join(f"<td>{e(cell)}</td>" for cell in row) + "</tr>" for row in rows)
    return f"<table class='{class_name}'><thead><tr>{head}</tr></thead><tbody>{body}</tbody></table>"


def build_html() -> str:
    """제출 폼에 올릴 작업물 리포트 HTML을 만든다."""
    font_dir = ROOT / "assets" / "fonts"
    cover = ROOT / "package" / "SpaceDefence" / "assets" / "backgrounds" / "stage_09_space_hd.jpg"
    ingame = ROOT / "package" / "report_ingame_combat.png"

    feature_rows = [
        ["전투", "긴 전장, 카메라 스크롤, 자동 타깃팅, 근접/원거리 공격, 투사체, 기지 파괴"],
        ["성장", "LUMEN 보상, 유닛 구매, Lv.5 강화/진화, 월렛 업그레이드, 자동 저장"],
        ["스테이지", "수성부터 태양까지 10개 행성 스테이지와 행성별 기믹"],
        ["연출", "보스 컷인, 보스 체력바, 위험 범위 예고, 로컬 조명, 방향성 그림자, 이미지 VFX"],
        ["편의", "시작/옵션/상점/도감/브리핑/결과/ESC 메뉴, 볼륨 슬라이더, 화면 안전 여백"],
    ]
    system_rows = [
        ["편성", "출격 전 5개 유닛을 고르며 전략을 세움", "로스터 14종, 역할 라벨, 시너지 판정"],
        ["월렛", "전투 중 성장 선택지를 제공", "비용 감소, 쿨타임 감소, 체력/공격 보정, 보급 펄스"],
        ["행성 기믹", "스테이지마다 다른 대응을 요구", "열파, 산성 안개, 운석, 중력, 고리 지원군, 플레어"],
        ["보스", "후반 긴장감을 만드는 목표물", "1회 등장, 체력 구간 페이즈, 전용 패턴, 카메라 반응"],
        ["상점", "클리어 보상을 다음 도전에 연결", "유닛 구매, 강화, 성장 추천, 자동 저장"],
    ]
    process_rows = [
        ["1. 범위 설정", "과제 주제에 맞춰 실행 가능한 탑뷰 디펜스를 우선 목표로 정하고, 장르 구조만 참고한 오리지널 세계관으로 설계했습니다."],
        ["2. 기반 구현", "Win32 창, Direct2D 렌더 타깃, 델타타임 기반 게임 루프, 유닛 이동/공격/피해/보상 흐름을 먼저 완성했습니다."],
        ["3. 화면 흐름", "타이틀, 옵션, 스테이지 선택, 편성, 브리핑, 상점, 도감, 전투, 결과 화면을 연결했습니다."],
        ["4. 성장과 전략", "LUMEN 보상, 유닛 잠금/구매, Lv.5 강화, 월렛, 시너지, 난이도, AI 디렉터를 추가했습니다."],
        ["5. 연출 강화", "우주 배경, 행성 기믹, 보스 컷인, 로컬 조명, 방향성 그림자, 이미지 VFX, FMOD 효과음을 적용했습니다."],
        ["6. 정리와 제출", "파일을 Core/Combat/Input/Layout/Render로 분리하고, README/문서/빌드/패키지/QA를 정리했습니다."],
    ]
    tech_rows = [
        ["SpaceDefenceGame / Impl", "공개 래퍼와 내부 게임 상태를 분리한 pimpl 구조"],
        ["Core", "초기화, 리소스 로딩, 저장/로드, 프레임 업데이트"],
        ["Combat", "스폰, 이동, 타깃 탐색, 공격, 피해, 보스 패턴, VFX 수명 갱신"],
        ["Input", "키보드/마우스 입력, 화면별 버튼 판정, 슬라이더 조작"],
        ["Layout", "입력과 렌더가 공유하는 UI 좌표. 보스 체력바/전투 로그 겹침도 이 파일에서 해결"],
        ["Render", "Direct2D 전장, UI, 유닛, 무기, 이미지 VFX, 후처리식 화면 효과"],
        ["GameData", "유닛/적/스테이지 밸런스 테이블"],
        ["AudioManager", "FMOD가 있으면 위치음향, 없으면 WinMM 대체 재생"],
    ]
    ai_rows = [
        ["기획 보조", "라인 디펜스 구조, 행성 스테이지, 성장 요소, 보스전 아이디어를 빠르게 비교하고 정리하는 데 사용", "최종 장르 방향, 기능 우선순위, 과제 기간 안에 구현할 범위는 직접 결정"],
        ["코딩 보조", "Direct2D/Win32 코드 초안, 델타타임 루프, 파일 분리, 전투/VFX/입력 처리 구현을 보조", "실행해 보며 오류를 고치고, UI 위치와 전투 감각을 직접 확인하며 수정"],
        ["QA 보조", "글자 겹침, 화면 잘림, 보스 체력바와 전투 로그 충돌, 효과음 크기 같은 문제를 찾고 수정 방향을 제안받음", "실제 화면을 보고 무엇을 남기고 무엇을 빼야 하는지 판단"],
        ["문서 보조", "README, 코드 구조 설명, AI 사용 기록, 리포트 초안 정리에 사용", "최종 표현, 사람의 작업 비중, 라이선스/기술 사실은 직접 검증"],
    ]
    problem_rows = [
        ["특정 게임과 너무 비슷해지는 문제", "장르 구조만 참고하고 캐릭터명, 그래픽, 수치, 스테이지 규칙은 모두 오리지널로 재설계했습니다."],
        ["Direct2D만으로 화려한 화면을 만드는 문제", "외부 배경/효과음/VFX 에셋과 Direct2D 절차 이펙트를 섞고, HLSL 레퍼런스의 효과를 안전한 렌더 패스로 옮겼습니다."],
        ["전략성이 약해 보이는 문제", "편성, 시너지, 월렛, 상점 강화, 난이도, 행성 기믹, 보스 패턴으로 선택지를 늘렸습니다."],
        ["UI 글씨와 게이지가 겹치는 문제", "좌표를 Layout 파일로 모으고, 각 화면에서 실제 겹침을 확인하며 위치와 크기를 조정했습니다."],
        ["다른 PC에서 화면이 잘리는 문제", "가상 1280x800 화면을 창 크기에 맞춰 스케일링하고, 옵션에 화면 안전 여백을 추가했습니다."],
        ["효과음과 에셋 용량 관리", "CC0/OFL/NASA 출처를 문서화하고, 200MB 제한 안에 들어가도록 이미지/BGM 품질과 압축을 조정했습니다."],
        ["코드가 커지는 문제", "Core, Combat, Input, Layout, Render, GameData, framework 폴더로 역할을 나누고 주요 변수/함수에는 한국어 주석을 추가했습니다."],
    ]

    return f"""<!doctype html>
<html lang="ko">
<head>
<meta charset="utf-8" />
<title>Space Defence 작업물 리포트</title>
<style>
@font-face {{ font-family: GyeonggiTitle; src: url("{uri(font_dir / 'GyeonggiTitle_Medium.otf')}"); }}
@font-face {{ font-family: GyeonggiTitle; src: url("{uri(font_dir / 'GyeonggiTitle_Bold.otf')}"); font-weight: 700; }}
@font-face {{ font-family: GyeonggiBatang; src: url("{uri(font_dir / 'GyeonggiBatang_Regular.otf')}"); }}
@font-face {{ font-family: GyeonggiBatang; src: url("{uri(font_dir / 'GyeonggiBatang_Bold.otf')}"); font-weight: 700; }}
@font-face {{ font-family: Galmuri; src: url("{uri(font_dir / 'Galmuri11.ttf')}"); }}
@page {{ size: A4; margin: 15mm 16mm 16mm; }}
* {{ box-sizing: border-box; }}
body {{
  margin: 0;
  color: #132330;
  background: #f5f8fb;
  font-family: GyeonggiBatang, serif;
  font-size: 10.2pt;
  line-height: 1.58;
  -webkit-print-color-adjust: exact;
  print-color-adjust: exact;
}}
.page {{
  break-after: page;
  position: relative;
}}
.page:last-child {{ break-after: auto; }}
.cover {{
  width: 100%;
  min-height: 267mm;
  margin: -15mm -16mm -16mm;
  padding: 70mm 26mm 24mm;
  color: #f3fbff;
  background:
    linear-gradient(rgba(4, 9, 14, .72), rgba(4, 9, 14, .82)),
    url("{uri(cover)}") center / cover no-repeat;
}}
.cover-card {{
  max-width: 150mm;
  margin: 0 auto;
  padding: 22mm 18mm;
  border: 1.2pt solid rgba(126, 188, 232, .7);
  border-radius: 12mm;
  background: rgba(7, 19, 28, .72);
  box-shadow: 0 8mm 20mm rgba(0,0,0,.28);
  text-align: center;
}}
.kicker {{
  font-family: Galmuri, sans-serif;
  color: #b8ff89;
  letter-spacing: .08em;
  font-size: 9pt;
  margin-bottom: 7mm;
}}
h1, h2, h3 {{
  font-family: GyeonggiTitle, sans-serif;
  margin: 0;
}}
.cover h1 {{
  font-family: Galmuri, GyeonggiTitle, sans-serif;
  font-size: 31pt;
  line-height: 1.16;
  margin-bottom: 7mm;
  letter-spacing: .03em;
}}
.cover .sub {{
  font-family: GyeonggiTitle, sans-serif;
  font-size: 13.2pt;
  line-height: 1.55;
  color: #ddf1ff;
}}
.meta {{
  margin-top: 18mm;
  width: 100%;
  border-collapse: collapse;
  color: #eaf7ff;
}}
.meta th, .meta td {{
  border-bottom: 1px solid rgba(126,188,232,.25);
  padding: 2.4mm 1mm;
  background: rgba(6, 16, 25, .72) !important;
  color: #eaf7ff !important;
}}
.meta th {{
  width: 28mm;
  color: #b8ff89 !important;
  font-family: GyeonggiTitle, sans-serif;
  text-align: left;
}}
.section-title {{
  display: flex;
  align-items: center;
  gap: 4mm;
  margin: 1mm 0 5mm;
  padding-bottom: 2.5mm;
  border-bottom: 1.2pt solid #7ebce8;
  color: #143c5b;
}}
.section-title::before {{
  content: "";
  width: 6mm;
  height: 6mm;
  border-radius: 50%;
  background: #65b8ff;
  box-shadow: 0 0 0 2mm rgba(101,184,255,.12);
}}
h2 {{
  font-size: 18pt;
  line-height: 1.2;
}}
h3 {{
  color: #245f7c;
  font-size: 12.8pt;
  margin: 5mm 0 2mm;
}}
p {{
  margin: 0 0 3.2mm;
  word-break: keep-all;
}}
ul {{
  margin: 0 0 3mm 4mm;
  padding: 0;
}}
li {{
  margin: 0 0 1.2mm;
  padding-left: 1mm;
}}
table {{
  width: 100%;
  border-collapse: collapse;
  margin: 3mm 0 5mm;
  page-break-inside: avoid;
}}
th {{
  background: #143c5b;
  color: #f3fbff;
  font-family: GyeonggiTitle, sans-serif;
  font-weight: 700;
  font-size: 9.1pt;
}}
td, th {{
  border: 1px solid #c7d8e6;
  padding: 2.5mm 2.4mm;
  vertical-align: top;
}}
tbody tr:nth-child(even) td {{ background: #f7fbfe; }}
tbody tr:nth-child(odd) td {{ background: #ffffff; }}
.callout {{
  padding: 4mm 5mm;
  border: 1px solid #b8dfa7;
  border-radius: 4mm;
  background: #f2ffe9;
  color: #355f2d;
  font-family: GyeonggiBatang, serif;
  font-weight: 700;
  margin: 4mm 0 5mm;
}}
.grid-2 {{
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 5mm;
  margin: 5mm 0;
}}
.figure-wide {{
  margin: 5mm 0;
}}
.figure {{
  border: 1px solid #c7d8e6;
  border-radius: 4mm;
  overflow: hidden;
  background: #07131c;
}}
.figure img {{
  width: 100%;
  display: block;
  aspect-ratio: 16 / 9;
  object-fit: cover;
}}
.figure-wide img {{
  aspect-ratio: 16 / 10;
}}
.caption {{
  padding: 2.2mm 3mm;
  color: #eaf7ff;
  background: #0e2534;
  font-family: GyeonggiTitle, sans-serif;
  font-size: 9pt;
}}
.tagline {{
  color: #31546a;
  font-family: GyeonggiTitle, sans-serif;
  font-size: 10.6pt;
  margin-bottom: 4mm;
}}
.footer-note {{
  margin-top: 8mm;
  color: #637887;
  font-size: 8.4pt;
}}
code {{
  font-family: Galmuri, monospace;
  color: #1b5b88;
  font-size: 8.8pt;
}}
a {{
  color: #1b5b88;
  text-decoration: none;
  font-weight: 700;
}}
.todo-link {{
  color: #9b3a24;
  font-weight: 700;
}}
</style>
</head>
<body>
<section class="cover page">
  <div class="cover-card">
    <div class="kicker">JAEMITNEX AI NATIVE GAMEJAM</div>
    <h1>Space Defence</h1>
    <div class="sub">2026 넥슨 대학생 게임잼 '재밌넥'<br/>2차 과제 작업물 리포트</div>
    <table class="meta">
      <tr><th>장르</th><td>탑뷰 라인 디펜스 / 행성 탐사 전선</td></tr>
      <tr><th>기술</th><td>Win32 + Direct2D + DirectWrite + FMOD 선택 빌드</td></tr>
      <tr><th>제출물</th><td>실행 파일, 제출용 ZIP, 작업물 리포트 PDF</td></tr>
      <tr><th>목표</th><td>AI를 적극 활용해 기획, 구현, QA, 제출 문서까지 완성한 플레이 가능한 게임</td></tr>
    </table>
  </div>
</section>

<section class="page">
  <div class="section-title"><h2>1. 결과물 설명</h2></div>
  {p("Space Defence는 수성에서 태양까지 이어지는 전선을 배경으로 한 탑뷰 라인 디펜스 게임입니다. 플레이어는 14종 유닛 중 5종을 편성하고, 전투 중 에너지를 소비해 유닛을 소환하며, 월렛과 문빔 캐논을 활용해 적 기지를 밀어냅니다. 유명 유닛 소환형 디펜스의 장르 구조는 참고했지만, 캐릭터명, 그래픽, 밸런스, 스테이지 기믹은 모두 새로 구성했습니다.")}
  {table(["구분", "구현 내용"], feature_rows)}
  <div class="callout">최종 패키지는 <code>package/SpaceDefence/SpaceDefence.exe</code>로 바로 실행할 수 있고, 제출용 압축 파일은 <code>package/SpaceDefence.zip</code>입니다. 고화질 우주 배경, 컷신, BGM, 효과음, VFX 시트를 포함하면서도 200MB 제출 제한 안에 들어가도록 관리했습니다.</div>
  <div class="figure figure-wide">
    <img src="{uri(ingame)}" />
    <div class="caption">최신 빌드에서 직접 캡처한 인게임 전투 화면 - 유닛 카드, 월렛, 카메라 HUD, 전투 로그, 데모 모드 표시</div>
  </div>
</section>

<section class="page">
  <div class="section-title"><h2>2. 핵심 게임 시스템</h2></div>
  {table(["시스템", "플레이어가 체감하는 의미", "구현 포인트"], system_rows)}
  <h3>유닛과 적 구성</h3>
  {bullets([
      "아군 유닛: 클로, 가드, 썬더, 대시, 벨, 타이탄, 프로스트, 코멧, 오빗, 솔라, 민트, 드릴, 프리즘, 네뷸라",
      "적 유닛: 더스트, 브루트, 스키터, 설퍼, 모스, 러스트, 스톰, 링, 프로스트, 타이드, 보이드, 플레어, 스포어, 퀘이크, 미러, 코멧, 솔라 보스",
      "브리핑 화면은 편성 전력과 스테이지 위협도를 비교해 난이도와 상점 강화를 판단할 수 있게 합니다.",
      "전투 화면에는 카메라 HUD, 전투 로그, 보스 체력바, 커맨드 패널, 월렛/캐논 버튼을 배치했습니다.",
  ])}
  <h3>전략성 보강</h3>
  {p("초기에는 유닛을 계속 뽑아 전선을 미는 구조가 단순해 보일 수 있었습니다. 그래서 편성 시너지, 월렛 성장, 난이도 선택, 스테이지 기믹, 위험 범위 예고, 상점 성장 추천을 추가해 전투 전 선택과 전투 중 판단이 모두 의미를 갖도록 만들었습니다.")}
</section>

<section class="page">
  <div class="section-title"><h2>3. 제작 과정</h2></div>
  {table(["단계", "작업 내용"], process_rows)}
  {p("제작 중에는 작은 기능을 추가할 때마다 실제 화면에서 글자 겹침, 버튼 판정, 전체 화면 잘림, 효과음 크기, 보스 난이도, 패키지 용량을 반복 확인했습니다. 마지막에는 게임명이 Space Defence로 통일되도록 코드, 문서, 패키지명을 다시 검색해 정리했습니다.")}
  <h3>제작 흐름 요약</h3>
  {bullets([
      "먼저 실행 가능한 전투 루프를 만들고, 이후 UI와 성장 구조를 붙였습니다.",
      "한 파일에 커지던 구현을 Core, Combat, Input, Layout, Render로 분리했습니다.",
      "행성별 배경과 기믹, 보스 패턴, VFX, 효과음, BGM을 단계적으로 강화했습니다.",
      "마지막에는 README, AI 사용 기록, 밸런스 문서, 코드 구조 문서, 제출용 ZIP을 최신화했습니다.",
  ])}
</section>

<section class="page">
  <div class="section-title"><h2>4. 기술 구조</h2></div>
  {table(["파일", "역할"], tech_rows)}
  <h3>델타타임과 프레임워크</h3>
  {bullets([
      "프레임 업데이트는 DeltaTimer의 실제 경과 시간을 기준으로 하고, 게임 속도 배율은 델타타임에 곱해 적용했습니다.",
      "SceneManager와 AudioManager를 framework 폴더에 두어 이후 화면 단위 분리가 쉬운 구조로 정리했습니다.",
      "입력 판정과 렌더링 좌표가 어긋나지 않도록 UI 사각형은 Layout 파일에 모았습니다.",
  ])}
  <h3>VFX와 파티클</h3>
  {p("파티클은 CPU에서 위치와 수명만 관리하고 Direct2D GPU 가속 렌더링으로 그립니다. GPU compute shader로 시뮬레이션하는 구조는 아니지만, 파티클, 링, 빔, 스파크, 이미지 VFX마다 최대 개수를 제한해 장시간 전투에서도 누적 렉이 생기지 않도록 했습니다. HLSL 파일은 셰이더 레퍼런스로 두고, 실제 제출 빌드에는 동일한 의도의 스캔라인, 비네트, 하프톤, 로컬 라이트를 Direct2D 절차 렌더링으로 구현했습니다.")}
</section>

<section class="page">
  <div class="section-title"><h2>5. 사용한 AI와 용도</h2></div>
  {table(["사용 AI", "사용 용도", "사람이 판단한 부분"], ai_rows)}
  <div class="callout">AI는 제작 속도를 높이는 보조 도구로 사용했습니다. 어떤 기능을 넣을지, 어떤 기능을 줄일지, 실제 화면에서 무엇이 어색한지, 제출 안정성을 위해 어느 정도에서 멈출지는 직접 판단했습니다.</div>
  <h3>직접 맡은 작업</h3>
  {bullets([
      "게임 방향과 기능 우선순위를 정하고, 구현 가능한 범위로 계속 조정했습니다.",
      "빌드된 게임을 직접 실행하며 UI 겹침, 화면 잘림, 전투 로그와 보스 체력바 충돌 같은 문제를 확인했습니다.",
      "유닛명, 스테이지명, 제출 파일명, README 내용이 서로 맞는지 반복해서 정리했습니다.",
      "외부 에셋 라이선스, 패키지 용량, 실행 가능 여부, 제출 폼 요구사항을 직접 확인했습니다.",
  ])}
  <h3>AI가 도와준 작업</h3>
  {bullets([
      "C++/Direct2D 코드 초안 작성과 오류 수정 방향 제안",
      "행성별 기믹, 보스 패턴, 유닛 역할, 밸런스 표 정리",
      "문서 초안 작성, QA 체크리스트 정리, 코드 구조 설명 보조",
  ])}
</section>

<section class="page">
  <div class="section-title"><h2>6. 어려웠던 점과 해결 과정</h2></div>
  {table(["어려웠던 점", "해결 방식"], problem_rows)}
  <h3>최근 QA 예시</h3>
  {p("전투 중 보스 체력바와 전투 로그가 같은 상단 영역에 떠서 겹치는 문제가 있었습니다. 이 문제는 보스가 살아 있을 때 MessageToastRect의 y 좌표를 보스 체력바 아래로 자동 이동시키는 방식으로 수정했습니다. 이처럼 UI 좌표를 Layout 파일에서 관리해 입력과 표시가 함께 고쳐지도록 했습니다.")}
</section>

<section class="page">
  <div class="section-title"><h2>7. QA와 제출 준비</h2></div>
  {bullets([
      "build.ps1로 실행 파일과 제출용 ZIP을 자동 생성합니다.",
      "FMOD SDK가 있으면 FMOD 빌드로 효과음과 위치음향을 사용하고, 없으면 WinMM 대체 경로로 실행됩니다.",
      "최신 패키지 실행 파일은 실행 후 바로 종료되지 않는지 스모크 테스트했습니다.",
      "README, 코드 구조 문서, AI 사용 기록, 밸런스 문서, 제출 체크리스트를 최신 이름과 유닛명 기준으로 정리했습니다.",
      "패키지 용량은 200MB 제한 안에 들어가도록 유지했습니다.",
  ])}
  {table(["제출 항목", "파일"], [
      ["제작 결과물 ZIP", "package/SpaceDefence.zip"],
      ["실행 파일", "package/SpaceDefence/SpaceDefence.exe"],
      ["작업물 리포트", f"output/pdf/{PDF_NAME}"],
      ["주요 문서", "README.md, docs/AI_USAGE.md, docs/CODE_STRUCTURE_KO.md, docs/BALANCE_TABLES.md"],
  ])}
  <h3>외부 확인 링크</h3>
  <table>
    <thead><tr><th>항목</th><th>링크</th></tr></thead>
    <tbody>
      <tr><td>GitHub 저장소</td><td><a href="{e(GITHUB_URL)}">{e(GITHUB_URL)}</a></td></tr>
      <tr><td>시연 영상</td><td><a href="{e(DEMO_VIDEO_URL)}">{e(DEMO_VIDEO_URL)}</a></td></tr>
    </tbody>
  </table>
  <h3>마무리</h3>
  {p("Space Defence는 짧은 과제 기간 안에서 AI를 적극 활용해 기획, 구현, 연출, QA, 문서화를 모두 이어 붙인 결과물입니다. 완성형 상용 게임보다는 게임잼 과제에 맞춘 프로토타입이지만, 실행 가능한 빌드, 성장 구조, 여러 스테이지, 보스전, 사운드, 저장, 옵션, 제출 문서까지 갖추는 것을 목표로 했습니다.")}
  {p("가장 중요하게 본 기준은 'AI가 만든 코드'가 아니라, AI를 활용해 어떤 판단을 하고 어떤 부분을 직접 고쳐 나갔는지였습니다. 그래서 반복적인 UI 수정, 밸런스 조정, 에셋 교체, 파일 분리, 문서 최신화까지 제작 과정 전체를 남겼습니다.")}
  <p class="footer-note">작성자: 양현빈 / 프로젝트: Space Defence / 문서 파일: {e(PDF_NAME)}</p>
</section>
</body>
</html>"""


def build_pdf() -> Path:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    TMP_DIR.mkdir(parents=True, exist_ok=True)
    PACKAGE_DIR.mkdir(parents=True, exist_ok=True)

    html_path = TMP_DIR / "SpaceDefence_작업물_리포트.html"
    pdf_path = OUT_DIR / PDF_NAME
    package_pdf = PACKAGE_DIR / PDF_NAME
    html_path.write_text(build_html(), encoding="utf-8")

    chrome = find_chrome()
    user_data_dir = TMP_DIR / "chrome_profile"
    if user_data_dir.exists():
        shutil.rmtree(user_data_dir)
    user_data_dir.mkdir(parents=True, exist_ok=True)

    command = [
        str(chrome),
        "--headless=new",
        "--disable-gpu",
        "--allow-file-access-from-files",
        "--no-pdf-header-footer",
        f"--user-data-dir={user_data_dir}",
        f"--print-to-pdf={pdf_path}",
        str(html_path.resolve().as_uri()),
    ]
    subprocess.run(command, check=True, timeout=90)
    shutil.copy2(pdf_path, package_pdf)
    print(pdf_path)
    return pdf_path


if __name__ == "__main__":
    build_pdf()
