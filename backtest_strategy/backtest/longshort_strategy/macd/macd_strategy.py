# 글로벌 자산을 대표하는 ETF 데이터
import pandas as pd
from sqlalchemy import create_engine
import bt
import talib as ta
import numpy as np

class macd_longshort():
    def __init__(self, tickers, initial_capitals, start_date, end_date, fastperiods, slowperiods, signalperiods, ohclv = 'close'):
        self.tickers = tickers
        self.initial_capitals = initial_capitals
        self.start_date = start_date
        self.end_date = end_date
        self.fastperiods = fastperiods
        self.slowperiods = slowperiods
        self.signalperiods = signalperiods
        self.ohclv = ohclv
        # fastperiods = 12
        # slowperiods = 26
        # signalperiods = 9
    def display(self):
        # select_w = {} # 데이터프레임으로 받아야한다.
        ticker_str = ', '.join(self.tickers)  # 티커를 SQL 쿼리에 맞는 형식으로 변환

        engine = create_engine('mysql+pymysql://root:1234@127.0.0.1:3306/stock_db')
        instruc_SQL =  f"select {ticker_str},Date from sample_etf_adj_close where Date between '{self.start_date}' and '{self.end_date}';"
        # 데이터 로드
        price = pd.read_sql(instruc_SQL, con=engine)
        price = price.set_index(['Date'])
        engine.dispose()

        # Assuming you have a DataFrame `price` loaded with necessary data.
        data = price[self.tickers]

        # Calculate MACD for each asset and create signals
        signals = pd.DataFrame(index=data.index)
        macds = {}

        
        for asset in tikers:
            # Calculate MACD components
            macd, macd_signal, macd_hist = ta.MACD(data[asset], fastperiod=self.fastperiods, slowperiod=self.slowperiods, signalperiod=self.signalperiods)
            macds[asset] = pd.DataFrame({
                'MACD': macd,
                'MACD_SIGNAL': macd_signal,
                'MACD_HIST': macd_hist
            })
            # Generate signals
            signals[asset] = np.where(macd > macd_signal, 1, np.where(macd < macd_signal, -1, 0))

        # Print the signals to review

        # Setup the strategy with bt
        strategy = bt.Strategy('MACD_Signal_Strategy',
                            [
                                bt.algos.WeighTarget(signals),
                                bt.algos.Rebalance()
                            ])

        backtest = bt.Backtest(strategy, data, initial_capital = self.initial_capitals)
        results = bt.run(backtest)

        # Display the results
        #results.plot()
        print(results.display())

        # # 상위 디렉토리로 경로 이동
        # import sys
        # sys.path.append('..')  # 이 코드는 main.py가 위치한 subpackage 디렉토리에서 상위 디렉토리로 경로를 추가합니다.
        # # my_module 모듈을 임포트
        # import csv_print

        # csv_print.process_backtest_results(result, initial_capitals)