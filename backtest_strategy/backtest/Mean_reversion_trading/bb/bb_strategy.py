import talib as ta
import pandas as pd
import numpy as np
from sqlalchemy import create_engine
import bt

#20 2 2
class bb_mean_reversion():
    def __init__(self, tickers, initial_capitals, start_date, end_date, timeperiod_input, nbdevup_input, nbdevdn_input):
        self.tickers = tickers
        self.initial_capitals = initial_capitals
        self.start_date = start_date
        self.end_date = end_date
        self.timeperiod_input = timeperiod_input
        self.nbdevup_input = nbdevup_input
        self.nbdevdn_input = nbdevdn_input

    def display(self):
        # select_w = {} # 데이터프레임으로 받아야한다.
        ticker_str = ', '.join(self.tickers)  # 티커를 SQL 쿼리에 맞는 형식으로 변환

        engine = create_engine('mysql+pymysql://root:1234@127.0.0.1:3306/stock_db')
        instruc_SQL =  f"select {ticker_str},Date from sample_etf_adj_close where Date between '{self.start_date}' and '{self.end_date}';"
        # 데이터 로드
        price = pd.read_sql(instruc_SQL, con=engine)
        price = price.set_index(['Date'])
        engine.dispose()

        #데이터 가져오기
        data = price[self.tickers].dropna()

        print(data)

        # 각각의 자산에 대해 볼린저밴드를 계산한다. Calculate Bollinger Bands for each asset
        bands = {}
        for asset in self.tickers:
            upperband, middleband, lowerband = ta.BBANDS(data[asset], timeperiod = self.timeperiod_input, nbdevup = self.nbdevup_input, nbdevdn = self.nbdevdn_input, matype=0)
            bands[asset] = pd.DataFrame({
                'Lower Band': lowerband,
                'Mid Band': middleband,
                'Upper Band': upperband,
                asset: data[asset]
            })

        # bb_SPY, bb_TLT, bb_IEF, bb_GLD, bb_DBC 틀을 만듬
        # 시그널 DataFrame을 초기화한다.
        signal = pd.DataFrame(index=data.index)

        # 각 자산에 대한 조건을 정의
        assets = self.tickers
        for asset in assets:
            bb = bands[asset]  
            signal[asset] = 0  # 각 시그널 0으로 초기화
            signal.loc[bb[asset] > bb['Upper Band'], asset] = -1  # 매도
            signal.loc[bb[asset] < bb['Lower Band'], asset] = 1   # 매수

        # 나머지 NAN 값들 0으로 처리 
        signal.fillna(0, inplace=True)

        strategy = bt.Strategy('BB',
                            [bt.algos.WeighTarget(signal),
                                bt.algos.Rebalance()
                            ]
                            )
        backtest = bt.Backtest(strategy, data, initial_capital = self.initial_capitals)
        result = bt.run(backtest)
        print(result.display())

        # # 상위 디렉토리로 경로 이동
        # import sys
        # sys.path.append('..')  # 이 코드는 main.py가 위치한 subpackage 디렉토리에서 상위 디렉토리로 경로를 추가합니다.
        # # my_module 모듈을 임포트
        # import csv_print

        # csv_print.process_backtest_results(result, initial_capitals)