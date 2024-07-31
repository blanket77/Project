import pandas as pd
from sqlalchemy import create_engine
import matplotlib.pyplot as plt
import numpy as np
import quantstats as qs
import plotly.express as px

# 데이터 불러오기
origin_data = pd.read_csv("../../uploads/backtest.csv") # csv 파일을 읽어옵니다.
origin_data['date'] = pd.to_datetime(origin_data['date']).dt.date.astype(str) # Date 컬럼을 날짜 형식으로 변환
origin_data = origin_data.set_index(['date']) # 이걸로 인덱스 설정하겠다.
date_to_symbols = origin_data.groupby('date')['ticker'].apply(list).to_dict() # {날짜: [주식 기호]}

# MySQL 데이터베이스 연결 설정 
engine = create_engine('mysql+pymysql://root:1234@127.0.0.1:3306/stock_db') # 데이터베이스 접속
stock_price = pd.read_sql('select * from lab_assignment2_open;', con=engine) # lab_assignment 테이블에서 모든 데이터를 가져옵니다.
stock_price['Date'] = pd.to_datetime(stock_price['Date']).dt.date.astype(str) # Date 컬럼을 날짜 형식으로 변환
stock_price = stock_price.set_index(['Date']) # 이걸로 인덱스 설정하겠다.
engine.dispose() # 데이터베이스 접속 종료

# 주식 데이터가 없는 날짜에 대한 딕셔너리 생성
for date in stock_price.index: # 주식 데이터가 없는 날짜에 대한 딕셔너리 생성
    if date not in date_to_symbols.keys(): # 주식 데이터가 없는 날짜에 대한 딕셔너리 생성
        date_to_symbols[date] = [] # 주식 데이터가 없는 날짜에 대한 딕셔너리 생성

# 날짜를 기준으로 주식 기호를 정렬
date_to_symbols = {date: date_to_symbols[date] for date in sorted(date_to_symbols)}

i = 0 
tax = 0.001

# 결과를 저장할 딕셔너리 생성
Cumulative_Return = {'Date': [], 'Price':[], 'Daily_Rerutn':[], 'Cumulative Return':[]} # 결과를 저장할 딕셔너리 생성
initial_f = 100000 # 초기 자금

initial_f_array = [initial_f/3, initial_f/3, initial_f/3] # 초기 자금을 3등분하여 저장

