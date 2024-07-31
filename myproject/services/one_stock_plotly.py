import plotly.graph_objs as go
from plotly.subplots import make_subplots
import pandas as pd
from sqlalchemy import create_engine
import numpy as np
from datetime import timedelta

# 데이터베이스에서 데이터 불러오기
engine = create_engine('mysql+pymysql://root:1234@127.0.0.1:3306/stock_db') # 데이터베이스 연결
stock_price_df = pd.read_sql('select * from lab_assignment;', con=engine) # 데이터 불러오기
engine.dispose() # 데이터베이스 연결 종료

# JSON 파일에서 데이터 불러오기
file_path = 'Record/stock_history.json'
data = pd.read_json(file_path).to_dict() # JSON 파일을 딕셔너리로 변환

# 주식 종목별 수익률 그래프 생성
def one_stock_plotly(stock_symbol):
    dates = []
    prices = []
    buys = []
    sells = []
    return_stock_selling = []
    cash_plus_stock_values = []

    # 날짜별로 데이터 추출
    for date, details in data.items(): # JSON 파일에서 날짜별로 데이터 추출
        cash_plus_stock_values.append(details['cash_plus_stock']) 
        if stock_symbol in details['stocks']: # 주식 종목이 존재할 경우
            stock_data = details['stocks'][stock_symbol] # 주식 종목 데이터 추출
            buys.append(stock_data['quantity_buy']) # 매수량 추가
            sells.append(stock_data['quantity_sold']) # 매도량 추가
            return_stock_selling.append(stock_data['return_stock_selling']*100) # 수익률 추가
        else:
            buys.append(0) # 주식 종목이 없을 경우 0으로 채움
            sells.append(0)  # 주식 종목이 없을 경우 0으로 채움
            return_stock_selling.append(0) # 주식 종목이 없을 경우 0으로 채움

    # 날짜와 주가 데이터 추출
    dates = stock_price_df['Date'].tolist()
    # 주가 데이터 추출
    prices = stock_price_df[stock_symbol].tolist()
    # 주가 데이터 소수점 둘째 자리까지 반올림
    prices = [round(price, 2) for price in prices]
    # prices 리스트를 numpy 배열로 변환
    prices_array = np.array(prices)

    # 매수, 매도, 수익률 리스트 길이를 맞추기 위해 0으로 채움
    for i in range(len(dates)-len(buys)):
        buys.append(0) # 매수량 추가
        sells.append(0) # 매도량 추가
        return_stock_selling.append(0) # 수익률 추가
        cash_plus_stock_values.append(0) # 현금 및 주식 가치 추가

    # 누적 수익률 계산
    cumulative_returns = (prices_array - prices_array[0]) / prices_array[0] * 100

    # 그래프 생성
    fig = make_subplots(rows=1, cols=1)

    # 라벨링을 위한 텍스트 생성
    price_hovertext = [
        f"Date: {dates[i].strftime('%d/%m/%Y')}<br>" # 날짜 추가
        f"CR: {cumulative_returns[i]:.1f}%" # 누적 수익률 추가
        for i in range(len(dates)) # 날짜별로 반복
    ]

    # 주가 그래프 추가
    fig.add_trace(go.Scatter(x=dates, y=cumulative_returns, mode='lines', name='Price',  # 선 그래프 추가
                             line=dict(color='purple'), # 선 색상 변경
                             hoverlabel=dict(font=dict(size=20)), # hoverlabel 폰트 크기 변경
                             hovertext=price_hovertext, hoverinfo='text'), row=1, col=1) # hovertext 및 hoverinfo 추가
    
    # 매수 라벨링을 위한 텍스트 생성
    buy_hovertext = [
        f"Date: {dates[i].strftime('%d/%m/%Y')}<br>" # 날짜 추가
        f"CR: {cumulative_returns[i]:.1f}%<br>"  # 누적 수익률 추가
        f"{stock_symbol}: {prices[i] * buys[i] / cash_plus_stock_values[i]*100:.1f}%" # 주식 비율 추가
        for i in range(len(dates)) if buys[i] > 0 # 매수량이 0보다 큰 경우에만 추가
    ]

    # 매수 포인트 추가
    fig.add_trace(go.Scatter(x=[dates[i] + timedelta(minutes=400) for i in range(len(dates)) if buys[i] > 0], # 매수 포인트 추가
                             y=[cumulative_returns[i] for i in range(len(dates)) if buys[i] > 0],  # 누적 수익률 추가
                             mode='markers', name='Buy', # 마커 추가
                             marker=dict(color='red', symbol='triangle-up', size=15), # 마커 색상 및 모양 변경
                             hoverlabel=dict(font=dict(size=20)), # hoverlabel 폰트 크기 변경
                             hovertext=buy_hovertext, # hovertext 추가
                             hoverinfo='text'), row=1, col=1) # hoverinfo 추가
    
    # 매도 라벨ㄹ링을 위한 텍스트 생성
    sell_hovertext = [
        f"Date: {dates[i].strftime('%d/%m/%Y')}<br>" # 날짜 추가
        f"CR: {cumulative_returns[i]:.1f}%<br>" # 누적 수익률 추가
        f"{stock_symbol} : {prices[i] * sells[i] / cash_plus_stock_values[i]*100:.1f}%<br>" # 주식 비율 추가
        f"ROI: {return_stock_selling[i]:.1f}%" # 수익률 추가
        for i in range(len(dates)) if sells[i] > 0 # 매도량이 0보다 큰 경우에만 추가
    ]

    # 매도 포인트 추가
    fig.add_trace(go.Scatter(x=[dates[i] - timedelta(minutes=400) for i in range(len(dates)) if sells[i] > 0], # 매도 포인트 추가
                             y=[cumulative_returns[i] for i in range(len(dates)) if sells[i] > 0],  # 누적 수익률 추가
                             mode='markers', name='Sell',  # 마커 추가
                             marker=dict(color='blue', symbol='triangle-down', size=15), # 마커 색상 및 모양 변경
                             hoverlabel=dict(font=dict(size=20)), # hoverlabel 폰트 크기 변경
                             hovertext=sell_hovertext, # hovertext 추가
                             hoverinfo='text'), row=1, col=1) # hoverinfo 추가

    # 그래프 레이아웃 설정
    fig.update_layout(
                hoverlabel_align = 'right', # hoverlabel 위치 변경
                title={'text': f'{stock_symbol} Stock Cumulative Returns(CR) with Buy/Sell Points', # 제목 추가
                        'font': {'size': 24}}, # 제목 폰트 크기 변경
                xaxis_title='Date', # x축 제목 추가
                yaxis_title='Cumulative Returns(%)', # y축 제목 추가
                xaxis=dict(title=dict(font=dict(size=17))),  # x축 폰트 크기 변경
                yaxis=dict(title=dict(font=dict(size=17))),  # y축 폰트 크기 변경
                showlegend=True # 범례 추가
                )

    # 그래프 저장
    fig.write_html("C:/Users/ggp05/OneDrive/Desktop/myproject/static/stock_plot.html") # 그래프를 HTML 파일로 저장

if __name__ == '__main__':
    one_stock_plotly('HPQ')
