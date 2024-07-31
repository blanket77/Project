# 글로벌 자산을 대표하는 ETF 데이터
import bt
import talib as ta
from backtest.mysql import get_price_data

class sma_longshort():
    #14, 100, 30
    def __init__(self, tickers, initial_capitals, start_date, end_date, sma_period_long, sma_period_short):
        self.tickers = tickers
        self.initial_capitals = initial_capitals
        self.start_date = start_date
        self.end_date = end_date
        self.sma_period_long = sma_period_long
        self.sma_period_short = sma_period_short

    def display(self):
        # select_w = {} # 데이터프레임으로 받아야한다.
        price = get_price_data(self.tickers, self.start_date, self.end_date)

        data = price[self.tickers].dropna() # 3. 어떤 종목으로 할 건지 정해야함

        SMA_Long = data.apply(lambda x: ta.SMA(x, self.sma_period_long)) ## 1.장기 매개변수 값 설정
        SMA_Short = data.apply(lambda x: ta.SMA(x, self.sma_period_short))  ## 2.단기 매개변수 값 설정
        signal = SMA_Long.copy()
        signal[SMA_Short >= SMA_Long] = 1 # 골든크로스, 매수해야함 
        signal[SMA_Short < SMA_Long] = -1 # 데드코로스, 매도해야함
        signal[signal.isnull()] = 0  # 처음부분은 이동평균 정의에 의해 판단할 수 없다. NAN이 필요 
        print(signal)
        strategy = bt.Strategy(
            'SMA_crossover',
            [
            #bt.algos.SelectWhere(select_w), 
            bt.algos.WeighTarget(signal),
            bt.algos.Rebalance()]
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