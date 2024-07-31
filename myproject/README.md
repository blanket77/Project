## 개요
이 프로젝트는 다양한 분산 투자 전략 및 정적 자산 배분과 관련된 백테스팅 및 데이터 분석 작업을 수행하기 위해 설계되었습니다.

## 설치 방법
필요한 종속성을 설치하려면 다음 명령어를 실행하십시오:
```bash
pip install -r requirements.txt


## 가상환경
myproject 작업환경에서 명령 프롬프트에서 아래와 같은 명령어를 입력한다.
py -3 -m venv .venv

설치 후 아래와 같은 명령어를 쳐서 가상환경에 접속한다.
.venv\Scripts\activate

## 데이터베이스
database폴더에 sql파일 있음
MYSQL Workbench에서 메뉴판에 있는 Server -> Data import로 해서 폴더 자체로 import를 시킨다. 만약 안 되면 파일 하나 하나 import 시킨다. schema 이름은 stock_db이다.

디렉터리 구조

MYPROJECT/
├── pycache/ # Python 바이트코드 캐시 디렉토리
├── .venv/ # 가상 환경
├── image/ # 이미지 파일 저장 디렉토리
├── Record/ # 백테스트 돌린 결과를 json 파일을 받아놓음
│ ├── day_graph.json # 일일 그래프 데이터 파일
│ ├── sorted_stock_adj.json # 해당 주식 매수하고 그 다음 날 매도하고 전략에서 특정 주식을 제거하고 다시 백테스트 돌린 후 매수, 매도 ROI 빈도 데이터 파일
│ ├── sorted_stock.json #  해당 주식 매수하고 그 다음 날 매도하고 전략으로 백테스트 돌린 후 매수, 매도 ROI 빈도 데이터 파일
│ ├── stock_history.json # 주식 히스토리 데이터 파일
│ ├── stock_history_adj.json # 해당 주식 매수하고 그 다음 날 매도하고 전략에서 특정 주식을 제거하고 백테스트를 돌린 주식 히스토리 데이터 파일
│ ├── stock_rate_adj.json # 해당 주식 매수하고 그 다음 날 매도하고 전략에서 특정 주식을 제거하고 다시 백테스트 돌린 후 주식 비율 데이터 파일
│ └── stock_rate.json #  해당 주식 매수하고 그 다음 날 매도하고 전략으로 백테스트 돌린 후 주식 비율 데이터 파일
│
├── routes/ # 애플리케이션의 라우트를 정의하는 디렉토리
│ ├── pycache/ # 바이트코드 캐시
│ ├── init.py # 초기화 파일
│ └── main.py # 주요 라우팅 로직
│
├── services/ # 애플리케이션에서 사용되는 서비스들을 제공하는 디렉토리
│ ├── pycache/ # 바이트코드 캐시
│ ├── backtests/ # 백테스팅 스크립트가 포함된 디렉토리
│ │ ├── pycache/ # 바이트코드 캐시
│ │ ├── init.py # 초기화 파일
│ │ ├── diversified_investment.py # 분산 투자 백테스팅 스크립트(아직 웹에 띄우게는 안 했음)
│ │ └── Static_Asset_Allocation.py # 정적 자산 배분 백테스팅 스크립트
│ ├── Client/ # 클라이언트 관련 스크립트가 포함된 디렉토리
│ │ ├── pycache/ # 바이트코드 캐시
│ │ ├── init.py # 초기화 파일
│ │ ├── adjust_day_run.py # 해당 주식 매수해서 그 다음 날 매도하는 전략에서 특정 종목을 빼고 다시 백테스트 실행
│ │ ├── day_buy_day_sell.py # 해당 주식 매수해서 그 다음 날 매도하는 전략
│ │ ├── day_run.py # 해당 주식 매수해서 그 다음 날 매도하는 전략
│ │ └── one_stock_plotly.py # Plotly를 사용한 개별 주식 시각화
│
├── static/ # 이미지, CSS, JavaScript와 같은 정적 파일
│ ├── backtest_report.html # 백테스트 보고서 HTML
│ ├── day_report_adj.html # 수정된 일일 보고서 HTML
│ ├── day_report.html # 일일 보고서 HTML
│ ├── report.html # 보고서 HTML
│ ├── stock_plot.html # 주식 그래프 HTML
│ ├── styles_upload.css # 업로드 스타일 CSS
│ └── styles.css # 스타일 CSS
│
├── templates/ # 웹 페이지를 렌더링하기 위한 HTML 템플릿
│ ├── adj_day.html # 해당 주식 매수해서 그 다음 날 매도하는 전략에서 특정 종목을 없애고 백테스트 돌린 결과 템플릿
│ ├── index.html # 메인 인덱스 템플릿
│ ├── plot.html # static_Asset_Allocation 백테스트한 결과를 그래프 템플릿
│ ├── stock_plot.html # 개별 주식 그래프 템플릿
│ └── upload.html # csv를 업로드해서 백테스트 결과가 나오는 템플릿 (전략은 해당 주식 매수한 그 다음 날 매도하는 전략)
│
├── uploads/ # 업로드된 파일들이 저장되는 디렉토리
│ ├── backtest_adj.csv # 수정된 백테스트 CSV
│ └── backtest.csv # 백테스트 CSV
│
├── .gitignore # Git 무시 파일 목록
├── app.py # 애플리케이션의 진입점
├── config.py # 설정 파일
├── README.md # README 파일
├── requirements.txt # 필요한 package와 각 버전들
└── run.py # 애플리케이션 실행 스크립트
