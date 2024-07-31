from sqlalchemy import create_engine
import pandas as pd

def get_price_data(tickers, start_date, end_date):
    ticker_str = ', '.join(tickers)  # 티커를 SQL 쿼리에 맞는 형식으로 변환

    engine = create_engine('mysql+pymysql://root:1234@127.0.0.1:3306/stock_db')
    instruc_SQL =  f"select {ticker_str},Date from sample_etf_adj_close where Date between '{start_date}' and '{end_date}';"
    # 데이터 로드
    price = pd.read_sql(instruc_SQL, con=engine)
    price = price.set_index(['Date'])
    print("price")
    print(price)
    engine.dispose()

    return price