# 주식을 매매하는 함수
def trade_stocks(date, symbols, initial_f_array, i): # 주식을 매매하는 함수
    if not symbols: # 주식 기호가 없으면
        return
     
    initial_funds = initial_f_array[i] # 초기 자금
    # 주식 데이터를 가져옵니다.
    start_date = date # 시작 날짜
    end_date = (pd.Timestamp(start_date) + pd.DateOffset(days=9)).strftime('%Y-%m-%d')  # 8일간의 데이터를 포함하려면 7일을 더해야 합니다.

    # 해당 날짜 범위에 있는 데이터 선택
    data = stock_price.loc[start_date:end_date] # 해당 날짜 범위에 있는 데이터 선택
    data = data.dropna() # 결측치 제거

    if data.empty: # 데이터가 비어있으면
        return "데이터를 불러오는데 실패했습니다. 날짜나 주식 기호를 확인해주세요."
    
    # 첫째 날 조정 종가를 가져옵니다.
    buy_adj_prices = (data.iloc[0])[symbols]

    # 각 주식에 할당할 자금을 계산합니다.
    funds_per_stock = initial_funds / len(symbols)
    
    # 주식을 구매합니다. (주식 : 수량)
    stocks_owned = {}
    for symbol, price in buy_adj_prices.items(): # 주식을 구매합니다.
        if price <= funds_per_stock: # 자금이 주식 가격보다 크거나 같으면
            stocks_owned[symbol] = int(funds_per_stock // price)  # 소수 주식은 제외된다.
    # print("stocks_owned")
    # print(stocks_owned) 

    # 셋째 날 조정 종가를 가져와서 매도합니다.
    try:
        adj_closing_prices = data.iloc[3]
    except IndexError:
        # tmp_fund = sum(initial_f_array)
        # initial_f_array[i] = initial_funds
        # result_tmp_fund = sum(initial_f_array)

        # Cumulative_Return['Date'].append(data.index[0])
        # Cumulative_Return['Price'].append(result_tmp_fund)
        # Cumulative_Return['Daily_Rerutn'].append((result_tmp_fund-tmp_fund)/tmp_fund*100)
        # Cumulative_Return['Cumulative Return'].append((result_tmp_fund-initial_f)/initial_f*100)
        return

    tmp_fund = sum(initial_f_array) # 임시 자금
    # 최종 자금을 계산합니다.
    for symbol, owned in stocks_owned.items(): # 최종 자금을 계산합니다.
        initial_funds -= owned * buy_adj_prices[symbol]  # 구매한 주식 가격만큼 자금을 차감합니다.
        initial_funds += owned * adj_closing_prices[symbol] * (1 - tax) # 매도할 때 세금을 제외합니다.
    
    initial_f_array[i] = initial_funds # 초기 자금을 업데이트합니다.
    result_tmp_fund = sum(initial_f_array) # 최종 자금을 계산합니다.

    Cumulative_Return['Date'].append(data.index[3])  # 날짜 추가
    Cumulative_Return['Price'].append(result_tmp_fund) # 가격 추가
    Cumulative_Return['Daily_Rerutn'].append((result_tmp_fund-tmp_fund)/tmp_fund*100) # 일일 수익률 추가
    Cumulative_Return['Cumulative Return'].append((result_tmp_fund-initial_f)/initial_f*100) # 누적 수익률 추가

    return 

print(date_to_symbols) # 날짜별 주식 기호 출력
initial_funds = initial_f # 초기 자금
 
for date, symbols in date_to_symbols.items(): # 날짜별로 반복   
    # print(date) # 날짜 출력
    trade_stocks(date, symbols, initial_f_array, i) # 주식을 매매하는 함수 호출
    i = i+1     
    if i > 2:
        i = 0

result = sum(initial_f_array) # 최종 자금

print(f"처음 자금: ${initial_funds:.2f}") # 처음 자금 출력
print(f"최종 자금: ${result:.2f}") # 최종 자금 출력
print(f"수익률: {(result-initial_funds)/initial_funds*100:.2f}%") # 수익률 출력

# 'Price' 리스트에서 MDD 계산
prices = np.array(Cumulative_Return['Price']) # 가격 데이터를 numpy 배열로 변환
peak_prices = np.maximum.accumulate(prices)  # 각 시점까지의 최대 포트폴리오 가치
drawdowns = (prices - peak_prices) / peak_prices  # 각 시점에서의 drawdown 계산
max_drawdown = drawdowns.min()  # 최대 drawdown

print(f"최대 손실율 (MDD): {max_drawdown * 100:.2f}%") # 최대 손실율 출력

# DataFrame 생성
df = pd.DataFrame(Cumulative_Return)
df['Date'] = pd.to_datetime(df['Date'])  # 날짜 형식으로 변환
df = df.set_index('Date')  # 인덱스를 날짜로 설정

# Debugging: Print the DataFrame
print(df)

# 0인 값을 NaN으로 대체하고 제거
df['Price'] = df['Price'].replace(0, np.nan).dropna()

# 수익률 계산
prices = df['Price']
# 인덱스를 datetime 형식으로 변환
prices.index.name = None
# 추가할 데이터
additional_data = pd.Series({"2024-05-13": 100000})
# 추가할 데이터의 인덱스를 datetime 형식으로 변환
additional_data.index = pd.to_datetime(additional_data.index)

# 기존 데이터에 추가
prices = pd.concat([pd.Series(additional_data), prices])

returns = prices.pct_change().dropna()  # 첫 번째 NaN 값을 제거

# QuantStats를 사용해 수익률 계산
report = qs.reports.metrics(returns)

# Plotly Express를 사용해 선 그래프 생성
fig = px.line(df, x=df.index, y='Cumulative Return', title='Cumulative Return')

# 그래프 표시
fig.show()

# Plotly Express를 사용해 선 그래프 생성
fig = px.line(df, x=df.index, y='Daily_Rerutn', title='Daily Returns Over Time')

# 그래프 표시
fig.show()
