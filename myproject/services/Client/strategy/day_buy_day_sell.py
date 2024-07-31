import pandas as pd
from sqlalchemy import create_engine

def get_date_to_symbols(csv_path): # csv 파일을 읽어서 날짜별 주식 기호를 반환합니다.
    origin_data = pd.read_csv(csv_path) # csv 파일을 읽어옵니다.
    origin_data['date'] = pd.to_datetime(origin_data['date']).dt.date.astype(str) # Date 컬럼을 날짜 형식으로 변환
    origin_data = origin_data.set_index(['date']) # 이걸로 인덱스 설정하겠다.
    date_to_symbols = origin_data.groupby('date')['ticker'].apply(list).to_dict() # {날짜: [주식 기호]}
    return date_to_symbols # {날짜: [주식 기호]}

# 주식 거래 전략(하루 사고 다음날 판다)
def trade_stocks(portfolio, date, symbols, period = 1):
 # 주식 데이터를 가져옵니다.
    # 날짜 범위 계산
    engine = create_engine('mysql+pymysql://root:1234@127.0.0.1:3306/stock_db') # 데이터베이스 접속
    stock_price = pd.read_sql('select * from lab_assignment;', con=engine) # lab_assignment 테이블에서 모든 데이터를 가져옵니다.
    stock_price['Date'] = pd.to_datetime(stock_price['Date']).dt.date.astype(str) # Date 컬럼을 날
    stock_price = stock_price.set_index(['Date']) # 이걸로 인덱스 설정하겠다.
    engine.dispose()

    start_date = date
    end_date = (pd.Timestamp(start_date) + pd.DateOffset(days=7)).strftime('%Y-%m-%d')  # 8일간의 데이터를 포함하려면 7일을 더해야 합니다.

    # 해당 날짜 범위에 있는 데이터 선택
    data = stock_price.loc[start_date:end_date] # 해당 날짜 범위에 있는 데이터 선택
    data = data.dropna() # 결측치 제거

    if data.empty:
        return "데이터를 불러오는데 실패했습니다. 날짜나 주식 기호를 확인해주세요." # 데이터가 비어있으면 에러 메시지를 반환합니다.
    
    # 첫날 종가를 가져옵니다.
    opening_prices = (data.iloc[0])[symbols] # 첫날 종가를 가져옵니다.
    
    # 각 주식에 할당할 자금을 계산합니다. 
    funds_per_stock = portfolio.cash_plus_stock / len(symbols)

    # 주식을 구매합니다.
    stocks_owned = {}
    for symbol, price in opening_prices.items():
        if price <= funds_per_stock: # 자금이 주식 가격보다 크거나 같으면
            stocks_owned[symbol] = funds_per_stock // price  # 소수 주식은 제외된다.
            
    # 다음 날 조정 종가를 가져와서 매도합니다.
    adj_closing_prices = data.iloc[period]

    #  매수합니다.
    for symbol, owned in stocks_owned.items():
        portfolio.buy_stock(opening_prices.name ,symbol, float(opening_prices[symbol]), owned)

    # 매도합니다.
    for symbol, owned in stocks_owned.items():
        portfolio.sell_stock(adj_closing_prices.name ,symbol, float(adj_closing_prices[symbol]), owned)