import pandas as pd

import os

def process_backtest_results(result, initial_capital):
    
    # 매매내역 추출 및 저장
    transactions = result.get_transactions()
    transactions.to_csv('transactions.csv')
    
    # 현재 자산 추출 및 저장(price는 백분율로 나타냄, 초기자산 필요)
    Cumulative_Return = result.prices * initial_capital / 100
    Cumulative_Return.to_csv('price.csv')

    # 누적수익률
    cumulative_returns = result.prices / result.prices.iloc[0] - 1
    cumulative_returns.to_csv('cumulative returns.csv')

    # 최대 낙폭(MDD)과 평균 낙폭 지속 기간 추출 및 출력
    mdd = result.stats.loc['max_drawdown']
    print(f"max drawdown: {mdd}")
    mdd.to_csv('mdd.csv')
    
    # CAGR 계산 및 CSV 파일로 저장
    final_value = result.prices.iloc[-1]
    initial_value = result.prices.iloc[0]
    years = (result.prices.index[-1] - result.prices.index[0]).days / 365.25
    cagr = (final_value / initial_value) ** (1 / years) - 1
    cagr_df = pd.DataFrame(cagr, columns=['CAGR'])
    cagr_df.to_csv('cagr results.csv')


    #투자기간
    # 시작 날짜와 종료 날짜 추출
    start_date = result.prices.index[0]
    end_date = result.prices.index[-1]

    # 투자 기간 계산 (일 단위)
    investment_period_days = (end_date - start_date).days

    # 결과를 데이터프레임으로 저장
    investment_period_df = pd.DataFrame({
        'Start Date': [start_date],
        'End Date': [end_date],
        'Investment Period (days)': [investment_period_days]
    })
    # CSV 파일로 저장
    investment_period_df.to_csv('investment_period.csv', index=False)

    #투자원금
    initial_value = result.prices.iloc[0]*initial_capital / 100
    initial_asset = pd.DataFrame(initial_value)
    initial_asset.to_csv('initial accest.csv', index=False)
    
    #최종자산
    final_value = result.prices.iloc[-1]*initial_capital / 100
    final_asset = pd.DataFrame(final_value)
    final_asset.to_csv('final asset.csv', index=False)
 
    #실현손익
    Realised_P_L = pd.DataFrame(final_value-initial_value, columns=['Realised P_L'])
    Realised_P_L.to_csv('Realised P_L.csv')
    
    # 자산 비율 데이터 추출
    security_weights = result.get_security_weights()
    security_weights.to_csv('stock ratio.csv')
    # 결과 요약 출력
    result.display()