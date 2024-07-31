from services.Client.StockPortfolio import StockPortfolio
from services.Client.strategy.day_buy_day_sell import *
import pandas as pd
import csv
from datetime import datetime


sell_commission = 0 

# __main__ 를 쓰기 위해 변경(위에 두줄 코드는 주석처리)
# from Client.StockPortfolio import StockPortfolio
# from Client.strategy.day_buy_day_sell import *

# CSV 파일을 읽어서 날짜별로 종목을 그룹화
def find_earliest_date(filename):
    earliest_date = None # 가장 빠른 날짜
    with open(filename, mode='r') as file: # 파일 열기
        reader = csv.DictReader(file) # CSV 파일을 딕셔너리 형태로 읽기
        for row in reader: # 각 행에 대해 반복
            # 날짜 파싱: 'M/D/YYYY' 형식을 datetime 객체로 변환
            date = datetime.strptime(row['date'], '%m/%d/%Y') # 날짜 파싱
            if earliest_date is None or date < earliest_date: # 가장 빠른 날짜 찾기
                earliest_date = date # 가장 빠른 날짜 업데이트
    return earliest_date.strftime('%Y-%m-%d') if earliest_date else None # 날짜를 문자열로 변환하여 반환

# CSV 파일을 읽어서 날짜별로 종목을 그룹화
def adj_run_portfolio_analysis(remove_tickers):
    # CSV 파일 읽기
    df = pd.read_csv('uploads/backtest.csv')
    # 입력 문자열을 대문자로 변환하고, 쉼표로 구분하여 리스트로 변환
    remove_tickers = [ticker.strip().upper() for ticker in remove_tickers.split(',')]

    # 리스트에 포함된 종목 제거
    df_filtered = df[~df['ticker'].isin(remove_tickers)]

    # 날짜별로 그룹화하고 각 그룹에 대해 가중치 조정
    adjusted_weights = []
    for date, group in df_filtered.groupby('date'): # 날짜별로 그룹화
        total_weight = group['weight'].sum() # 가중치 합계 계산
        adjusted_group = group.copy() # 그룹 복사
        adjusted_group['weight'] = group['weight'] / total_weight  # 가중치 조정
        adjusted_weights.append(adjusted_group) # 특정 종목을 뺀 그룹 추가

    # 특정 종목을 뺀 데이터프레임 생성
    adjusted_df = pd.concat(adjusted_weights)

    # 특정 종목을 뺀 데이터를 새로운 CSV 파일로 저장
    adjusted_df.to_csv('uploads/backtest_adj.csv', index=False)

    # 백테스트 로직 실행
    date_to_symbols = get_date_to_symbols("uploads/backtest_adj.csv")
    # 가장 빠른 날짜 찾기
    earliest_date = find_earliest_date('uploads/backtest_adj.csv')
    # 포트폴리오 생성
    portfolio2 = StockPortfolio(earliest_date, 1000000)

    # 여기서 date_to_symbols는 어딘가에서 정의되어 있어야 합니다.
    for date, symbols in date_to_symbols.items():
        trade_stocks(portfolio2, date, symbols)

    # 이전 그래프 데이터 로드
    portfolio2.previous_fig_json('Record/day_graph.json')
    # 수익률 그래프 생성
    portfolio2.plot_rate_of_return_history()
    # 포트폴리오 요약 정보
    portfolio2.get_portfolio_summary()
    # 수익률 계산
    portfolio2.calculate_return_rate()
    # 일별 기록
    portfolio2.get_daily_history()
    #주식 수익률 계산하고 json으로 저장
    portfolio2.calculate_all_stock_return_rates('stock_rate_adj.json')
    #주식 기록 파일 생성
    portfolio2.get_daily_history_file('stock_history_adj.json')
    #주식 기록을 읽어서 데이터 전처리 이후 json으로 저장
    portfolio2.statistics_stock('Record/stock_history_adj.json', 'Record/sorted_stock_adj.json')
    # 그래프 저장
    portfolio2.fig.write_html("static/day_report_adj.html")


if __name__ == '__main__':
    adj_run_portfolio_analysis('ALB')

    