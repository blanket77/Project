from sqlalchemy import create_engine
import pandas as pd
import talib as ta
import bt

# 70 30
class rsi_mean_reversion():
    def __init__(self, tickers, initial_capitals, start_date, end_date, high_rsi, low_rsi):
        self.tickers = tickers
        self.initial_capitals = initial_capitals
        self.start_date = start_date
        self.end_date = end_date
        self.high_rsi = high_rsi
        self.low_rsi = low_rsi

    def display(self):
        ticker_str = ', '.join(self.tickers)  # 티커를 SQL 쿼리에 맞는 형식으로 변환

        engine = create_engine('mysql+pymysql://root:1234@127.0.0.1:3306/stock_db')
        instruc_SQL =  f"select {ticker_str},Date from sample_etf_adj_close where Date between '{self.start_date}' and '{self.end_date}';"
        # 데이터 로드
        price = pd.read_sql(instruc_SQL, con=engine)
        price = price.set_index(['Date'])
        engine.dispose()

        data = price[self.tickers].dropna()   ### 1.어떤 주식으로 할거냐
        spy_rsi = data.apply(lambda x: ta.RSI(x, 14))   ##2.RSI를 몇일 기준으로 할 거냐? 

        signal = spy_rsi.copy()

        signal[spy_rsi > self.high_rsi] = -1  ## 70 rsi 과매수 => 매도 
        signal[spy_rsi < self.low_rsi] = 1  ## 30 rsi 과매도 => 매수
        signal[(spy_rsi <= self.high_rsi) & (spy_rsi >= self.low_rsi)] = 0   ##이동평균선 정의에 의해서 처음부분은 삭제해야함
        signal[signal.isnull()] = 0


        strategy = bt.Strategy('RSI_MeanReversion',
                            [bt.algos.WeighTarget(signal),
                                bt.algos.Rebalance()
                            ])
        backtest = bt.Backtest(strategy,
                            data, initial_capital = self.initial_capitals
                            )

        result = bt.run(backtest)
        result.display()

        # # 상위 디렉토리로 경로 이동
        # import sys
        # sys.path.append('..')  # 이 코드는 main.py가 위치한 subpackage 디렉토리에서 상위 디렉토리로 경로를 추가합니다.
        # # my_module 모듈을 임포트
        # import csv_print

        # csv_print.process_backtest_results(result, initial_capitals)