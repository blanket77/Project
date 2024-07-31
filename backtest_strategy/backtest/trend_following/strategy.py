# 글로벌 자산을 대표하는 ETF 데이터
import pandas as pd
from sqlalchemy import create_engine
import talib as ta
import bt

# sma_period = 50 
class trend_following():
    def __init__(self, tickers, initial_capitals, start_date, end_date, sma_period):
        self.tickers = tickers
        self.initial_capitals = initial_capitals
        self.start_date = start_date
        self.end_date = end_date
        self.sma_period = sma_period

    def display(self):
        ticker_str = ', '.join(self.tickers)  # 티커를 SQL 쿼리에 맞는 형식으로 변환

        select_w = {} # 데이터프레임으로 받아야한다.

        engine = create_engine('mysql+pymysql://root:1234@127.0.0.1:3306/stock_db')
        instruc_SQL =  f"select {ticker_str},Date from sample_etf_adj_close where Date between '{self.start_date}' and '{self.end_date}';"
        # 데이터 로드
        price = pd.read_sql(instruc_SQL, con=engine)
        price = price.set_index(['Date'])
        engine.dispose()

        # SMA

        ###############################
        ########paramater##############
        # sma_period = 50 

        data = price[self.tickers].dropna() #어떤 데이터로 백테스트를 할 거냐
        sma = data.apply(lambda x: ta.SMA(x, self.sma_period)) # 200을 변수로 받아야한다.

        bt_sma = bt.Strategy('Timing', [
            bt.algos.SelectWhere(data > sma),
            bt.algos.WeighEqually(),  # 100% 투자한다
            bt.algos.Rebalance()
        ])

        bt_sma_backtest = bt.Backtest(bt_sma, data, initial_capital = self.initial_capitals)

        def buy_and_hold(data, name):

            # 벤치마크 전략 생성
            bt_strategy = bt.Strategy(name, [     
                #bt.algos.SelectWhere(select_w),   
                bt.algos.SelectAll(),
                bt.algos.WeighEqually(), 
                bt.algos.RunOnce(),
                bt.algos.Rebalance()
            ])
            # Return the backtest
            return bt.Backtest(bt_strategy, data)


        # 벤치마크 전략 백테스트
        stock = buy_and_hold(data[self.tickers], name='stock') 

        # 두개 백테스트 동시에 실행
        # bt_sma_result = bt.run(bt_sma_backtest, stock)
        bt_sma_result = bt.run(bt_sma_backtest)
        print(bt_sma_result.display())

        # # 상위 디렉토리로 경로 이동
        # import sys
        # sys.path.append('..')  # 이 코드는 main.py가 위치한 subpackage 디렉토리에서 상위 디렉토리로 경로를 추가합니다.
        # # my_module 모듈을 임포트
        # import csv_print

        # csv_print.process_backtest_results(bt_sma_result, initial_capitals)