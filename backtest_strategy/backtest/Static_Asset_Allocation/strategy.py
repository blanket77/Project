# 글로벌 자산을 대표하는 ETF 데이터
import pandas as pd
from sqlalchemy import create_engine
import bt

# ETF 티커 리스트

# tikers = ['SPY', 'TLT', 'IEF', 'GLD', 'DBC']
# initial_capitals = 1000000
# rebalence = 'monthly'
# start_date = '2020-07-01'
# end_date = '2022-12-30'
class Static_Asset_Allocation():
    def __init__(self, tickers, initial_capitals, rebalence, start_date, end_date):
        self.tickers = tickers
        self.initial_capitals = initial_capitals
        self.rebalence = rebalence
        self.start_date = start_date
        self.end_date = end_date

    def display(self):

        ticker_str = ', '.join(self.tickers)  # 티커를 SQL 쿼리에 맞는 형식으로 변환

        # MySQL 데이터베이스 연결 설정
        engine = create_engine('mysql+pymysql://root:1234@127.0.0.1:3306/stock_db')

        instruc_SQL =  f"select {ticker_str},Date from sample_etf_adj_close where Date between '{self.start_date}' and '{self.end_date}';"
        # 데이터 로드
        price = pd.read_sql(instruc_SQL, con=engine)
        price = price.set_index(['Date'])
        engine.dispose()

        # 시그널 데이터프레임 초기화
        # select_w = pd.DataFrame(index=price.index, columns=price.columns, dtype=bool)
        # select_w.loc[:,:] = False
        # signal = {}
        # # 데이터 프레임 생성
        # signal_data = {
        #     'SPY': [True, False, False],
        #     'TLT': [False, True, True],
        #     'IEF': [False, True, True],
        #     'GLD': [True, False, False],
        #     'DBC': [True, False, True]
        # }

        # # 날짜 인덱스 설정
        # datess = ['2020-07-01', '2020-08-01', '2020-09-01']

        # # 데이터프레임 생성, 인덱스를 날짜로 설정
        # df = pd.DataFrame(signal_data, index=pd.to_datetime(datess))

        # # 가정: select_w와 df의 인덱스가 문자열 형태인 경우
        # select_w.index = pd.to_datetime(select_w.index)
        # df.index = pd.to_datetime(df.index)

        # # 같은 년도와 월을 기준으로 df 값을 select_w에 복사
        # for date in select_w.index:
        #     matching_dates = df.index[(df.index.year == date.year) & (df.index.month == date.month)]
        #     if not matching_dates.empty:
        #         # 해당하는 날짜가 있을 경우, df에서 해당 날짜의 데이터를 select_w의 해당 날짜에 할당
        #         select_w.loc[date] = df.loc[matching_dates[0]]

        # print(select_w)

        # NaN 값 제거
        data = price.dropna()

        # 전략 설정
        if self.rebalence == 'once':
            strategy = bt.Strategy("Asset_EW", [
                bt.algos.SelectAll(),
                bt.algos.WeighEqually(),
                bt.algos.RunOnce(),
                bt.algos.Rebalance()
            ])
        elif self.rebalence == 'weekly':
            strategy = bt.Strategy("Asset_EW", [
                bt.algos.SelectAll(),
                bt.algos.WeighEqually(),
                bt.algos.RunWeekly(),
                bt.algos.Rebalance()
            ])
        elif self.rebalence == 'monthly':
            strategy = bt.Strategy("Asset_EW", [
                bt.algos.SelectAll(),
                # bt.algos.SelectWhere(select_w),
                bt.algos.WeighEqually(),
                bt.algos.RunMonthly(),
                bt.algos.Rebalance()
            ])
        elif self.rebalence == 'daily':
            strategy = bt.Strategy("Asset_EW", [
                bt.algos.SelectAll(),
                bt.algos.WeighEqually(),
                bt.algos.RunDaily(),
                bt.algos.Rebalance()
            ])
        elif self.rebalence == 'yearly':
            strategy = bt.Strategy("Asset_EW", [
                bt.algos.SelectAll(),
                bt.algos.WeighEqually(),
                bt.algos.RunYearly(),
                bt.algos.Rebalance()
            ])
        elif self.rebalence == 'quarterly':
            strategy = bt.Strategy("Asset_EW", [
                bt.algos.SelectAll(),
                bt.algos.WeighEqually(),
                bt.algos.RunQuarterly(),
                bt.algos.Rebalance()
            ])
        else:
            raise ValueError("Invalid rebalence option")

        # 백테스트 실행
        backtest = bt.Backtest(strategy, data, initial_capital = self.initial_capitals)
        result = bt.run(backtest)

        # 수익률 출력
        print(result.display())

        # # 상위 디렉토리로 경로 이동
        # import sys
        # sys.path.append('..')  # 이 코드는 main.py가 위치한 subpackage 디렉토리에서 상위 디렉토리로 경로를 추가합니다.
        # # my_module 모듈을 임포트
        # import csv_print

        # csv_print.process_backtest_results(result, initial_capitals)