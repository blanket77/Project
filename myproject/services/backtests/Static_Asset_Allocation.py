from sqlalchemy import create_engine
import pandas as pd
import bt
import io
import contextlib
import quantstats as qs
import plotly.graph_objects as go

# 정적 자산 배분 전략
def Static_Asset_Allocation(tickers, start_date, end_date):
    ticker_str = ', '.join(tickers)  # 티커를 SQL 쿼리에 맞는 형식으로 변환

    # 초기 자본금 설정
    initial_capitals =1000000

    # 리밸런싱 변수
    rebalence = 'monthly'  # 'weekly', 'monthly'와 같은 다른 옵션을 추가할 수 있음

    # MySQL 데이터베이스 연결 설정
    engine = create_engine('mysql+pymysql://root:1234@127.0.0.1:3306/stock_db')

    instruc_SQL =  f"select {ticker_str},Date from sample_etf_adj_close where Date between '{start_date}' and '{end_date}';"
    # 데이터 로드
    price = pd.read_sql(instruc_SQL, con=engine)
    price = price.set_index(['Date'])
    engine.dispose()

    # 시그널 데이터프레임 초기화
    select_w = pd.DataFrame(index=price.index, columns=price.columns, dtype=bool)
    select_w.loc[:,:] = False
    signal = {}
    # 데이터 프레임 생성
    signal_data = {
        'SPY': [True, False, False],
        'TLT': [False, True, True],
        'IEF': [False, True, True],
        'GLD': [True, False, False],
        'DBC': [True, False, True]
    }

    # 날짜 인덱스 설정
    datess = ['2020-07-01', '2020-08-01', '2020-09-01']

    # 데이터프레임 생성, 인덱스를 날짜로 설정
    df = pd.DataFrame(signal_data, index=pd.to_datetime(datess))

    # 가정: select_w와 df의 인덱스가 문자열 형태인 경우
    select_w.index = pd.to_datetime(select_w.index)
    df.index = pd.to_datetime(df.index)

    # 같은 년도와 월을 기준으로 df 값을 select_w에 복사
    for date in select_w.index:
        matching_dates = df.index[(df.index.year == date.year) & (df.index.month == date.month)]
        if not matching_dates.empty:
            # 해당하는 날짜가 있을 경우, df에서 해당 날짜의 데이터를 select_w의 해당 날짜에 할당
            select_w.loc[date] = df.loc[matching_dates[0]]

    print(select_w)

    # NaN 값 제거
    data = price.dropna()

    # 전략 설정
    if rebalence == 'once':
        strategy = bt.Strategy("Asset_EW", [
            bt.algos.SelectAll(),
            bt.algos.WeighEqually(),
            bt.algos.RunOnce(),
            bt.algos.Rebalance()
        ])
    elif rebalence == 'weekly':
        strategy = bt.Strategy("Asset_EW", [
            bt.algos.SelectAll(),
            bt.algos.WeighEqually(),
            bt.algos.RunWeekly(),
            bt.algos.Rebalance()
        ])
    elif rebalence == 'monthly':
        strategy = bt.Strategy("Asset_EW", [
            bt.algos.SelectAll(),
            # bt.algos.SelectWhere(select_w),
            bt.algos.WeighEqually(),
            bt.algos.RunMonthly(),
            bt.algos.Rebalance()
        ])
    elif rebalence == 'daily':
        strategy = bt.Strategy("Asset_EW", [
            bt.algos.SelectAll(),
            bt.algos.WeighEqually(),
            bt.algos.RunDaily(),
            bt.algos.Rebalance()
        ])
    elif rebalence == 'yearly':
        strategy = bt.Strategy("Asset_EW", [
            bt.algos.SelectAll(),
            bt.algos.WeighEqually(),
            bt.algos.RunYearly(),
            bt.algos.Rebalance()
        ])
    elif rebalence == 'quarterly':
        strategy = bt.Strategy("Asset_EW", [
            bt.algos.SelectAll(),
            bt.algos.WeighEqually(),
            bt.algos.RunQuarterly(),
            bt.algos.Rebalance()
        ])
    else:
        raise ValueError("Invalid rebalence option")

    # 백테스트 실행
    backtest = bt.Backtest(strategy, data, initial_capital = initial_capitals)
    # 백테스트 실행
    result = bt.run(backtest)

    # 백테스트 결과 데이터 가져오기
    data = result.get_security_weights()
    
    # 수익률 데이터 가져오기
    returns = result.prices

    # Plotly를 사용하여 시각화
    fig = go.Figure()

    # 수익률 차트 추가
    for col in returns.columns:
        fig.add_trace(go.Scatter(x=returns.index, y=returns[col], mode='lines', name=col))

    # 차트 레이아웃 설정
    fig.update_layout(
        title='Backtest Result',
        xaxis_title='Date',
        yaxis_title='Price',
        template='plotly_dark'
    )

    # HTML 파일로 저장
    fig.write_html("static/backtest_report.html")


    # result 객체에서 누적 수익률 추출
    returns = result.get('Asset_EW').prices.pct_change().dropna()


    # QuantStats 리포트 생성 및 저장
    qs.reports.html(returns, output='static/report.html')
    # 수익률 출력
    print(result.display())

    # 종목별 수익률 데이터프레임으로 저장
    returns = result.get_security_weights().dropna()
    returns.columns = [f'{col}_return' for col in returns.columns]

    # # csv_print 모듈의 print_csv 함수 호출
    with io.StringIO() as buf, contextlib.redirect_stdout(buf):
        result.display()
        result_string = buf.getvalue()
    
    result_string = result_string.replace('\n', '<br>')
    return result_string

if __name__ == '__main__':
    Static_Asset_Allocation(['SPY', 'TLT', 'IEF', 'GLD', 'DBC'], '2020-04-30', '2022-04-30')