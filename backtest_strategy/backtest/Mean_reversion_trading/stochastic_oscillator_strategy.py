# 스토캐스틱 오실레이터
import bt
import pandas as pd
import numpy as np
import talib as ta
# 시그널 DataFrame을 초기화
from sqlalchemy import create_engine
import pandas as pd

#14,3,3
# fastk_periods = 14
# slowk_periods = 3
# slowd_periods = 3

class stochastic_oscillator:
    def __init__(self, tickers, initial_capitals, start_date, end_date, fastk_periods, slowk_periods, slowd_periods):
        self.tickers = tickers
        self.initial_capitals = initial_capitals
        self.start_date = start_date
        self.end_date = end_date
        self.fastk_periods = fastk_periods
        self.slowk_periods = slowk_periods
        self.slowd_periods = slowd_periods

    def display(self):
        start_date = '2020-07-01'
        end_date = '2022-12-30'
        ticker_str = ', '.join(self.tickers)  # 티커를 SQL 쿼리에 맞는 형식으로 변환

        engine = create_engine('mysql+pymysql://root:1234@127.0.0.1:3306/stock_db')
        instruc_SQL =  f"select {ticker_str},Date from sample_etf_high where Date between '{self.start_date}' and '{self.end_date}';"
        price_H = pd.read_sql(instruc_SQL, con=engine)
        price_H = price_H.set_index(['Date'])

        instruc_SQL =  f"select {ticker_str},Date from sample_etf_low where Date between '{start_date}' and '{end_date}';"
        price_L = pd.read_sql(instruc_SQL, con=engine)
        price_L = price_L.set_index(['Date'])

        instruc_SQL =  f"select {ticker_str},Date  from sample_etf_close where Date between '{start_date}' and '{end_date}';"
        price_C = pd.read_sql(instruc_SQL, con=engine)
        price_C = price_C.set_index(['Date'])

        instruc_SQL =  f"select {ticker_str},Date  from sample_etf_adj_close where Date between '{start_date}' and '{end_date}';"
        price_AC = pd.read_sql(instruc_SQL, con=engine)
        price_AC = price_AC.set_index(['Date'])
        engine.dispose()

        print(price_H)

        data_H = price_H[self.tickers].dropna()   ### 1.어떤 주식으로 할거냐
        data_L = price_L[self.tickers].dropna()
        data_C = price_C[self.tickers].dropna()
        data_AC = price_AC[self.tickers].dropna()

        signal = pd.DataFrame(index=data_H.index)

        # 스토캐스틱 오실레이터 계산 및 신호 생성
        assets = self.tickers
        for asset in assets:
            # 스토캐스틱 오실레이터 계산
            high, low, close = data_H[asset], data_L[asset], data_C[asset]
            k, d = ta.STOCH(high, low, close, fastk_period=self.fastk_periods, slowk_period=self.slowd_periods, slowk_matype=0, slowd_period=self.slowd_periods, slowd_matype=0)

            # 매수 신호: %K가 %D를 아래에서 위로 교차할 때
            # 매도 신호: %K가 %D를 위에서 아래로 교차할 때
            signal[asset] = np.where(k > d, 1, 0)
            signal[asset] = np.where(k < d, -1, signal[asset])

        # 백테스트 전략 생성
        strategy = bt.Strategy('Stochastic_Oscillator_Strategy', 
            [
                bt.algos.WeighTarget(signal),
                bt.algos.Rebalance()
            ])

        # 백테스트 실행
        backtest = bt.Backtest(strategy, data_AC)
        result = bt.run(backtest)

        # 결과 출력
        print(result.display())
