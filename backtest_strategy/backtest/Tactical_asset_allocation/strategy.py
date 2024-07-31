from sqlalchemy import create_engine
import pandas as pd
import bt

class Tactical_asset_allocation():
    def __init__(self, tickers, initial_capitals, rebalence, start_date, end_date, n_s, year_s):
        self.tickers = tickers
        self.initial_capitals = initial_capitals
        self.rebalence = rebalence
        self.start_date = start_date
        self.end_date = end_date
        self.n_s = n_s
        self.year_s = year_s

    def display(self):

        ticker_str = ', '.join(self.tickers)  # 티커를 SQL 쿼리에 맞는 형식으로 변환

        # MySQL 데이터베이스 연결 설정
        engine = create_engine('mysql+pymysql://root:1234@127.0.0.1:3306/stock_db')

        instruc_SQL =  f"select {ticker_str},Date from sample_etf_adj_close where Date between '{self.start_date}' and '{self.end_date}';"
        # 데이터 로드
        price = pd.read_sql(instruc_SQL, con=engine)
        price = price.set_index(['Date'])
        engine.dispose()

        # NaN 값 제거
        data = price.dropna()

        if self.rebalence == 'once':
            gdaa  = bt.Strategy("GDAA", [
                bt.algos.SelectAll(),
                bt.algos.SelectMomentum(n=self.n_s, lookback=pd.DateOffset(years=self.year_s)),
                # bt.algos.SelectWhere(selecw_h);
                #bt.algos.WeighERC(lookback=pd.DateOffset(years=1)),
                bt.algos.RunOnce(),
                bt.algos.Rebalance()
            ])
        elif self.rebalence == 'weekly':
            gdaa = bt.Strategy("GDAA", [
                bt.algos.SelectAll(),
                bt.algos.SelectMomentum(n=self.n_s, lookback=pd.DateOffset(years=self.year_s)),
                #bt.algos.WeighERC(lookback=pd.DateOffset(years=1)),
                bt.algos.RunWeekly(),
                bt.algos.Rebalance()
            ])
        elif self.rebalence == 'monthly':
            gdaa = bt.Strategy('GDAA', [
                bt.algos.SelectAll(),
                bt.algos.SelectMomentum(n=self.n_s, lookback=pd.DateOffset(years=self.year_s)),
                bt.algos.WeighERC(lookback=pd.DateOffset(years=1)),
                bt.algos.RunMonthly(),    
                bt.algos.Rebalance()
            ])
        elif self.rebalence == 'daily':
            gdaa  = bt.Strategy("GDAA", [
                bt.algos.SelectAll(),
                bt.algos.SelectMomentum(n=self.n_s, lookback=pd.DateOffset(years=self.year_s)),
                #bt.algos.WeighERC(lookback=pd.DateOffset(years=1)),
                bt.algos.RunDaily(),
                bt.algos.Rebalance()
            ])
        elif self.rebalence == 'yearly':
            gdaa  = bt.Strategy("GDAA", [
                bt.algos.SelectAll(),
                bt.algos.SelectMomentum(n=self.n_s, lookback=pd.DateOffset(years=self.year_s)),
                #bt.algos.WeighERC(lookback=pd.DateOffset(years=1)),
                bt.algos.RunYearly(),
                bt.algos.Rebalance()
            ])
        elif self.rebalence == 'quarterly':
            gdaa  = bt.Strategy("GDAA", [
                bt.algos.SelectAll(),
                bt.algos.SelectMomentum(n=self.n_s, lookback=pd.DateOffset(years=self.year_s)),
                #bt.algos.WeighERC(lookback=pd.DateOffset(years=1)),
                bt.algos.RunQuarterly(),
                bt.algos.Rebalance()
            ])
        else:
            raise ValueError("Invalid rebalence option")


        gdaas = bt.Strategy('GDAA', [
            bt.algos.SelectAll(),
            bt.algos.SelectMomentum(n=self.n_s, lookback=pd.DateOffset(years=self.year_s)),
            # bt.algos.WeighERC(lookback=pd.DateOffset(years=1)),
            bt.algos.RunMonthly(),    
            bt.algos.Rebalance()
        ])

        # 백테스트 실행
        gdaa_backtest = bt.Backtest(gdaa , data, initial_capital = self.initial_capitals)
        gdaa_result = bt.run(gdaa_backtest)

        # 결과 출력
        print(gdaa_result.display())

        # # 상위 디렉토리로 경로 이동
        # import sys
        # sys.path.append('..')  # 이 코드는 main.py가 위치한 subpackage 디렉토리에서 상위 디렉토리로 경로를 추가합니다.
        # # my_module 모듈을 임포트
        # import csv_print

        # csv_print.process_backtest_results(gdaa_result, self.initial_capitals)